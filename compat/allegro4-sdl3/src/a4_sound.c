/*
 * Samples, voices, software mixer, WAV/VOC loading, MIDI stubs.
 *
 * Model: the historical Windows build autodetected Allegro 4.4.1's
 * "Allegmix" DirectSound driver (DIGI_DIRECTAMX: Allegro's own software
 * mixer feeding one DirectSound buffer) with its defaults: 8 mixer voices
 * (MIXER_DEF_SFX), 44100 Hz, 16-bit stereo output, quality 2 (linearly
 * interpolated mixing), volume-per-voice scale 1, no pan flip.  This file
 * ports that stack:
 *   - virtual/physical voice bookkeeping, allocation, priorities and voice
 *     stealing, play/adjust/stop_sample, set_volume, the voice_* calls
 *     (Allegro 4.4.1 src/sound.c);
 *   - the mixer: 24.8 fixed-point positions, volume/pan law, 8/16-bit
 *     unsigned mono/stereo input, the interpolated (quality 2) mix paths,
 *     the silent-voice fast path, 24-bit accumulation and clamping to the
 *     16-bit output (src/mixer.c);
 *   - load_wav/load_voc (src/sound.c), load_sample dispatch (src/readsmp.c).
 * Allegro is giftware; see third_party/licenses/allegro-license.txt.
 *
 * Device: install_sound() opens the platform audio device
 * (plat_audio_open(44100, a4_mix)); SDL then pulls stereo float frames
 * through a4_mix() on its audio thread.  Without a device (headless mode,
 * or no audio hardware) all bookkeeping still works exactly the same and
 * nothing advances voices except explicit a4_mix() calls, which is how the
 * unit tests drive the mixer.
 *
 * Threading: every public entry point that touches voice or mixer state runs
 * under plat_audio_lock() (the SDL audio stream lock).  SDL holds the same
 * lock while it runs the stream callback that calls a4_mix(), so the mixer
 * and the main thread never see each other's state half-updated;
 * voice_get_position() therefore always reports the position reached by the
 * last completed mix call.  a4_mix() itself does not lock.
 *
 * Positions: as in Allegro, a voice position is the 24.8 fixed-point index
 * into its SAMPLE, advanced per output frame by
 * diff = (freq << 8) / device_freq with freq = sample->freq * pitch / 1000
 * (integer truncation as Allegro), whether or not the voice is audible
 * (volume-0 voices use the silent path which advances identically).
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "a4_internal.h"
#include "port/platform/platform.h"

/* ------------------------------------------------------------ constants */

#define VIRTUAL_VOICES     256      /* aintern.h */
#define MIXER_DEF_SFX      8        /* default mixer voices (Allegmix drivers) */
#define MIXER_MAX_SFX      64

#define PLAYMODE_PLAY      0        /* digi.h */
#define PLAYMODE_LOOP      1
#define PLAYMODE_BIDIR     4

#define MIX_FIX_SHIFT      8        /* mixer.c */
#define MIX_FIX_SCALE      (1 << MIX_FIX_SHIFT)

#define A4_SOUND_FREQ      44100    /* Allegmix default when sound_freq is unset */
#define A4_MIX_CHUNK       1024     /* frames mixed per inner pass */

/* ------------------------------------------------------------ state */

typedef struct VOICE {              /* sound.c: a virtual voice */
   const SAMPLE *sample;            /* the sample being played, or NULL */
   int num;                         /* physical voice number, or -1 */
   int autokill;                    /* set by release_voice() */
   long time;                       /* retrace_count at allocation/start */
   int priority;
} VOICE;

typedef struct PHYS_VOICE {         /* aintern.h: a physical voice */
   int num;                         /* virtual voice number, or -1 */
   int playmode;
   int vol;                         /* 0..255 << 12 */
   int pan;                         /* 0..255 << 12 */
   int freq;                        /* Hz << 12 */
} PHYS_VOICE;

typedef struct MIXER_VOICE {        /* mixer.c */
   int playing;
   int channels;
   int bits;
   const void *data;
   int64_t pos;                     /* 24.8 fixed point (Allegro: 32-bit long) */
   int64_t diff;
   int64_t len;
   int64_t loop_start;
   int64_t loop_end;
   int lvol, rvol;                  /* 0..65535 */
} MIXER_VOICE;

static VOICE virt_voice[VIRTUAL_VOICES];
static PHYS_VOICE phys_voice[MIXER_MAX_SFX];
static MIXER_VOICE mixer_voice[MIXER_MAX_SFX];

static int tables_ready;
static int sound_installed;
static int device_open;
static int digi_voices;             /* physical voices: 0 = digi_none */
static int digi_volume = -1;        /* _digi_volume */
static int midi_volume = -1;        /* _midi_volume (stored only) */
static int mix_freq = A4_SOUND_FREQ;
static const int voice_volume_scale = 1;   /* set_volume_per_voice() never called */

static void lock(void) { plat_audio_lock(); }
static void unlock(void) { plat_audio_unlock(); }

static void init_tables(void)
{
   int c;
   for (c = 0; c < VIRTUAL_VOICES; c++) {
      virt_voice[c].sample = NULL;
      virt_voice[c].num = -1;
      virt_voice[c].autokill = 0;
   }
   for (c = 0; c < MIXER_MAX_SFX; c++) {
      phys_voice[c].num = -1;
      mixer_voice[c].playing = 0;
      mixer_voice[c].data = NULL;
   }
   tables_ready = 1;
}

static void ensure_tables(void)
{
   if (!tables_ready)
      init_tables();
}

/* Allegro's retrace_count: a 70 Hz counter, used only as the LRU bias when
 * a voice has to be stolen. */
static long retrace_count(void)
{
   return (long)(plat_ticks_ns() / (1000000000u / 70u));
}

static int clamp_val(int i, int max)
{
   if (i < 0) return 0;
   if (i > max) return max;
   return i;
}

static int valid_voice(int voice)
{
   return voice >= 0 && voice < VIRTUAL_VOICES;
}

/* ------------------------------------------------------------ mixer.c */

/* update_mixer_volume (mixer.c) */
static void update_mixer_volume(MIXER_VOICE *mv, const PHYS_VOICE *pv)
{
   int vol, pan, lvol, rvol;

   vol = pv->vol >> 12;
   pan = pv->pan >> 12;

   lvol = vol * (255 - pan);
   rvol = vol * pan;

   /* Adjust for 255*255 < 256*256-1 */
   lvol += lvol >> 7;
   rvol += rvol >> 7;

   mv->lvol = clamp_val((lvol * 2) >> voice_volume_scale, 65535);
   mv->rvol = clamp_val((rvol * 2) >> voice_volume_scale, 65535);
}

/* update_mixer_freq (mixer.c) */
static void update_mixer_freq(MIXER_VOICE *mv, const PHYS_VOICE *pv)
{
   mv->diff = (pv->freq >> (12 - MIX_FIX_SHIFT)) / mix_freq;
}

static void mixer_init_voice(int voice, const SAMPLE *sample)
{
   MIXER_VOICE *mv = mixer_voice + voice;
   mv->playing = 0;
   mv->channels = sample->stereo ? 2 : 1;
   mv->bits = sample->bits;
   mv->pos = 0;
   mv->len = (int64_t)sample->len << MIX_FIX_SHIFT;
   mv->loop_start = (int64_t)sample->loop_start << MIX_FIX_SHIFT;
   mv->loop_end = (int64_t)sample->loop_end << MIX_FIX_SHIFT;
   mv->data = sample->data;
   update_mixer_volume(mv, phys_voice + voice);
   update_mixer_freq(mv, phys_voice + voice);
}

static void mixer_release_voice(int voice)
{
   mixer_voice[voice].playing = 0;
   mixer_voice[voice].data = NULL;
}

static void mixer_start_voice(int voice)
{
   if (mixer_voice[voice].pos >= mixer_voice[voice].len)
      mixer_voice[voice].pos = 0;
   mixer_voice[voice].playing = 1;
}

static void mixer_stop_voice(int voice)
{
   mixer_voice[voice].playing = 0;
}

static int mixer_get_position(int voice)
{
   if (!mixer_voice[voice].playing || mixer_voice[voice].pos >= mixer_voice[voice].len)
      return -1;
   return (int)(mixer_voice[voice].pos >> MIX_FIX_SHIFT);
}

/* one channel value of a frame as signed 24-bit, like the hq2 mixers */
static inline int32_t smp24(const MIXER_VOICE *mv, int64_t index)
{
   if (mv->bits == 8)
      return ((int32_t)((const uint8_t *)mv->data)[index] << 16) - 0x800000;
   return ((int32_t)((const uint16_t *)mv->data)[index] << 8) - 0x800000;
}

/* MULSC (mixer.c): ((a << 4) * (b << 12)) >> 32 in 64 bits */
static inline int32_t mulsc(int32_t a, int32_t b)
{
   return (int32_t)(((int64_t)a * 16 * ((int64_t)b * 4096)) >> 32);
}

/* mix_hq2_{8,16}x{1,2}_samples through the MIXER() body (forward play;
 * the backward/bidirectional modes cannot be selected through this API). */
static void mix_hq2_samples(MIXER_VOICE *spl, const PHYS_VOICE *voice, int32_t *buf, int len)
{
   const int ch = spl->channels;
   const int lvol = spl->lvol, rvol = spl->rvol;
   const int looping = (voice->playmode & PLAYMODE_LOOP) && spl->loop_start < spl->loop_end;
   /* the interpolation partner of the last frame wraps to loop_start only for
    * plain loops that end at the sample end */
   const int wrap_interp = (voice->playmode & (PLAYMODE_LOOP | PLAYMODE_BIDIR)) == PLAYMODE_LOOP &&
                           spl->loop_start < spl->loop_end && spl->loop_end == spl->len;

   while (len--) {
      int64_t f = spl->pos >> MIX_FIX_SHIFT;
      int32_t v1a, v1b, v2a, v2b, va, vb;
      int64_t frac;

      v1a = smp24(spl, f * ch);
      v1b = (ch == 2) ? smp24(spl, f * ch + 1) : v1a;

      if (spl->pos >= spl->len - MIX_FIX_SCALE) {
         if (wrap_interp) {
            int64_t g = spl->loop_start >> MIX_FIX_SHIFT;
            v2a = smp24(spl, g * ch);
            v2b = (ch == 2) ? smp24(spl, g * ch + 1) : v2a;
         }
         else
            v2a = v2b = 0;
      }
      else {
         v2a = smp24(spl, (f + 1) * ch);
         v2b = (ch == 2) ? smp24(spl, (f + 1) * ch + 1) : v2a;
      }

      frac = spl->pos & (MIX_FIX_SCALE - 1);
      va = (int32_t)(((int64_t)v2a * frac + (int64_t)v1a * (MIX_FIX_SCALE - frac)) >> MIX_FIX_SHIFT);
      vb = (ch == 2) ? (int32_t)(((int64_t)v2b * frac + (int64_t)v1b * (MIX_FIX_SCALE - frac)) >> MIX_FIX_SHIFT) : va;

      *(buf++) += mulsc(va, lvol);
      *(buf++) += mulsc(vb, rvol);

      spl->pos += spl->diff;
      if (looping) {
         if (spl->pos >= spl->loop_end)
            spl->pos -= (spl->loop_end - spl->loop_start);
      }
      else if ((uint64_t)spl->pos >= (uint64_t)spl->len) {
         spl->playing = 0;
         return;
      }
   }
}

/* mix_silent_samples (mixer.c), forward modes */
static void mix_silent_samples(MIXER_VOICE *spl, const PHYS_VOICE *voice, int len)
{
   if ((voice->playmode & PLAYMODE_LOOP) && spl->loop_start < spl->loop_end) {
      spl->pos += spl->diff * len;
      if (spl->pos >= spl->loop_end) {
         do {
            spl->pos -= (spl->loop_end - spl->loop_start);
         } while (spl->pos >= spl->loop_end);
      }
   }
   else {
      spl->pos += spl->diff * len;
      if ((uint64_t)spl->pos >= (uint64_t)spl->len)
         spl->playing = 0;
   }
}

void a4_mix(float *out, int frames, int device_freq)
{
   static int32_t buf[A4_MIX_CHUNK * 2];
   int i, k, n;

   if (!out || frames <= 0)
      return;

   if (device_freq > 0 && device_freq != mix_freq) {
      mix_freq = device_freq;
      for (i = 0; i < digi_voices; i++)
         if (phys_voice[i].num >= 0)
            update_mixer_freq(mixer_voice + i, phys_voice + i);
   }

   while (frames > 0) {
      n = frames < A4_MIX_CHUNK ? frames : A4_MIX_CHUNK;
      memset(buf, 0, sizeof(int32_t) * 2 * (size_t)n);

      for (i = 0; i < digi_voices; i++) {
         MIXER_VOICE *mv = mixer_voice + i;
         if (!mv->playing)
            continue;
         if (!mv->data || mv->len <= 0) {      /* guard: nothing to read */
            mv->playing = 0;
            continue;
         }
         if (phys_voice[i].vol > 0)
            mix_hq2_samples(mv, phys_voice + i, buf, n);
         else
            mix_silent_samples(mv, phys_voice + i, n);
      }

      /* _mix_some_samples: clamp the 24-bit sum and keep 16 bits (the
       * DirectSound buffer format), then scale to float */
      for (k = 0; k < n * 2; k++) {
         int32_t s = buf[k] + 0x800000;
         if (s < 0) s = 0;
         if (s > 0xFFFFFF) s = 0xFFFFFF;
         out[k] = (float)((s >> 8) - 0x8000) * (1.0f / 32768.0f);
      }
      out += n * 2;
      frames -= n;
   }
}

/* ------------------------------------------------------------ sound.c voices */

static void voice_set_volume_locked(int voice, int volume)
{
   if (digi_volume >= 0)
      volume = (volume * digi_volume) / 255;

   if (virt_voice[voice].num >= 0) {
      PHYS_VOICE *pv = phys_voice + virt_voice[voice].num;
      pv->vol = volume * 4096;
      update_mixer_volume(mixer_voice + virt_voice[voice].num, pv);
   }
}

static int voice_get_volume_locked(int voice)
{
   int vol;

   if (virt_voice[voice].num >= 0)
      vol = phys_voice[virt_voice[voice].num].vol >> 12;
   else
      vol = -1;

   if (vol >= 0 && digi_volume >= 0) {
      if (digi_volume > 0)
         vol = clamp_val((vol * 255) / digi_volume, 255);
      else
         vol = 0;
   }
   return vol;
}

static void voice_set_pan_locked(int voice, int pan)
{
   if (virt_voice[voice].num >= 0) {
      PHYS_VOICE *pv = phys_voice + virt_voice[voice].num;
      pv->pan = pan * 4096;
      update_mixer_volume(mixer_voice + virt_voice[voice].num, pv);
   }
}

static void voice_set_frequency_locked(int voice, int frequency)
{
   if (frequency < 1)                 /* Allegro ASSERTs frequency > 0 */
      frequency = 1;
   if (virt_voice[voice].num >= 0) {
      PHYS_VOICE *pv = phys_voice + virt_voice[voice].num;
      pv->freq = frequency << 12;
      update_mixer_freq(mixer_voice + virt_voice[voice].num, pv);
   }
}

static void voice_set_playmode_locked(int voice, int playmode)
{
   if (virt_voice[voice].num >= 0) {
      phys_voice[virt_voice[voice].num].playmode = playmode;
      update_mixer_freq(mixer_voice + virt_voice[voice].num, phys_voice + virt_voice[voice].num);
   }
}

static void voice_start_locked(int voice)
{
   if (virt_voice[voice].num >= 0)
      mixer_start_voice(virt_voice[voice].num);
   virt_voice[voice].time = retrace_count();
}

static int voice_get_position_locked(int voice)
{
   if (virt_voice[voice].num >= 0)
      return mixer_get_position(virt_voice[voice].num);
   return -1;
}

/* allocate_physical_voice (sound.c) */
static int allocate_physical_voice(int priority)
{
   VOICE *voice;
   int best = -1;
   int best_score = 0;
   int score;
   int c;

   /* look for a free voice */
   for (c = 0; c < digi_voices; c++)
      if (phys_voice[c].num < 0)
         return c;

   /* look for an autokill voice that has stopped */
   for (c = 0; c < digi_voices; c++) {
      voice = virt_voice + phys_voice[c].num;
      if (voice->autokill && mixer_get_position(c) < 0) {
         mixer_release_voice(c);
         voice->sample = NULL;
         voice->num = -1;
         phys_voice[c].num = -1;
         return c;
      }
   }

   /* ok, we're going to have to get rid of something to make room... */
   for (c = 0; c < digi_voices; c++) {
      long age;
      voice = virt_voice + phys_voice[c].num;

      /* sort by voice priorities */
      if (voice->priority <= priority) {
         score = 65536 - voice->priority * 256;

         /* bias with a least-recently-used counter */
         age = retrace_count() - voice->time;
         score += (int)(age < 0 ? 0 : age > 32768 ? 32768 : age);

         /* bias according to whether the voice is looping or not */
         if (!(phys_voice[c].playmode & PLAYMODE_LOOP))
            score += 32768;

         if (score > best_score) {
            best = c;
            best_score = score;
         }
      }
   }

   if (best >= 0) {
      /* kill off the old voice */
      mixer_stop_voice(best);
      mixer_release_voice(best);
      virt_voice[phys_voice[best].num].num = -1;
      phys_voice[best].num = -1;
      return best;
   }

   return -1;
}

/* allocate_virtual_voice (sound.c) */
static int allocate_virtual_voice(void)
{
   int c;

   /* look for a free voice */
   for (c = 0; c < VIRTUAL_VOICES; c++)
      if (!virt_voice[c].sample)
         return c;

   /* look for a stopped autokill voice */
   for (c = 0; c < VIRTUAL_VOICES; c++) {
      if (virt_voice[c].autokill) {
         if (virt_voice[c].num < 0) {
            virt_voice[c].sample = NULL;
            return c;
         }
         else if (mixer_get_position(virt_voice[c].num) < 0) {
            mixer_release_voice(virt_voice[c].num);
            phys_voice[virt_voice[c].num].num = -1;
            virt_voice[c].sample = NULL;
            virt_voice[c].num = -1;
            return c;
         }
      }
   }

   return -1;
}

/* allocate_voice (sound.c) */
static int allocate_voice_locked(const SAMPLE *spl)
{
   int phys, virt;

   phys = allocate_physical_voice(spl->priority);
   virt = allocate_virtual_voice();

   if (virt >= 0) {
      virt_voice[virt].sample = spl;
      virt_voice[virt].num = phys;
      virt_voice[virt].autokill = 0;
      virt_voice[virt].time = retrace_count();
      virt_voice[virt].priority = spl->priority;

      if (phys >= 0) {
         phys_voice[phys].num = virt;
         phys_voice[phys].playmode = 0;
         phys_voice[phys].vol = ((digi_volume >= 0) ? digi_volume : 255) << 12;
         phys_voice[phys].pan = 128 << 12;
         phys_voice[phys].freq = spl->freq << 12;
         mixer_init_voice(phys, spl);
      }
   }

   return virt;
}

/* deallocate_voice (sound.c) */
static void deallocate_voice_locked(int voice)
{
   if (virt_voice[voice].num >= 0) {
      mixer_stop_voice(virt_voice[voice].num);
      mixer_release_voice(virt_voice[voice].num);
      phys_voice[virt_voice[voice].num].num = -1;
      virt_voice[voice].num = -1;
   }
   virt_voice[voice].sample = NULL;
}

static void stop_sample_locked(const SAMPLE *spl)
{
   int c;
   for (c = 0; c < VIRTUAL_VOICES; c++)
      if (virt_voice[c].sample == spl)
         deallocate_voice_locked(c);
}

/* absolute_freq (sound.c) */
static int absolute_freq(int freq, const SAMPLE *spl)
{
   if (freq == 1000)
      return spl->freq;
   return (spl->freq * freq) / 1000;
}

/* ------------------------------------------------------------ public API */

int install_sound(int digi, int midi, const char *cfg_path)
{
   (void)midi; (void)cfg_path;

   if (sound_installed)
      return 0;

   lock();
   init_tables();
   /* read_sound_config() without a [sound] config section */
   digi_volume = -1;
   midi_volume = -1;
   mix_freq = A4_SOUND_FREQ;
   digi_voices = (digi == DIGI_NONE) ? 0 : MIXER_DEF_SFX;
   sound_installed = 1;
   unlock();

   /* The mixer and the voices work without a device (headless runs, no
    * audio hardware); they then only advance through explicit a4_mix(). */
   if (digi_voices > 0 && !plat_headless() && plat_audio_open(A4_SOUND_FREQ, a4_mix))
      device_open = 1;

   return 0;
}

void remove_sound(void)
{
   int c;

   if (!sound_installed)
      return;

   if (device_open) {
      plat_audio_close();
      device_open = 0;
   }

   lock();
   for (c = 0; c < VIRTUAL_VOICES; c++)
      if (virt_voice[c].sample)
         deallocate_voice_locked(c);
   digi_voices = 0;
   sound_installed = 0;
   unlock();
}

/* set_volume (sound.c): rescales the voices so their relative volumes stay */
void set_volume(int digi_vol, int midi_vol)
{
   int i;

   lock();
   ensure_tables();
   if (digi_vol >= 0) {
      static int voice_vol[VIRTUAL_VOICES];

      for (i = 0; i < VIRTUAL_VOICES; i++)
         voice_vol[i] = voice_get_volume_locked(i);

      digi_volume = clamp_val(digi_vol, 255);

      for (i = 0; i < VIRTUAL_VOICES; i++)
         if (voice_vol[i] >= 0)
            voice_set_volume_locked(i, voice_vol[i]);
   }
   if (midi_vol >= 0)
      midi_volume = clamp_val(midi_vol, 255);
   unlock();
}

SAMPLE *create_sample(int bits, int stereo, int freq, int len)
{
   SAMPLE *spl;
   size_t bytes;

   if (len < 0)
      return NULL;

   spl = (SAMPLE *)malloc(sizeof(SAMPLE));
   if (!spl)
      return NULL;

   spl->bits = bits;
   spl->stereo = stereo;
   spl->freq = freq;
   spl->priority = 128;
   spl->len = (unsigned long)len;
   spl->loop_start = 0;
   spl->loop_end = (unsigned long)len;
   spl->param = 0;

   bytes = (size_t)len * ((bits == 8) ? 1 : sizeof(short)) * (stereo ? 2 : 1);
   spl->data = calloc(bytes ? bytes : 1, 1);
   if (!spl->data) {
      free(spl);
      return NULL;
   }
   return spl;
}

void destroy_sample(SAMPLE *spl)
{
   if (!spl)
      return;

   /* stop every voice that plays it before the data goes away */
   lock();
   ensure_tables();
   stop_sample_locked(spl);
   unlock();

   free(spl->data);
   free(spl);
}

int play_sample(const SAMPLE *spl, int vol, int pan, int freq, int loop)
{
   int voice;

   if (!spl)
      return -1;

   lock();
   ensure_tables();
   voice = allocate_voice_locked(spl);
   if (voice >= 0) {
      voice_set_volume_locked(voice, vol);
      voice_set_pan_locked(voice, pan);
      voice_set_frequency_locked(voice, absolute_freq(freq, spl));
      voice_set_playmode_locked(voice, loop ? PLAYMODE_LOOP : PLAYMODE_PLAY);
      voice_start_locked(voice);
      virt_voice[voice].autokill = 1;          /* release_voice() */
   }
   unlock();

   return voice;
}

/* adjust_sample (sound.c): the first virtual voice that uses the sample */
void adjust_sample(const SAMPLE *spl, int vol, int pan, int freq, int loop)
{
   int c;

   if (!spl)
      return;

   lock();
   ensure_tables();
   for (c = 0; c < VIRTUAL_VOICES; c++) {
      if (virt_voice[c].sample == spl) {
         voice_set_volume_locked(c, vol);
         voice_set_pan_locked(c, pan);
         voice_set_frequency_locked(c, absolute_freq(freq, spl));
         voice_set_playmode_locked(c, loop ? PLAYMODE_LOOP : PLAYMODE_PLAY);
         break;
      }
   }
   unlock();
}

void stop_sample(const SAMPLE *spl)
{
   if (!spl)
      return;
   lock();
   ensure_tables();
   stop_sample_locked(spl);
   unlock();
}

void voice_stop(int voice)
{
   if (!valid_voice(voice))
      return;
   lock();
   ensure_tables();
   if (virt_voice[voice].num >= 0)
      mixer_stop_voice(virt_voice[voice].num);
   unlock();
}

int voice_get_position(int voice)
{
   int pos;
   if (!valid_voice(voice))
      return -1;
   lock();
   ensure_tables();
   pos = voice_get_position_locked(voice);
   unlock();
   return pos;
}

/* ------------------------------------------------------------ loaders */

static SAMPLE *load_voc_pf(PACKFILE *f);

/* load_voc (sound.c) */
static SAMPLE *load_voc(const char *filename)
{
   PACKFILE *f;
   SAMPLE *spl;

   if (!filename)
      return NULL;
   f = pack_fopen(filename, F_READ);
   if (!f)
      return NULL;
   spl = load_voc_pf(f);
   pack_fclose(f);
   return spl;
}

/* load_voc_pf (sound.c) */
static SAMPLE *load_voc_pf(PACKFILE *f)
{
   char buffer[30];
   int freq = 22050;
   int bits = 8;
   SAMPLE *spl = NULL;
   int len;
   int x, ver;
   int s;

   if (!f)
      return NULL;

   memset(buffer, 0, sizeof buffer);

   pack_fread(buffer, 0x16, f);

   if (memcmp(buffer, "Creative Voice File", 0x13))
      goto getout;

   ver = pack_igetw(f);
   if (ver != 0x010A && ver != 0x0114) /* version: should be 0x010A or 0x0114 */
      goto getout;

   ver = pack_igetw(f);
   if (ver != 0x1129 && ver != 0x111f) /* subversion: should be 0x1129 or 0x111f */
      goto getout;

   ver = pack_getc(f);
   if (ver != 0x01 && ver != 0x09)     /* sound data: should be 0x01 or 0x09 */
      goto getout;

   len = pack_igetw(f);                /* length is three bytes long: two */
   x = pack_getc(f);                   /* .. and one byte */
   x <<= 16;
   len += x;

   if (ver == 0x01) {                  /* block type 1 */
      len -= 2;                        /* sub. size of the rest of block header */
      x = pack_getc(f);                /* one byte of frequency */
      freq = 1000000 / (256-x);

      x = pack_getc(f);                /* skip one byte */

      spl = create_sample(8, FALSE, freq, len);

      if (spl) {
         if (pack_fread(spl->data, len, f) < len) {
            destroy_sample(spl);
            spl = NULL;
         }
      }
   }
   else {                              /* block type 9 */
      len -= 12;                       /* sub. size of the rest of block header */
      freq = pack_igetw(f);            /* two bytes of frequency */

      x = pack_igetw(f);               /* skip two bytes */

      bits = pack_getc(f);             /* # of bits per sample */
      if (bits != 8 && bits != 16)
         goto getout;

      x = pack_getc(f);
      if (x != 1)                      /* # of channels: should be mono */
         goto getout;

      pack_fread(buffer, 0x6, f);      /* skip 6 bytes of unknown data */

      spl = create_sample(bits, FALSE, freq, len*8/bits);

      if (spl) {
         if (bits == 8) {
            if (pack_fread(spl->data, len, f) < len) {
               destroy_sample(spl);
               spl = NULL;
            }
         }
         else {
            len /= 2;
            for (x=0; x<len; x++) {
               if ((s = pack_igetw(f)) == EOF) {
                  destroy_sample(spl);
                  spl = NULL;
                  break;
               }
               ((signed short *)spl->data)[x] = (signed short)(s^0x8000);
            }
         }
      }
   }

   getout:
   return spl;
}

/* load_wav (sound.c) */
SAMPLE *load_wav(const char *filename)
{
   PACKFILE *f;
   SAMPLE *spl;

   if (!filename)
      return NULL;
   f = pack_fopen(filename, F_READ);
   if (!f)
      return NULL;
   spl = load_wav_pf(f);
   pack_fclose(f);
   return spl;
}

/* load_wav_pf (sound.c) */
SAMPLE *load_wav_pf(PACKFILE *f)
{
   char buffer[25];
   int i;
   int length, len;
   int freq = 22050;
   int bits = 8;
   int channels = 1;
   int s;
   SAMPLE *spl = NULL;

   if (!f)
      return NULL;

   memset(buffer, 0, sizeof buffer);

   pack_fread(buffer, 12, f);          /* check RIFF header */
   if (memcmp(buffer, "RIFF", 4) || memcmp(buffer+8, "WAVE", 4))
      goto getout;

   while (TRUE) {
      if (pack_fread(buffer, 4, f) != 4)
         break;

      length = (int)pack_igetl(f);     /* read chunk length */

      if (memcmp(buffer, "fmt ", 4) == 0) {
         i = pack_igetw(f);            /* should be 1 for PCM data */
         length -= 2;
         if (i != 1)
            goto getout;

         channels = pack_igetw(f);     /* mono or stereo data */
         length -= 2;
         if ((channels != 1) && (channels != 2))
            goto getout;

         freq = (int)pack_igetl(f);    /* sample frequency */
         length -= 4;

         pack_igetl(f);                /* skip six bytes */
         pack_igetw(f);
         length -= 6;

         bits = pack_igetw(f);         /* 8 or 16 bit data? */
         length -= 2;
         if ((bits != 8) && (bits != 16))
            goto getout;
      }
      else if (memcmp(buffer, "data", 4) == 0) {
         if (channels == 2) {
            /* allocate enough space even if length is odd for some reason */
            len = (length + 1) / 2;
         }
         else {
            len = length;
         }

         if (bits == 16)
            len /= 2;

         spl = create_sample(bits, ((channels == 2) ? TRUE : FALSE), freq, len);

         if (spl) {
            if (bits == 8) {
               if (pack_fread(spl->data, length, f) < length) {
                  destroy_sample(spl);
                  spl = NULL;
               }
            }
            else {
               for (i=0; i<len*channels; i++) {
                  if ((s = pack_igetw(f)) == EOF) {
                     destroy_sample(spl);
                     spl = NULL;
                     break;
                  }
                  ((signed short *)spl->data)[i] = (signed short)(s^0x8000);
               }
            }

            length = 0;
         }
      }

      while (length > 0) {             /* skip the remainder of the chunk */
         if (pack_getc(f) == EOF)
            break;

         length--;
      }
   }

   getout:
   return spl;
}

/* get_extension without a4_file.c: text after the last '.' of the name */
static const char *sample_extension(const char *filename)
{
   const char *p = filename + strlen(filename);
   while (p > filename && p[-1] != '.' && p[-1] != '/' && p[-1] != '\\' && p[-1] != ':')
      p--;
   if (p > filename && p[-1] == '.')
      return p;
   return filename + strlen(filename);
}

static int ascii_casecmp(const char *a, const char *b)
{
   for (;; a++, b++) {
      int ca = (unsigned char)*a, cb = (unsigned char)*b;
      if (ca >= 'A' && ca <= 'Z') ca += 'a' - 'A';
      if (cb >= 'A' && cb <= 'Z') cb += 'a' - 'A';
      if (ca != cb || !ca)
         return ca - cb;
   }
}

/* load_sample (readsmp.c) with the built-in wav and voc types */
SAMPLE *load_sample(const char *filename)
{
   const char *ext;
   if (!filename)
      return NULL;
   ext = sample_extension(filename);
   if (!ascii_casecmp(ext, "wav"))
      return load_wav(filename);
   if (!ascii_casecmp(ext, "voc"))
      return load_voc(filename);
   return NULL;
}

/* ------------------------------------------------------------ MIDI */

/* MIDI playback is not provided: load_midi() fails (NULL), so custom
 * characters with .mid/.midi music simply have no background music, which
 * is what the game does when a MIDI file fails to load.  The other calls
 * are harmless no-ops; set_volume() still stores the MIDI volume. */
MIDI *load_midi(const char *filename) { (void)filename; return NULL; }
void destroy_midi(MIDI *midi) { (void)midi; }
int play_midi(MIDI *midi, int loop) { (void)loop; return midi ? -1 : 0; }
void stop_midi(void) { }

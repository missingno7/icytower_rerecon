/*
 * Unit tests for compat/allegro4-sdl3/src/a4_sound.c.
 *
 * No audio device is needed: without plat_init() the device cannot open,
 * install_sound() still sets up the voices, and the test drives the mixer
 * by calling a4_mix() directly.  Expected output values are recomputed here
 * from the formulas of Allegro 4.4.1 src/mixer.c (quality 2, 44100 Hz).
 *
 * Optional: A4_TEST_AUDIO_DEVICE=1 additionally opens the real device and
 * checks that voice_get_position() follows real playback time.
 *
 * usage: test_sound [TEMP_DIR]
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <allegro.h>
#include "a4_internal.h"
#include "port/platform/platform.h"

#define FREQ 44100

static int failures, checks;
static char tmpdir[1024];

#define CHECK(cond, ...) do { checks++; if (!(cond)) { failures++; \
   printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* ------------------------------------------------------------ reference */

static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

/* update_mixer_volume with voice_volume_scale 1 */
static int ref_lvol(int vol, int pan) { int l = vol * (255 - pan); l += l >> 7; return clampi((l * 2) >> 1, 0, 65535); }
static int ref_rvol(int vol, int pan) { int r = vol * pan; r += r >> 7; return clampi((r * 2) >> 1, 0, 65535); }

/* MULSC + clamp to 24 bits + 16-bit output, as a float */
static int32_t ref_mulsc(int32_t v, int vol)
{
   return (int32_t)(((int64_t)v * 16 * ((int64_t)vol * 4096)) >> 32);
}
static float ref_out(int32_t sum)
{
   int32_t s = clampi(sum + 0x800000, 0, 0xFFFFFF);
   return (float)((s >> 8) - 0x8000) / 32768.0f;
}
static float ref1(int32_t v24, int volfix) { return ref_out(ref_mulsc(v24, volfix)); }

/* position after n frames at the given sample rate and pitch (non-looping) */
static long ref_pos(int sample_freq, int pitch, long frames)
{
   int f = pitch == 1000 ? sample_freq : (sample_freq * pitch) / 1000;
   long long diff = ((long long)(f << 12) >> 4) / FREQ;
   return (long)((diff * frames) >> 8);
}

/* ------------------------------------------------------------ helpers */

/* 16-bit sample of constant signed values l (and r when stereo) */
static SAMPLE *make16(int stereo, int freq, int len, int l, int r)
{
   SAMPLE *s = create_sample(16, stereo, freq, len);
   int i;
   unsigned short *d;
   if (!s) return NULL;
   d = (unsigned short *)s->data;
   for (i = 0; i < len; i++) {
      if (stereo) {
         d[2 * i] = (unsigned short)((l & 0xFFFF) ^ 0x8000);
         d[2 * i + 1] = (unsigned short)((r & 0xFFFF) ^ 0x8000);
      } else {
         d[i] = (unsigned short)((l & 0xFFFF) ^ 0x8000);
      }
   }
   return s;
}

/* 8-bit sample of constant unsigned bytes l (and r) */
static SAMPLE *make8(int stereo, int freq, int len, int l, int r)
{
   SAMPLE *s = create_sample(8, stereo, freq, len);
   int i;
   unsigned char *d;
   if (!s) return NULL;
   d = (unsigned char *)s->data;
   for (i = 0; i < len; i++) {
      if (stereo) { d[2 * i] = (unsigned char)l; d[2 * i + 1] = (unsigned char)r; }
      else d[i] = (unsigned char)l;
   }
   return s;
}

static float buf[2 * 8192];

static void mix(int frames)
{
   while (frames > 0) {
      int n = frames > 8192 ? 8192 : frames;
      a4_mix(buf, n, FREQ);
      frames -= n;
   }
}

static int all_zero(const float *p, int n)
{
   int i;
   for (i = 0; i < n; i++)
      if (p[i] != 0.0f)
         return 0;
   return 1;
}

static void reinstall(void)
{
   remove_sound();
   CHECK(install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) == 0, "install_sound");
}

/* ------------------------------------------------------------ mixer tests */

static void test_amplitude_and_pan(void)
{
   /* 16-bit mono, +0x4000 -> 24-bit +0x400000 */
   SAMPLE *s = make16(0, FREQ, 4000, 0x4000, 0);
   int v, pan;
   reinstall();

   v = play_sample(s, 255, 128, 1000, 0);
   CHECK(v == 0, "first voice number %d", v);
   a4_mix(buf, 64, FREQ);
   CHECK(buf[0] == ref1(0x400000, ref_lvol(255, 128)), "centre L %f vs %f", buf[0], ref1(0x400000, ref_lvol(255, 128)));
   CHECK(buf[1] == ref1(0x400000, ref_rvol(255, 128)), "centre R %f vs %f", buf[1], ref1(0x400000, ref_rvol(255, 128)));
   CHECK(buf[126] == buf[0] && buf[127] == buf[1], "constant output");
   CHECK(buf[0] > 0.248f && buf[0] < 0.25f, "centre pan is about -6 dB (%f)", buf[0]);
   CHECK(voice_get_position(v) == 64, "position after 64 frames: %d", voice_get_position(v));
   stop_sample(s);
   CHECK(voice_get_position(v) == -1, "stopped voice position");
   a4_mix(buf, 64, FREQ);
   CHECK(all_zero(buf, 128), "silence after stop_sample");

   for (pan = 0; pan <= 255; pan += 255) {
      v = play_sample(s, 200, pan, 1000, 0);
      a4_mix(buf, 16, FREQ);
      CHECK(buf[0] == ref1(0x400000, ref_lvol(200, pan)), "pan %d L", pan);
      CHECK(buf[1] == ref1(0x400000, ref_rvol(200, pan)), "pan %d R", pan);
      CHECK(pan == 0 ? buf[1] == 0.0f : buf[0] == 0.0f, "hard pan %d silences the other side", pan);
      stop_sample(s);
   }
   destroy_sample(s);
}

static void test_formats(void)
{
   SAMPLE *s8m = make8(0, FREQ, 1000, 0xC0, 0);          /* +0x400000 */
   SAMPLE *s8s = make8(1, FREQ, 1000, 0xC0, 0x20);       /* +0x400000 / -0x600000 */
   SAMPLE *s16s = make16(1, FREQ, 1000, -0x2000, 0x7000);/* -0x200000 / +0x700000 */
   reinstall();

   play_sample(s8m, 255, 0, 1000, 0);
   a4_mix(buf, 8, FREQ);
   CHECK(buf[0] == ref1(0x400000, ref_lvol(255, 0)) && buf[1] == 0.0f, "8-bit mono");
   stop_sample(s8m);

   play_sample(s8s, 255, 128, 1000, 0);
   a4_mix(buf, 8, FREQ);
   CHECK(buf[0] == ref1(0x400000, ref_lvol(255, 128)), "8-bit stereo L %f", buf[0]);
   CHECK(buf[1] == ref1(-0x600000, ref_rvol(255, 128)), "8-bit stereo R %f", buf[1]);
   CHECK(buf[0] > 0 && buf[1] < 0, "8-bit stereo channels are independent");
   stop_sample(s8s);

   play_sample(s16s, 255, 128, 1000, 0);
   a4_mix(buf, 8, FREQ);
   CHECK(buf[0] == ref1(-0x200000, ref_lvol(255, 128)), "16-bit stereo L %f", buf[0]);
   CHECK(buf[1] == ref1(0x700000, ref_rvol(255, 128)), "16-bit stereo R %f", buf[1]);

   /* two voices add */
   play_sample(s8m, 255, 128, 1000, 0);
   a4_mix(buf, 8, FREQ);
   CHECK(buf[0] == ref_out(ref_mulsc(-0x200000, ref_lvol(255, 128)) + ref_mulsc(0x400000, ref_lvol(255, 128))),
         "two voices sum");
   destroy_sample(s8m); destroy_sample(s8s); destroy_sample(s16s);
}

static void test_interpolation(void)
{
   /* 16-bit mono ramp 0, 0x100, 0x200 ... played at half speed: odd output
    * frames fall halfway between two input frames */
   SAMPLE *s = create_sample(16, 0, FREQ, 64);
   unsigned short *d = (unsigned short *)s->data;
   int i, v;
   reinstall();
   for (i = 0; i < 64; i++)
      d[i] = (unsigned short)((i * 0x100) ^ 0x8000);
   v = play_sample(s, 255, 255, 500, 0);
   a4_mix(buf, 8, FREQ);
   for (i = 0; i < 8; i++) {
      int32_t a = (int32_t)(i / 2) * 0x100 * 256, b = (int32_t)(i / 2 + 1) * 0x100 * 256;
      int32_t vv = (i & 1) ? (b * 128 + a * 128) >> 8 : a;
      CHECK(buf[2 * i + 1] == ref1(vv, ref_rvol(255, 255)), "interpolated frame %d: %f vs %f",
            i, buf[2 * i + 1], ref1(vv, ref_rvol(255, 255)));
   }
   CHECK(voice_get_position(v) == 4, "half speed position %d", voice_get_position(v));
   destroy_sample(s);
}

static void test_pitch_and_rate(void)
{
   SAMPLE *a = make16(0, FREQ, 200000, 0x1000, 0);
   SAMPLE *b = make16(0, 22050, 200000, 0x1000, 0);
   int va, vb, vc, vd;
   reinstall();

   va = play_sample(a, 255, 128, 1000, 0);
   vb = play_sample(a, 255, 128, 2000, 0);
   vc = play_sample(b, 255, 128, 1000, 0);
   vd = play_sample(a, 255, 128, 925, 0);
   mix(10000);
   CHECK(voice_get_position(va) == 10000, "freq 1000: %d", voice_get_position(va));
   CHECK(voice_get_position(vb) == 20000, "freq 2000: %d", voice_get_position(vb));
   CHECK(voice_get_position(vc) == 5000, "22050 Hz sample: %d", voice_get_position(vc));
   CHECK(voice_get_position(vd) == ref_pos(FREQ, 925, 10000), "freq 925: %d vs %ld",
         voice_get_position(vd), ref_pos(FREQ, 925, 10000));
   CHECK(ref_pos(FREQ, 925, 10000) == 9218, "Allegro's truncated step for 925 (%ld)", ref_pos(FREQ, 925, 10000));
   destroy_sample(a); destroy_sample(b);
}

static void test_loop_and_end(void)
{
   SAMPLE *s = make16(0, FREQ, 100, 0x4000, 0);
   int v, i, last, wraps = 0, ok = 1;
   reinstall();

   /* non-looping: plays 100 frames, then stops and frees its position */
   v = play_sample(s, 255, 0, 1000, 0);
   a4_mix(buf, 150, FREQ);
   CHECK(buf[2 * 99] == ref1(0x400000, ref_lvol(255, 0)), "last frame is still mixed");
   CHECK(all_zero(buf + 200, 100), "silence after the end");
   CHECK(voice_get_position(v) == -1, "finished voice position %d", voice_get_position(v));
   /* the finished voice still owns the sample (Allegro frees it lazily), so
    * adjust_sample() would hit it first; release it */
   stop_sample(s);

   /* looping: wraps back to loop_start */
   v = play_sample(s, 255, 0, 1000, 1);
   a4_mix(buf, 250, FREQ);
   CHECK(voice_get_position(v) == 50, "looped position %d", voice_get_position(v));
   CHECK(!all_zero(buf + 2 * 240, 20), "loop keeps playing");
   last = voice_get_position(v);
   for (i = 0; i < 1000; i++) {
      int p;
      a4_mix(buf, 1, FREQ);
      p = voice_get_position(v);
      if (p < last) { wraps++; if (p != 0) ok = 0; }
      else if (p != last + 1) ok = 0;
      last = p;
   }
   CHECK(ok && wraps == 10, "monotonic between wraps (wraps %d)", wraps);
   CHECK(voice_get_position(v) == 50, "position after 1250 frames %d", voice_get_position(v));

   /* adjust_sample removes the loop flag: the voice stops at the end */
   adjust_sample(s, 255, 0, 1000, 0);
   a4_mix(buf, 60, FREQ);
   CHECK(voice_get_position(v) == -1, "unlooped voice ends");
   destroy_sample(s);
}

static void test_silent_voice_tracks_time(void)
{
   /* the game's music + anti-cheat pattern: an audible loop and a silent
    * copy of the same sample, sampled every 20 ms tick */
   SAMPLE *bg = make16(1, FREQ, 3 * FREQ + 17, 0x100, -0x100);
   int music, check, i, ok = 1, total = 0, prev = 0, mono = 1;
   static const int chunks[] = { 1024, 441, 7, 2048, 882, 1 };
   reinstall();

   music = play_sample(bg, 200, 128, 1000, 1);
   check = play_sample(bg, 0, 128, 1000, 1);
   CHECK(music != check && music >= 0 && check >= 0, "two voices");
   for (i = 0; i < 600; i++) {
      int n = chunks[i % 6], pm, pc;
      a4_mix(buf, n, FREQ);
      total += n;
      pm = voice_get_position(music);
      pc = voice_get_position(check);
      if (pm != pc || pc != total % (int)bg->len) ok = 0;
      if (pc < prev && prev + n < (int)bg->len) mono = 0;
      prev = pc;
   }
   CHECK(ok, "silent voice position == audible voice position == frames mod len");
   CHECK(mono, "no backwards step except on wrap");
   /* the silent copy contributes nothing */
   stop_sample(bg);
   check = play_sample(bg, 0, 128, 1000, 1);
   a4_mix(buf, 512, FREQ);
   CHECK(all_zero(buf, 1024), "volume 0 voice is silent");
   CHECK(voice_get_position(check) == 512, "volume 0 voice advances (%d)", voice_get_position(check));
   /* voice_stop (focus loss) freezes the position report at -1 */
   voice_stop(check);
   CHECK(voice_get_position(check) == -1, "stopped voice");
   destroy_sample(bg);
}

static void test_adjust_and_volume(void)
{
   SAMPLE *s = make16(0, FREQ, 10000, 0x4000, 0);
   SAMPLE *t = make16(0, FREQ, 10000, 0x4000, 0);
   int v;
   reinstall();

   v = play_sample(s, 255, 128, 1000, 1);
   adjust_sample(s, 100, 30, 2000, 1);
   a4_mix(buf, 10, FREQ);
   CHECK(buf[0] == ref1(0x400000, ref_lvol(100, 30)) && buf[1] == ref1(0x400000, ref_rvol(100, 30)),
         "adjust_sample volume/pan");
   CHECK(voice_get_position(v) == 20, "adjust_sample pitch (%d)", voice_get_position(v));
   adjust_sample(t, 0, 0, 1000, 0);              /* not playing: no effect */
   a4_mix(buf, 1, FREQ);
   CHECK(buf[0] == ref1(0x400000, ref_lvol(100, 30)), "adjust of another sample has no effect");

   /* set_volume scales existing voices and new ones */
   adjust_sample(s, 255, 128, 1000, 1);
   set_volume(128, -1);
   a4_mix(buf, 1, FREQ);
   CHECK(buf[0] == ref1(0x400000, ref_lvol(128, 128)), "set_volume rescales playing voices");
   play_sample(t, 255, 128, 1000, 1);
   a4_mix(buf, 1, FREQ);
   CHECK(buf[0] == ref_out(2 * ref_mulsc(0x400000, ref_lvol(128, 128))), "set_volume scales new voices");
   set_volume(255, 77);
   a4_mix(buf, 1, FREQ);
   CHECK(buf[0] == ref_out(2 * ref_mulsc(0x400000, ref_lvol(255, 128))), "set_volume back to full");
   set_volume(0, -1);
   a4_mix(buf, 4, FREQ);
   CHECK(all_zero(buf, 8), "digital volume 0");
   destroy_sample(s); destroy_sample(t);
   reinstall();
}

static void test_voice_allocation(void)
{
   SAMPLE *shortspl = make16(0, FREQ, 50, 0x100, 0);
   SAMPLE *longspl[10];
   SAMPLE *loop = make16(0, FREQ, 1000, 0x100, 0);
   int v[12], i, w;
   reinstall();

   for (i = 0; i < 10; i++)
      longspl[i] = make16(0, FREQ, 100000, 0x100, 0);

   /* a finished one-shot keeps its virtual voice until reused */
   v[0] = play_sample(shortspl, 255, 128, 1000, 0);
   a4_mix(buf, 100, FREQ);
   CHECK(voice_get_position(v[0]) == -1, "one-shot finished");
   v[1] = play_sample(longspl[0], 255, 128, 1000, 0);
   CHECK(v[0] == 0 && v[1] == 1, "voice numbers %d %d", v[0], v[1]);
   stop_sample(shortspl);
   w = play_sample(longspl[1], 255, 128, 1000, 0);
   CHECK(w == 0, "virtual voice 0 reused after stop_sample (%d)", w);
   stop_sample(longspl[0]); stop_sample(longspl[1]);

   /* 8 physical voices: a loop and 7 one-shots fill them; the 9th one-shot
    * steals a one-shot (looping voices score lower), not the loop */
   v[0] = play_sample(loop, 255, 128, 1000, 1);
   for (i = 1; i < 8; i++)
      v[i] = play_sample(longspl[i], 255, 128, 1000, 0);
   a4_mix(buf, 10, FREQ);
   for (i = 0; i < 8; i++)
      CHECK(voice_get_position(v[i]) == 10, "voice %d playing", i);
   v[8] = play_sample(longspl[8], 255, 128, 1000, 0);
   CHECK(v[8] == 8, "9th virtual voice %d", v[8]);
   CHECK(voice_get_position(v[0]) == 10, "loop survives");
   CHECK(voice_get_position(v[1]) == -1, "oldest one-shot was stolen");
   CHECK(voice_get_position(v[8]) == 0, "new voice starts at 0");
   for (i = 2; i < 8; i++)
      CHECK(voice_get_position(v[i]) == 10, "voice %d untouched", i);

   /* a stopped (autokill) voice is reclaimed before stealing */
   voice_stop(v[5]);
   v[9] = play_sample(longspl[9], 255, 128, 1000, 0);
   for (i = 2; i < 8; i++)
      if (i != 5)
         CHECK(voice_get_position(v[i]) == 10, "voice %d untouched by reclaim", i);
   CHECK(voice_get_position(v[9]) == 0, "reclaimed voice plays");

   /* destroying a playing sample stops its voices */
   destroy_sample(loop);
   CHECK(voice_get_position(v[0]) == -1, "destroyed sample's voice stopped");
   a4_mix(buf, 100, FREQ);   /* must not touch freed data */

   for (i = 0; i < 10; i++)
      destroy_sample(longspl[i]);
   destroy_sample(shortspl);
   a4_mix(buf, 100, FREQ);
   CHECK(all_zero(buf, 200), "all voices gone");
}

static void test_clipping(void)
{
   SAMPLE *hi = make16(0, FREQ, 1000, 0x7FFF, 0);
   SAMPLE *lo = make16(0, FREQ, 1000, -0x8000, 0);
   int i, k, ok = 1;
   reinstall();
   for (i = 0; i < 8; i++)
      play_sample(hi, 255, 0, 1000, 1);
   a4_mix(buf, 32, FREQ);
   for (k = 0; k < 32; k++)
      if (buf[2 * k] != 32767.0f / 32768.0f || buf[2 * k + 1] != 0.0f) ok = 0;
   CHECK(ok, "positive clip to 32767/32768 (%f)", buf[0]);
   stop_sample(hi);
   for (i = 0; i < 8; i++)
      play_sample(lo, 255, 255, 1000, 1);
   a4_mix(buf, 32, FREQ);
   CHECK(buf[1] == -1.0f && buf[0] == 0.0f, "negative clip to -1 (%f)", buf[1]);
   for (k = 0; k < 64; k++)
      if (buf[k] < -1.0f || buf[k] > 1.0f) ok = 0;
   CHECK(ok, "output within [-1,1]");
   destroy_sample(hi); destroy_sample(lo);
}

static void test_bookkeeping_edges(void)
{
   SAMPLE *s = make16(0, FREQ, 100, 0x100, 0);
   int v;
   CHECK(voice_get_position(-1) == -1 && voice_get_position(100000) == -1, "invalid voices");
   voice_stop(-1);
   voice_stop(4096);
   stop_sample(NULL);
   adjust_sample(NULL, 1, 2, 3, 4);
   destroy_sample(NULL);
   CHECK(play_sample(NULL, 255, 128, 1000, 0) == -1, "NULL sample");

   /* without installed sound (Allegro's digi_none): a virtual voice but no
    * playback position */
   remove_sound();
   v = play_sample(s, 255, 128, 1000, 0);
   CHECK(v >= 0, "virtual voice without sound (%d)", v);
   CHECK(voice_get_position(v) == -1, "no position without sound");
   stop_sample(s);
   remove_sound();
   CHECK(install_sound(DIGI_NONE, MIDI_NONE, NULL) == 0, "DIGI_NONE");
   v = play_sample(s, 255, 128, 1000, 0);
   CHECK(voice_get_position(v) == -1, "DIGI_NONE has no voices");
   stop_sample(s);

   /* MIDI stubs */
   CHECK(load_midi("x.mid") == NULL, "load_midi");
   CHECK(play_midi(NULL, 1) == 0, "play_midi(NULL)");
   stop_midi();
   destroy_midi(NULL);

   reinstall();
   destroy_sample(s);
}

/* ------------------------------------------------------------ WAV / VOC */

static void put16(FILE *f, int v) { fputc(v & 0xFF, f); fputc((v >> 8) & 0xFF, f); }
static void put32(FILE *f, long v) { put16(f, (int)(v & 0xFFFF)); put16(f, (int)((v >> 16) & 0xFFFF)); }

static const char *tmp_path(const char *name)
{
   static char p[4][1200];
   static int n;
   char *b = p[n++ & 3];
   snprintf(b, sizeof(p[0]), "%s/%s", tmpdir, name);
   return b;
}

/* RIFF WAVE with a LIST chunk before fmt, an 18-byte fmt chunk and a junk
 * chunk before data, like many real-world files */
static void write_wav(const char *path, int channels, int bits, int freq, const void *data, int bytes)
{
   FILE *f = fopen(path, "wb");
   if (!f) return;
   fwrite("RIFF", 1, 4, f); put32(f, 4 + 8 + 4 + 8 + 18 + 8 + 6 + 8 + bytes);
   fwrite("WAVE", 1, 4, f);
   fwrite("LIST", 1, 4, f); put32(f, 4); fwrite("INFO", 1, 4, f);
   fwrite("fmt ", 1, 4, f); put32(f, 18);
   put16(f, 1); put16(f, channels); put32(f, freq);
   put32(f, (long)freq * channels * bits / 8); put16(f, channels * bits / 8);
   put16(f, bits); put16(f, 0);
   fwrite("junk", 1, 4, f); put32(f, 6); fwrite("abcdef", 1, 6, f);
   fwrite("data", 1, 4, f); put32(f, bytes);
   fwrite(data, 1, (size_t)bytes, f);
   fclose(f);
}

static void test_wav(void)
{
   static const int chans[] = { 1, 2 };
   static const int bitss[] = { 8, 16 };
   int ci, bi, i;
   for (ci = 0; ci < 2; ci++) {
      for (bi = 0; bi < 2; bi++) {
         int ch = chans[ci], bits = bitss[bi], frames = 1234, freq = 11025 * (ci + 1) * (bi + 1);
         int n = frames * ch, bytes = n * bits / 8, ok = 1;
         unsigned char raw[1234 * 4];
         const char *path = tmp_path(bits == 8 ? (ch == 1 ? "t8m.wav" : "t8s.WAV") : (ch == 1 ? "t16m.wav" : "t16s.Wav"));
         SAMPLE *s;
         for (i = 0; i < n; i++) {
            if (bits == 8) raw[i] = (unsigned char)(i * 7);
            else { int v = (int16_t)(i * 997 - 30000); raw[2 * i] = (unsigned char)(v & 0xFF); raw[2 * i + 1] = (unsigned char)((v >> 8) & 0xFF); }
         }
         write_wav(path, ch, bits, freq, raw, bytes);
         s = (ci == 0) ? load_wav(path) : load_sample(path);
         CHECK(s != NULL, "load %s", path);
         if (!s) continue;
         CHECK(s->bits == bits && (s->stereo != 0) == (ch == 2) && s->freq == freq && s->priority == 128,
               "%s header: bits %d stereo %d freq %d", path, s->bits, s->stereo, s->freq);
         CHECK(s->len == (unsigned long)frames && s->loop_start == 0 && s->loop_end == (unsigned long)frames,
               "%s len %lu", path, s->len);
         for (i = 0; i < n; i++) {
            if (bits == 8) { if (((unsigned char *)s->data)[i] != raw[i]) ok = 0; }
            else {
               unsigned short want = (unsigned short)((raw[2 * i] | (raw[2 * i + 1] << 8)) ^ 0x8000);
               if (((unsigned short *)s->data)[i] != want) ok = 0;
            }
         }
         CHECK(ok, "%s data (16-bit converted to unsigned)", path);
         destroy_sample(s);
      }
   }

   /* odd 8-bit stereo data length: len rounds up, the bytes present are read */
   {
      unsigned char raw[7] = { 1, 2, 3, 4, 5, 6, 7 };
      SAMPLE *s;
      write_wav(tmp_path("odd.wav"), 2, 8, 8000, raw, 7);
      s = load_wav(tmp_path("odd.wav"));
      CHECK(s && s->len == 4 && !memcmp(s->data, raw, 7), "odd stereo data chunk");
      if (s) destroy_sample(s);
   }

   /* rejects */
   {
      FILE *f = fopen(tmp_path("bad.wav"), "wb");
      if (f) { fputs("RIFX....WAVEfmt ", f); fclose(f); }
      CHECK(load_wav(tmp_path("bad.wav")) == NULL, "bad RIFF header");
      CHECK(load_wav(tmp_path("missing.wav")) == NULL, "missing file");
      CHECK(load_sample(tmp_path("t8m.ogg")) == NULL, "unknown extension");
      CHECK(load_sample(tmp_path("noext")) == NULL, "no extension");
   }

   /* a loaded WAV plays through the mixer */
   {
      SAMPLE *s = load_sample(tmp_path("t16s.Wav"));
      int v;
      reinstall();
      v = s ? play_sample(s, 255, 128, 1000, 0) : -1;
      a4_mix(buf, 100, FREQ);
      CHECK(v >= 0 && voice_get_position(v) == ref_pos(s ? s->freq : 0, 1000, 100), "wav playback position");
      if (s) destroy_sample(s);
   }
}

static void test_voc(void)
{
   /* Creative Voice File, block type 1 (8-bit mono) */
   FILE *f = fopen(tmp_path("t.voc"), "wb");
   int i, n = 300;
   SAMPLE *s;
   if (!f) { CHECK(0, "write voc"); return; }
   fwrite("Creative Voice File\x1A", 1, 20, f);
   put16(f, 0x1A);                 /* header size */
   put16(f, 0x010A);               /* version */
   put16(f, 0x1129);               /* check */
   fputc(1, f);                    /* block type 1 */
   put16(f, (n + 2) & 0xFFFF); fputc((n + 2) >> 16, f);
   fputc(256 - 1000000 / 11025, f);/* time constant */
   fputc(0, f);                    /* 8-bit PCM */
   for (i = 0; i < n; i++)
      fputc((i * 3) & 0xFF, f);
   fputc(0, f);                    /* terminator */
   fclose(f);
   s = load_sample(tmp_path("t.voc"));
   CHECK(s && s->bits == 8 && !s->stereo && s->len == (unsigned long)n && s->freq == 1000000 / (1000000 / 11025),
         "voc header");
   if (s) {
      int ok = 1;
      for (i = 0; i < n; i++)
         if (((unsigned char *)s->data)[i] != ((i * 3) & 0xFF)) ok = 0;
      CHECK(ok, "voc data");
      destroy_sample(s);
   }
}

/* ------------------------------------------------------------ real device */

static void test_device(void)
{
   const char *e = getenv("A4_TEST_AUDIO_DEVICE");
   char *argv[4];
   SAMPLE *s;
   int v, p0, p1;
   uint64_t t0, t1;
   double expect;
   if (!e || strcmp(e, "1")) {
      printf("device test skipped (set A4_TEST_AUDIO_DEVICE=1)\n");
      return;
   }
   remove_sound();
   argv[0] = (char *)"test_sound"; argv[1] = (char *)"--user-dir"; argv[2] = tmpdir; argv[3] = NULL;
   if (!plat_init(3, argv)) { printf("device test: plat_init failed\n"); return; }
   install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);
   s = make16(1, FREQ, 10 * FREQ, 0, 0);
   v = play_sample(s, 0, 128, 1000, 1);
   plat_sleep_ns(200000000u);
   p0 = voice_get_position(v); t0 = plat_ticks_ns();
   plat_sleep_ns(1000000000u);
   p1 = voice_get_position(v); t1 = plat_ticks_ns();
   expect = (double)(t1 - t0) * 1e-9 * FREQ;
   if (p0 == 0 && p1 == 0) {
      printf("device test: no audio device (positions stay 0)\n");
   } else {
      printf("device test: %d frames in %.3f s (expected %.0f)\n", p1 - p0, (double)(t1 - t0) * 1e-9, expect);
      CHECK(p1 - p0 > expect - 0.05 * FREQ && p1 - p0 < expect + 0.05 * FREQ,
            "position follows real time: %d vs %.0f", p1 - p0, expect);
   }
   remove_sound();
   destroy_sample(s);
   plat_shutdown();
}

int main(int argc, char **argv)
{
   const char *t = argc > 1 ? argv[1] : getenv("TEMP");
   if (!t) t = getenv("TMPDIR");
   if (!t) t = "/tmp";
   snprintf(tmpdir, sizeof(tmpdir), "%s", t);

   CHECK(install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) == 0, "install_sound without a device");
   test_amplitude_and_pan();
   test_formats();
   test_interpolation();
   test_pitch_and_rate();
   test_loop_and_end();
   test_silent_voice_tracks_time();
   test_adjust_and_volume();
   test_voice_allocation();
   test_clipping();
   test_bookkeeping_edges();
   test_wav();
   test_voc();
   remove_sound();
   test_device();

   printf("test_sound: %d checks, %d failures\n", checks, failures);
   return failures != 0;
}

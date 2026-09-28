/*
 * The game's Ogg Vorbis sample loader (port/game/logg_port.c, the port of
 * third_party/recovered/logg.c's logg_load/logg_load_memory) over the
 * vendored libvorbis/libogg, creating SAMPLEs through the compat
 * create_sample(), then played through the compat mixer.
 *
 * The .ogg inputs are generated at test time with ffmpeg from WAV files this
 * test writes; without ffmpeg on PATH the test is skipped (exit 0).
 *
 * usage: test_logg [WORK_DIR]      (default: $TEMP / $TMPDIR / /tmp)
 */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <allegro.h>
#include "a4_internal.h"

SAMPLE *logg_load(const char *filename);
SAMPLE *logg_load_memory(void *pData, size_t iSize);

static int failures, checks;
static char dir[1024];

#define CHECK(cond, ...) do { checks++; if (!(cond)) { failures++; \
   printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

#ifdef _WIN32
#define NULLDEV "NUL"
#else
#define NULLDEV "/dev/null"
#endif

static void put16(FILE *f, int v) { fputc(v & 0xFF, f); fputc((v >> 8) & 0xFF, f); }
static void put32(FILE *f, long v) { put16(f, (int)(v & 0xFFFF)); put16(f, (int)((v >> 16) & 0xFFFF)); }

/* sine tones: 440 Hz left, 660 Hz right, amplitude 0.5 */
static int write_tone_wav(const char *path, int channels, int freq, int frames)
{
   FILE *f = fopen(path, "wb");
   int i, c;
   if (!f) return 0;
   fwrite("RIFF", 1, 4, f); put32(f, 36 + frames * channels * 2);
   fwrite("WAVEfmt ", 1, 8, f); put32(f, 16);
   put16(f, 1); put16(f, channels); put32(f, freq); put32(f, (long)freq * channels * 2);
   put16(f, channels * 2); put16(f, 16);
   fwrite("data", 1, 4, f); put32(f, frames * channels * 2);
   for (i = 0; i < frames; i++)
      for (c = 0; c < channels; c++)
         put16(f, (int)lrint(16384.0 * sin(2.0 * 3.14159265358979 * (c ? 660.0 : 440.0) * i / freq)));
   fclose(f);
   return 1;
}

static int encode(const char *wav, const char *ogg)
{
   char cmd[4096];
   snprintf(cmd, sizeof(cmd), "ffmpeg -nostdin -y -loglevel error -i \"%s\" -c:a libvorbis -q:a 4 \"%s\" > " NULLDEV " 2>&1", wav, ogg);
   if (system(cmd) == 0)
      return 1;
   snprintf(cmd, sizeof(cmd), "ffmpeg -nostdin -y -loglevel error -i \"%s\" -c:a vorbis -strict -2 \"%s\" > " NULLDEV " 2>&1", wav, ogg);
   return system(cmd) == 0;
}

/* RMS and zero crossings of one channel of an unsigned 16-bit sample */
static void analyse(const SAMPLE *s, int ch, double *rms, int *crossings)
{
   const unsigned short *d = (const unsigned short *)s->data;
   int nch = s->stereo ? 2 : 1;
   unsigned long i;
   double acc = 0;
   int prev = 0, n = 0;
   for (i = 0; i < s->len; i++) {
      int v = (int)d[i * nch + ch] - 0x8000;
      acc += (double)v * v;
      if (i > 0 && ((prev < 0) != (v < 0))) n++;
      prev = v;
   }
   *rms = sqrt(acc / (double)s->len);
   *crossings = n;
}

static void check_file(const char *name, int channels, int freq, int frames)
{
   char wav[1200], ogg[1200];
   SAMPLE *s, *m;
   FILE *f;
   long size;
   void *mem;
   int c;

   snprintf(wav, sizeof(wav), "%s/%s.wav", dir, name);
   snprintf(ogg, sizeof(ogg), "%s/%s.ogg", dir, name);
   if (!write_tone_wav(wav, channels, freq, frames) || !encode(wav, ogg)) {
      CHECK(0, "could not generate %s", ogg);
      return;
   }

   s = logg_load(ogg);
   CHECK(s != NULL, "logg_load %s", ogg);
   if (!s) return;
   CHECK(s->bits == 16 && s->stereo == (channels == 2) && s->freq == freq && s->priority == 128,
         "%s: bits %d stereo %d freq %d", name, s->bits, s->stereo, s->freq);
   CHECK(s->len == (unsigned long)frames && s->loop_start == 0 && s->loop_end == s->len,
         "%s: len %lu (expected %d)", name, s->len, frames);
   for (c = 0; c < channels; c++) {
      double rms, want = 16384.0 / sqrt(2.0);
      int zc, wantzc = (int)(2.0 * (c ? 660.0 : 440.0) * frames / freq);
      analyse(s, c, &rms, &zc);
      CHECK(fabs(rms - want) < 0.05 * want, "%s ch%d rms %.0f vs %.0f", name, c, rms, want);
      CHECK(abs(zc - wantzc) <= 4, "%s ch%d zero crossings %d vs %d", name, c, zc, wantzc);
   }

   /* the memory variant (used for the datafile's sounds) decodes the same */
   f = fopen(ogg, "rb");
   CHECK(f != NULL, "reopen %s", ogg);
   if (f) {
      fseek(f, 0, SEEK_END);
      size = ftell(f);
      fseek(f, 0, SEEK_SET);
      mem = malloc((size_t)size);
      if (mem && fread(mem, 1, (size_t)size, f) == (size_t)size) {
         m = logg_load_memory(mem, (size_t)size);
         CHECK(m && m->len == s->len && m->freq == s->freq && m->stereo == s->stereo &&
               !memcmp(m->data, s->data, s->len * 2 * (s->stereo ? 2 : 1)), "%s: logg_load_memory", name);
         if (m) destroy_sample(m);
      }
      free(mem);
      fclose(f);
   }

   /* play it looped through the mixer: one full pass returns to 0 */
   {
      static float out[2 * 4096];
      int v, left = frames, peak = 0, i;
      install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);
      v = play_sample(s, 255, 128, 1000, 1);
      while (left > 0) {
         int n = left > 4096 ? 4096 : left;
         a4_mix(out, n, freq);
         for (i = 0; i < 2 * n; i++)
            if (fabsf(out[i]) > 0.2f) peak = 1;
         left -= n;
      }
      CHECK(voice_get_position(v) == 0, "%s: looped back to 0 (%d)", name, voice_get_position(v));
      CHECK(peak, "%s: audible through the mixer", name);
      a4_mix(out, 1000, freq);
      CHECK(voice_get_position(v) == 1000, "%s: position 1000 (%d)", name, voice_get_position(v));
      remove_sound();
   }

   destroy_sample(s);
}

int main(int argc, char **argv)
{
   const char *t = argc > 1 ? argv[1] : getenv("TEMP");
   if (!t) t = getenv("TMPDIR");
   if (!t) t = "/tmp";
   snprintf(dir, sizeof(dir), "%s", t);

   if (system("ffmpeg -version > " NULLDEV " 2>&1") != 0) {
      printf("test_logg: SKIPPED (ffmpeg not found on PATH)\n");
      return 0;
   }
   check_file("a4_logg_stereo44", 2, 44100, 44100);
   check_file("a4_logg_mono22", 1, 22050, 30000);
   CHECK(logg_load("definitely/missing.ogg") == NULL, "missing file");

   printf("test_logg: %d checks, %d failures\n", checks, failures);
   return failures != 0;
}

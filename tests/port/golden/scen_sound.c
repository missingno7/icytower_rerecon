/*
 * Sound golden scenario: WAV/VOC loading (decoded SAMPLE fields and data).
 * The scenario writes its own input files to $TEMP (or the current
 * directory) so it needs no game assets.  Mixing is not compared here
 * (real Allegro has no mixer without a sound driver); the mixer is covered
 * by tests/port/unit/test_sound.c against the mixer.c formulas.
 * C89: this file is also built with the historical GCC 4.4 toolchain.
 */
#include <stdlib.h>
#include <string.h>
#include "golden.h"

static char scen_dir[900];

static const char *spath(const char *name)
{
   static char buf[4][1024];
   static int n;
   char *b = buf[n++ & 3];
   strcpy(b, scen_dir);
   strcat(b, "/");
   strcat(b, name);
   return b;
}

static void w16(FILE *f, int v) { fputc(v & 0xFF, f); fputc((v >> 8) & 0xFF, f); }
static void w32(FILE *f, long v) { w16(f, (int)(v & 0xFFFF)); w16(f, (int)((v >> 16) & 0xFFFF)); }

static void fmt_chunk(FILE *f, int tag, int channels, long freq, int bits, int size)
{
   int i;
   fwrite("fmt ", 1, 4, f); w32(f, size);
   w16(f, tag); w16(f, channels); w32(f, freq);
   w32(f, freq * channels * bits / 8); w16(f, channels * bits / 8); w16(f, bits);
   for (i = 16; i < size; i++)
      fputc(0, f);
}

/* data bytes: a deterministic pattern */
static void data_chunk(FILE *f, long bytes, long declared)
{
   long i;
   fwrite("data", 1, 4, f); w32(f, declared);
   for (i = 0; i < bytes; i++)
      fputc((int)((i * 37 + (i >> 8)) & 0xFF), f);
}

static void record(golden_ctx *g, const char *name, SAMPLE *s)
{
   char nm[128];
   long bytes;
   sprintf(nm, "%s.null", name);
   golden_int(g, nm, s == NULL);
   if (!s)
      return;
   sprintf(nm, "%s.bits", name);       golden_int(g, nm, s->bits);
   sprintf(nm, "%s.stereo", name);     golden_int(g, nm, s->stereo);
   sprintf(nm, "%s.freq", name);       golden_int(g, nm, s->freq);
   sprintf(nm, "%s.priority", name);   golden_int(g, nm, s->priority);
   sprintf(nm, "%s.len", name);        golden_int(g, nm, (long)s->len);
   sprintf(nm, "%s.loop_start", name); golden_int(g, nm, (long)s->loop_start);
   sprintf(nm, "%s.loop_end", name);   golden_int(g, nm, (long)s->loop_end);
   bytes = (long)s->len * (s->bits == 8 ? 1 : 2) * (s->stereo ? 2 : 1);
   /* an odd 8-bit stereo data chunk leaves the last byte uninitialised */
   if (s->bits == 8 && s->stereo && bytes > 0)
      bytes--;
   sprintf(nm, "%s.data", name);       golden_bytes(g, nm, s->data, bytes);
   destroy_sample(s);
}

typedef struct wav_case {
   const char *name;
   int tag, channels, bits;
   long freq;
   int fmt_size;
   long bytes, declared;
   int extra;          /* 1: LIST before fmt + junk before data, 2: data before fmt */
} wav_case;

static const wav_case cases[] = {
   { "m8.wav",       1, 1,  8, 11025, 16, 1000, 1000, 0 },
   { "m16.wav",      1, 1, 16, 22050, 16, 2000, 2000, 0 },
   { "s8.wav",       1, 2,  8, 22050, 16, 2000, 2000, 1 },
   { "s16.WAV",      1, 2, 16, 44100, 18, 4000, 4000, 1 },
   { "s8odd.wav",    1, 2,  8,  8000, 16,    7,    7, 0 },
   { "m16odd.wav",   1, 1, 16, 44100, 16,    9,    9, 0 },
   { "s16trunc.wav", 1, 2, 16, 44100, 16,  100,  400, 0 },
   { "m8trunc.wav",  1, 1,  8, 44100, 16,  100,  400, 0 },
   { "datafirst.wav",1, 2, 16, 48000, 16,  400,  400, 2 },
   { "ext.wav", 0xFFFE, 2, 16, 44100, 40,  400,  400, 0 },
   { "b24.wav",      1, 2, 24, 44100, 16,  600,  600, 0 },
   { "c3.wav",       1, 3, 16, 44100, 16,  600,  600, 0 },
   { "empty.wav",    1, 1, 16, 44100, 16,    0,    0, 0 }
};

static void write_case(const wav_case *c)
{
   FILE *f = fopen(spath(c->name), "wb");
   if (!f)
      return;
   fwrite("RIFF", 1, 4, f); w32(f, 1000);
   fwrite("WAVE", 1, 4, f);
   if (c->extra == 1) {
      fwrite("LIST", 1, 4, f); w32(f, 4); fwrite("INFO", 1, 4, f);
   }
   if (c->extra == 2) {
      data_chunk(f, c->bytes, c->declared);
      fmt_chunk(f, c->tag, c->channels, c->freq, c->bits, c->fmt_size);
   }
   else {
      fmt_chunk(f, c->tag, c->channels, c->freq, c->bits, c->fmt_size);
      if (c->extra == 1) {
         fwrite("junk", 1, 4, f); w32(f, 6); fwrite("abcdef", 1, 6, f);
      }
      data_chunk(f, c->bytes, c->declared);
   }
   fclose(f);
}

static void write_voc(void)
{
   FILE *f = fopen(spath("t.voc"), "wb");
   int i;
   if (!f)
      return;
   fwrite("Creative Voice File\x1A", 1, 20, f);
   w16(f, 0x1A); w16(f, 0x010A); w16(f, 0x1129);
   fputc(1, f); w16(f, 302); fputc(0, f);
   fputc(256 - 1000000 / 11025, f); fputc(0, f);
   for (i = 0; i < 300; i++)
      fputc((i * 3) & 0xFF, f);
   fputc(0, f);
   fclose(f);
}

void scen_sound(golden_ctx *g)
{
   const char *t = getenv("TEMP");
   unsigned i;
   char nm[64];
   if (!t) t = getenv("TMPDIR");
   if (!t || strlen(t) > 800) t = ".";
   strcpy(scen_dir, t);

   for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
      write_case(&cases[i]);
      sprintf(nm, "wav.%s", cases[i].name);
      record(g, nm, load_wav(spath(cases[i].name)));
   }
   sprintf(nm, "sample.%s", cases[3].name);
   record(g, nm, load_sample(spath(cases[3].name)));
   write_voc();
   record(g, "sample.t.voc", load_sample(spath("t.voc")));
   record(g, "wav.missing", load_wav(spath("does-not-exist.wav")));
   record(g, "sample.unknown", load_sample(spath("m8.xyz")));
}

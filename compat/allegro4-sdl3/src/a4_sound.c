/*
 * Samples, voices, software mixer, WAV loading, MIDI stubs.
 * OWNER: sound worker.  Temporary stub: allocation only, silent playback.
 */
#include <stdlib.h>
#include "a4_internal.h"

SAMPLE *create_sample(int bits, int stereo, int freq, int len)
{
   SAMPLE *s = (SAMPLE *)calloc(1, sizeof(SAMPLE));
   if (!s) return NULL;
   s->bits = bits; s->stereo = stereo; s->freq = freq; s->priority = 128;
   s->len = (unsigned long)len; s->loop_start = 0; s->loop_end = (unsigned long)len;
   s->data = calloc(1, (size_t)len * (bits == 8 ? 1 : 2) * (stereo ? 2 : 1) + 4);
   if (!s->data) { free(s); return NULL; }
   return s;
}
void destroy_sample(SAMPLE *s) { if (s) { free(s->data); free(s); } }
int install_sound(int digi, int midi, const char *cfg) { (void)digi; (void)midi; (void)cfg; return 0; }
void remove_sound(void) { }
void set_volume(int d, int m) { (void)d; (void)m; }
SAMPLE *load_sample(const char *f) { (void)f; return NULL; }
SAMPLE *load_wav(const char *f) { (void)f; return NULL; }
SAMPLE *load_wav_pf(PACKFILE *f) { (void)f; return NULL; }
int play_sample(const SAMPLE *s, int v, int p, int f, int l) { (void)s; (void)v; (void)p; (void)f; (void)l; return -1; }
void adjust_sample(const SAMPLE *s, int v, int p, int f, int l) { (void)s; (void)v; (void)p; (void)f; (void)l; }
void stop_sample(const SAMPLE *s) { (void)s; }
void voice_stop(int v) { (void)v; }
int voice_get_position(int v) { (void)v; return -1; }
MIDI *load_midi(const char *f) { (void)f; return NULL; }
void destroy_midi(MIDI *m) { (void)m; }
int play_midi(MIDI *m, int l) { (void)m; (void)l; return 0; }
void stop_midi(void) { }
void a4_mix(float *out, int frames, int freq) { (void)out; (void)frames; (void)freq; }

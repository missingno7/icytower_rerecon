/*
 * Golden-image harness: the same scenario code is compiled twice,
 *   - against real Allegro 4.4.1 with the historical GCC 4.4 toolchain
 *     (main_allegro.c, built by tools/port/golden.py), and
 *   - against compat/allegro4-sdl3 with the modern toolchain
 *     (main_compat.c, CMake target a4_golden_compat).
 * Both write a stream of records; tools/port/golden.py compares them.
 * Scenario code may only use the API in compat/allegro4-sdl3/include/allegro.h.
 */
#ifndef GOLDEN_H
#define GOLDEN_H
#include <stdio.h>
#include <allegro.h>

typedef struct golden_ctx {
   const char *data_dir;     /* directory holding data/, characters/ (game assets) */
   FILE *out;
} golden_ctx;

/* record kinds */
#define GOLDEN_BITMAP 1
#define GOLDEN_BYTES  2
#define GOLDEN_INT    3

void golden_bitmap(golden_ctx *g, const char *name, BITMAP *bmp);
void golden_bytes(golden_ctx *g, const char *name, const void *p, long n);
void golden_int(golden_ctx *g, const char *name, long v);
/* path helper: data_dir + "/" + rel (static buffer rotated 4x) */
const char *golden_path(golden_ctx *g, const char *rel);

/* one file per area; each is owned by one implementer */
void scen_gfx(golden_ctx *g);     /* scen_gfx.c   : bitmaps, blits, sprites, primitives, blenders */
void scen_text(golden_ctx *g);    /* scen_text.c  : fonts and text output */
void scen_data(golden_ctx *g);    /* scen_data.c  : packfiles, datafiles, config */
void scen_sound(golden_ctx *g);   /* scen_sound.c : samples (decoded data only) */
#endif

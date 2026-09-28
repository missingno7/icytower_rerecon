/*
 * Display lists: resolution-independent recording of legacy canvas drawing.
 *
 * The historical code draws every screen in software into 640x480 bitmaps
 * (`swap_screen`, `screen` and a few canvas-sized temporaries).  Scaling
 * those pixels up is what the modern renderer must NOT depend on, so every
 * drawing call that targets a canvas bitmap is also recorded as an
 * operation (sprite/bitmap, fill, line, text, nested canvas, gameplay
 * underlay).  The modern renderer replays the list at output resolution:
 * sprites and glyphs are drawn from their original bitmaps as textures,
 * scaled once, with the configured filter.
 *
 * The software pixels are still produced as before (faithful renderer,
 * screenshots, read-backs); the list is an additional, lossless-where-
 * possible description of the same frame.  Anything the list cannot
 * express (XOR drawing, overflow, partial self-copies) marks it invalid,
 * and the renderer then shows that bitmap's pixels instead: correctness
 * never depends on the list.
 *
 * Rules:
 *  - canvas bitmaps (A4_BMP_CANVAS) are screen, swap_screen and any bitmap
 *    created at the canvas size; only they record;
 *  - a full-cover copy of one canvas into another copies the list (fades,
 *    background snapshots, blit_to_screen);
 *  - a canvas copied partially into another becomes a nested list;
 *  - a self-copy with a vertical offset (the game's screen shake) shifts
 *    the list;
 *  - operations reference their source bitmaps; a source destroyed before
 *    the list is replayed is cloned first (a4_dl_bitmap_dying);
 *  - a4_dl_set_underlay() replaces the list with a marker for the gameplay
 *    world, which the modern renderer draws from simulation snapshots.
 */
#ifndef A4_DL_H
#define A4_DL_H

#include "allegro.h"

#ifdef __cplusplus
extern "C" {
#endif

#define A4_BMP_CANVAS   0x0001u   /* records a display list */
#define A4_BMP_STATIC   0x0002u   /* loaded asset: pixels never change after load */
#define A4_BMP_CLONE    0x0004u   /* private copy owned by a display list */

enum {
   DLOP_BITMAP = 1,   /* src rect -> dst rect, optional flip/rotation/blend */
   DLOP_FILL,         /* solid or translucent rectangle */
   DLOP_LINE,         /* 1-pixel line (hline/vline/line/rect edge/putpixel) */
   DLOP_TEXT,         /* text run */
   DLOP_NESTED,       /* another (immutable) list, drawn at an offset */
   DLOP_UNDERLAY      /* gameplay world drawn from simulation snapshots */
};

enum {
   DLB_SOLID = 0,     /* opaque copy */
   DLB_MASKED,        /* mask-colour pixels transparent */
   DLB_TRANS,         /* masked, then blended with constant alpha (set_trans_blender) */
   DLB_ALPHA          /* per-pixel alpha from a 32-bit source (set_alpha_blender) */
};

typedef struct A4_DL A4_DL;

typedef struct a4_dl_op {
   uint8_t kind;
   uint8_t blend;            /* DLB_* for BITMAP; for FILL/LINE: DLB_SOLID or DLB_TRANS */
   uint8_t flip;             /* bit0 horizontal, bit1 vertical */
   uint8_t text_align;       /* unused (text is recorded already aligned) */
   int16_t cl, ct, cr, cb;   /* destination clip rectangle (right/bottom exclusive) */
   int32_t alpha;            /* DLB_TRANS: 0..255 */
   uint32_t rgb;             /* FILL/LINE/TEXT colour as 0xRRGGBB */
   int32_t text_bg;          /* TEXT background 0xRRGGBB or -1 */
   int32_t text_color_mode;  /* TEXT: -1 = colour glyphs as drawn, else solid colour */
   BITMAP *src;              /* BITMAP source */
   A4_DL *child;             /* NESTED */
   int32_t sx, sy, sw, sh;   /* source rectangle */
   float dx, dy, dw, dh;     /* destination rectangle (unrotated) */
   float angle;              /* rotation in degrees, clockwise, about (px, py) */
   float px, py;             /* rotation pivot in destination coordinates */
   int32_t x1, y1, x2, y2;   /* FILL (inclusive) / LINE endpoints */
   const FONT *font;
   char *text;
   uint64_t underlay;        /* UNDERLAY: snapshot generation */
} a4_dl_op;

struct A4_DL {
   int refs;                 /* >1: shared, copy before modifying */
   int n, cap;
   a4_dl_op *ops;
   int valid;                /* 0 = use the owning bitmap's pixels */
   int w, h;
   int shift_x, shift_y;     /* applied to the whole list (screen shake) */
   uint32_t version;         /* changes whenever the list changes */
};

/* lifetime hooks (a4_bitmap.c) */
void a4_dl_bitmap_created(BITMAP *b);
void a4_dl_bitmap_dying(BITMAP *b);
/* loaders wrap their work in these so their bitmaps become static assets */
void a4_dl_begin_static(void);
void a4_dl_end_static(void);
void a4_dl_mark_canvas(BITMAP *b);

/* recording (called by the drawing primitives before they draw) */
void a4_dl_blit(BITMAP *src, BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h, int blend);
void a4_dl_sprite(BITMAP *dst, BITMAP *spr, int x, int y, int flip, int blend);
void a4_dl_stretch(BITMAP *src, BITMAP *dst, int sx, int sy, int sw, int sh,
                   int dx, int dy, int dw, int dh, int masked);
void a4_dl_rotate(BITMAP *dst, BITMAP *spr, int x, int y, fixed angle, fixed scale);
void a4_dl_fill(BITMAP *dst, int x1, int y1, int x2, int y2, int color);
void a4_dl_line(BITMAP *dst, int x1, int y1, int x2, int y2, int color);
void a4_dl_text(BITMAP *dst, const FONT *f, const char *s, int x, int y, int color, int bg);
void a4_dl_clear(BITMAP *dst, int color);
void a4_dl_invalidate(BITMAP *dst);

/* gameplay underlay marker (port/game snapshot capture) */
void a4_dl_set_underlay(BITMAP *dst, uint64_t snapshot_generation);

/* consumers */
A4_DL *a4_dl_get(BITMAP *b);          /* NULL if b does not record */
void a4_dl_ref(A4_DL *dl);
void a4_dl_unref(A4_DL *dl);
/* renderer texture-cache eviction hook, called when a source bitmap dies */
extern void (*a4_dl_texture_evict)(BITMAP *b);
/* colour conversion helper for consumers: pixel of depth -> 0xRRGGBB */
uint32_t a4_dl_rgb(int depth, unsigned long c);
/* global switch: recording costs a little CPU, so the faithful renderer can
 * turn it off (lists then stay invalid) */
void a4_dl_enable(int on);
int  a4_dl_enabled(void);

#ifdef __cplusplus
}
#endif

#endif

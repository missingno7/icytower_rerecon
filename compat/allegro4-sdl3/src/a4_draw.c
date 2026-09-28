/*
 * Drawing modes, blenders and primitive drawing (hline, vline, line, rect,
 * rectfill).  Semantics follow Allegro 4.4.1 (colblend.c, c/cgfx.h,
 * graphics.c, gfx.c do_line).
 *
 * OWNER: graphics worker.  The state variables below are part of the
 * internal contract (a4_internal.h); a4_blend() must reproduce Allegro's
 * _blender_trans15/16/24/32 and _blender_alpha* results bit-exactly.
 */
#include "a4_internal.h"

int a4_draw_mode = DRAW_MODE_SOLID;
int a4_blend_kind = A4_BLEND_TRANS;
int a4_blend_r, a4_blend_g, a4_blend_b, a4_blend_a;

void drawing_mode(int mode, BITMAP *pattern, int x_anchor, int y_anchor)
{
   (void)pattern; (void)x_anchor; (void)y_anchor;
   a4_draw_mode = mode;
}

void solid_mode(void) { a4_draw_mode = DRAW_MODE_SOLID; }

void set_trans_blender(int r, int g, int b, int a)
{
   a4_blend_kind = A4_BLEND_TRANS;
   a4_blend_r = r; a4_blend_g = g; a4_blend_b = b; a4_blend_a = a;
}

void set_alpha_blender(void)
{
   a4_blend_kind = A4_BLEND_ALPHA;
}

unsigned long a4_blend(int depth, unsigned long src, unsigned long dst, unsigned long n)
{
   (void)depth; (void)dst; (void)n;
   return src;   /* TODO(graphics worker) */
}

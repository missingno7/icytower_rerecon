/*
 * Drawing modes, blenders and primitive drawing (hline, vline, line, rect,
 * rectfill).  Semantics follow Allegro 4.4.1 (giftware licence):
 * colblend.c (blenders), gfx.c (set_blender_mode, do_line, _normal_line,
 * _soft_rect, _normal_rectfill) and c/cgfx.h (_linear_hline/_linear_vline/
 * _linear_putpixel), as built with ALLEGRO_NO_ASM + ALLEGRO_USE_C.
 *
 * All blender arithmetic is done in uint32_t: Allegro's "unsigned long" was
 * 32 bits on Windows and the trans blenders rely on 32-bit wrap-around.
 *
 * Deviations (documented, unused by the game):
 *  - DRAW_MODE_TRANS at 8 bpp needs Allegro's color_map; it falls back to
 *    solid drawing here (the game never draws translucently at 8 bpp).
 *  - the pattern modes (COPY/SOLID/MASKED_PATTERN) draw solid.
 *  - with clipping switched off, drawing is still limited to the bitmap
 *    (Allegro would write out of bounds).
 */
#include <stdint.h>
#include "a4_internal.h"

int a4_draw_mode = DRAW_MODE_SOLID;
int a4_blend_kind = A4_BLEND_TRANS;
int a4_blend_r, a4_blend_g, a4_blend_b, a4_blend_a;

/* gfx.c drawing_mode(); pattern anchoring is not supported */
void drawing_mode(int mode, BITMAP *pattern, int x_anchor, int y_anchor)
{
   (void)pattern; (void)x_anchor; (void)y_anchor;
   a4_draw_mode = mode;
}

void solid_mode(void) { a4_draw_mode = DRAW_MODE_SOLID; }

/* gfx.c set_blender_mode() via colblend.c SET_BLENDER_FUNC(trans) */
void set_trans_blender(int r, int g, int b, int a)
{
   a4_blend_kind = A4_BLEND_TRANS;
   a4_blend_r = r; a4_blend_g = g; a4_blend_b = b; a4_blend_a = a;
}

/* colblend.c set_alpha_blender(): set_blender_mode_ex(..., 0, 0, 0, 0) */
void set_alpha_blender(void)
{
   a4_blend_kind = A4_BLEND_ALPHA;
   a4_blend_r = a4_blend_g = a4_blend_b = a4_blend_a = 0;
}

/* ---------------------------------------------------------------- blenders */

/* colblend.c _blender_trans24 (C version; also used for 32 bpp) */
static uint32_t blender_trans24(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t res, g;
   if (n)
      n++;
   res = ((x & 0xFF00FFu) - (y & 0xFF00FFu)) * n / 256 + y;
   y &= 0xFF00u;
   x &= 0xFF00u;
   g = (x - y) * n / 256 + y;
   res &= 0xFF00FFu;
   g &= 0xFF00u;
   return res | g;
}

/* colblend.c _blender_alpha32 */
static uint32_t blender_alpha32(uint32_t x, uint32_t y, uint32_t n)
{
   (void)n;
   return blender_trans24(x, y, (x >> 24) & 0xFF);
}

/* colblend.c _blender_trans16 */
static uint32_t blender_trans16(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t result;
   if (n)
      n = (n + 1) / 8;
   x = ((x & 0xFFFFu) | (x << 16)) & 0x7E0F81Fu;
   y = ((y & 0xFFFFu) | (y << 16)) & 0x7E0F81Fu;
   result = ((x - y) * n / 32 + y) & 0x7E0F81Fu;
   return (result & 0xFFFFu) | (result >> 16);
}

/* colblend.c _blender_trans15 */
static uint32_t blender_trans15(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t result;
   if (n)
      n = (n + 1) / 8;
   x = ((x & 0xFFFFu) | (x << 16)) & 0x3E07C1Fu;
   y = ((y & 0xFFFFu) | (y << 16)) & 0x3E07C1Fu;
   result = ((x - y) * n / 32 + y) & 0x3E07C1Fu;
   return (result & 0xFFFFu) | (result >> 16);
}

/* colblend.c _blender_alpha16_rgb (chosen by set_alpha_blender for the
 * Windows 5.6.5 layout) */
static uint32_t blender_alpha16_rgb(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t result;
   n = x >> 24;
   if (n)
      n = (n + 1) / 8;
   x = ((x >> 3) & 0x001Fu) | ((x >> 5) & 0x07E0u) | ((x >> 8) & 0xF800u);
   x = (x | (x << 16)) & 0x7E0F81Fu;
   y = ((y & 0xFFFFu) | (y << 16)) & 0x7E0F81Fu;
   result = ((x - y) * n / 32 + y) & 0x7E0F81Fu;
   return (result & 0xFFFFu) | (result >> 16);
}

/* colblend.c _blender_alpha15_rgb (including its 0xEC00 red mask) */
static uint32_t blender_alpha15_rgb(uint32_t x, uint32_t y, uint32_t n)
{
   uint32_t result;
   n = x >> 24;
   if (n)
      n = (n + 1) / 8;
   x = ((x >> 3) & 0x001Fu) | ((x >> 6) & 0x03E0u) | ((x >> 9) & 0xEC00u);
   x = (x | (x << 16)) & 0x3E07C1Fu;
   y = ((y & 0xFFFFu) | (y << 16)) & 0x3E07C1Fu;
   result = ((x - y) * n / 32 + y) & 0x3E07C1Fu;
   return (result & 0xFFFFu) | (result >> 16);
}

/* _blender_func<depth>: set_trans_blender installs trans15/trans16/trans24
 * (24 also for 32 bpp); set_alpha_blender installs _blender_black for
 * 15/16/24 and _blender_alpha32 for 32 bpp. */
unsigned long a4_blend(int depth, unsigned long src, unsigned long dst, unsigned long n)
{
   uint32_t x = (uint32_t)src, y = (uint32_t)dst, a = (uint32_t)n;
   if (a4_blend_kind == A4_BLEND_ALPHA)
      return depth == 32 ? blender_alpha32(x, y, a) : 0;
   switch (depth) {
      case 15: return blender_trans15(x, y, a);
      case 16: return blender_trans16(x, y, a);
      case 24:
      case 32: return blender_trans24(x, y, a);
   }
   return src;
}

/* _blender_func<depth>x: RGBA (32-bit source) onto 15/16/24 bpp, used by
 * draw_trans_sprite.  set_blender_mode() sets these to _blender_black. */
unsigned long a4_blend_rgba(int depth, unsigned long src, unsigned long dst, unsigned long n)
{
   uint32_t x = (uint32_t)src, y = (uint32_t)dst, a = (uint32_t)n;
   if (a4_blend_kind != A4_BLEND_ALPHA)
      return 0;
   switch (depth) {
      case 15: return blender_alpha15_rgb(x, y, a);
      case 16: return blender_alpha16_rgb(x, y, a);
      case 24: return blender_alpha32(x, y, a);   /* 24-bit RGB layout matches */
   }
   return 0;
}

/* ---------------------------------------------------------------- pixels */

/* one pixel in the current drawing mode, no clipping (cgfx.h putpixel body) */
static inline void plot_mode(BITMAP *bmp, int x, int y, unsigned long color)
{
   if (a4_draw_mode == DRAW_MODE_XOR) {
      a4_put_raw(bmp, x, y, a4_get_raw(bmp, x, y) ^ color);
   }
   else if (a4_draw_mode == DRAW_MODE_TRANS && bmp->depth != 8) {
      unsigned long d = a4_get_raw(bmp, x, y);
      a4_put_raw(bmp, x, y, a4_blend(bmp->depth, color, d, (unsigned long)a4_blend_a));
   }
   else {
      a4_put_raw(bmp, x, y, color);
   }
}

/* effective clip rectangle: the bitmap clip, or the whole bitmap */
static inline void clip_rect_of(BITMAP *bmp, int *cl, int *ct, int *cr, int *cb)
{
   if (bmp->clip) {
      *cl = bmp->cl; *ct = bmp->ct; *cr = bmp->cr; *cb = bmp->cb;
   }
   else {
      *cl = 0; *ct = 0; *cr = bmp->w; *cb = bmp->h;
   }
}

/* cgfx.h _linear_hline, without touching */
static int do_hline(BITMAP *bmp, int x1, int y, int x2, int color)
{
   int cl, ct, cr, cb, x;
   unsigned long c = (unsigned long)(uint32_t)color;
   if (x1 > x2) {
      int t = x1; x1 = x2; x2 = t;
   }
   clip_rect_of(bmp, &cl, &ct, &cr, &cb);
   if (x1 < cl)
      x1 = cl;
   if (x2 >= cr)
      x2 = cr - 1;
   if (x1 > x2 || y < ct || y >= cb)
      return 0;
   for (x = x1; x <= x2; x++)
      plot_mode(bmp, x, y, c);
   return 1;
}

/* cgfx.h _linear_vline, without touching */
static int do_vline(BITMAP *bmp, int x, int y1, int y2, int color)
{
   int cl, ct, cr, cb, y;
   unsigned long c = (unsigned long)(uint32_t)color;
   if (y1 > y2) {
      int t = y1; y1 = y2; y2 = t;
   }
   clip_rect_of(bmp, &cl, &ct, &cr, &cb);
   if (y1 < ct)
      y1 = ct;
   if (y2 >= cb)
      y2 = cb - 1;
   if (x < cl || x >= cr || y1 > y2)
      return 0;
   for (y = y1; y <= y2; y++)
      plot_mode(bmp, x, y, c);
   return 1;
}

void hline(BITMAP *bmp, int x1, int y, int x2, int color)
{
   if (do_hline(bmp, x1, y, x2, color))
      a4_touch(bmp);
}

void vline(BITMAP *bmp, int x, int y1, int y2, int color)
{
   if (do_vline(bmp, x, y1, y2, color))
      a4_touch(bmp);
}

/* ---------------------------------------------------------------- lines */

typedef struct {
   BITMAP *bmp;
   int cl, ct, cr, cb;
   int drawn;
} LINE_CTX;

static void line_plot(LINE_CTX *lc, int x, int y, int color)
{
   if (x < lc->cl || x >= lc->cr || y < lc->ct || y >= lc->cb)
      return;
   plot_mode(lc->bmp, x, y, (unsigned long)(uint32_t)color);
   lc->drawn = 1;
}

/* gfx.c do_line: Bresenham with Allegro's exact decision variables */
static void do_line_points(LINE_CTX *lc, int x1, int y1, int x2, int y2, int d)
{
   int dx = x2 - x1;
   int dy = y2 - y1;
   int i1, i2;
   int x, y;
   int dd;

#define DO_LINE(pri_sign, pri_c, pri_cond, sec_sign, sec_c, sec_cond)      \
   {                                                                        \
      if (d##pri_c == 0) {                                                  \
         line_plot(lc, x1, y1, d);                                          \
         return;                                                            \
      }                                                                     \
      i1 = 2 * d##sec_c;                                                    \
      dd = i1 - (sec_sign (pri_sign d##pri_c));                             \
      i2 = dd - (sec_sign (pri_sign d##pri_c));                             \
      x = x1;                                                               \
      y = y1;                                                               \
      while (pri_c pri_cond pri_c##2) {                                     \
         line_plot(lc, x, y, d);                                            \
         if (dd sec_cond 0) {                                               \
            sec_c = sec_c sec_sign 1;                                       \
            dd += i2;                                                       \
         }                                                                  \
         else                                                               \
            dd += i1;                                                       \
         pri_c = pri_c pri_sign 1;                                          \
      }                                                                     \
   }

   if (dx >= 0) {
      if (dy >= 0) {
         if (dx >= dy) {
            DO_LINE(+, x, <=, +, y, >=);
         }
         else {
            DO_LINE(+, y, <=, +, x, >=);
         }
      }
      else {
         if (dx >= -dy) {
            DO_LINE(+, x, <=, -, y, <=);
         }
         else {
            DO_LINE(-, y, >=, +, x, >=);
         }
      }
   }
   else {
      if (dy >= 0) {
         if (-dx >= dy) {
            DO_LINE(-, x, >=, +, y, >=);
         }
         else {
            DO_LINE(+, y, <=, -, x, <=);
         }
      }
      else {
         if (-dx >= -dy) {
            DO_LINE(-, x, >=, -, y, <=);
         }
         else {
            DO_LINE(-, y, >=, -, x, <=);
         }
      }
   }
#undef DO_LINE
}

/* gfx.c _normal_line */
void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   LINE_CTX lc;

   if (x1 == x2) {
      vline(bmp, x1, y1, y2, color);
      return;
   }
   if (y1 == y2) {
      hline(bmp, x1, y1, x2, color);
      return;
   }
   lc.bmp = bmp;
   clip_rect_of(bmp, &lc.cl, &lc.ct, &lc.cr, &lc.cb);
   lc.drawn = 0;
   /* bounding-box rejection as in _normal_line; per-pixel clipping gives
    * the same pixel set as its clip-off fast path */
   {
      int sx = x1 < x2 ? x1 : x2, dx = x1 < x2 ? x2 : x1;
      int sy = y1 < y2 ? y1 : y2, dy = y1 < y2 ? y2 : y1;
      if (sx >= lc.cr || sy >= lc.cb || dx < lc.cl || dy < lc.ct)
         return;
   }
   do_line_points(&lc, x1, y1, x2, y2, color);
   if (lc.drawn)
      a4_touch(bmp);
}

/* ---------------------------------------------------------------- rects */

/* gfx.c _soft_rect */
void rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   int t, drawn = 0;

   if (x2 < x1) {
      t = x1; x1 = x2; x2 = t;
   }
   if (y2 < y1) {
      t = y1; y1 = y2; y2 = t;
   }
   drawn |= do_hline(bmp, x1, y1, x2, color);
   if (y2 > y1)
      drawn |= do_hline(bmp, x1, y2, x2, color);
   if (y2 - 1 >= y1 + 1) {
      drawn |= do_vline(bmp, x1, y1 + 1, y2 - 1, color);
      if (x2 > x1)
         drawn |= do_vline(bmp, x2, y1 + 1, y2 - 1, color);
   }
   if (drawn)
      a4_touch(bmp);
}

/* gfx.c _normal_rectfill (hfill = _linear_hline) */
void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   int t, drawn = 0;

   if (y1 > y2) {
      t = y1; y1 = y2; y2 = t;
   }
   if (x1 > x2) {
      t = x1; x1 = x2; x2 = t;
   }
   {
      int cl, ct, cr, cb;
      clip_rect_of(bmp, &cl, &ct, &cr, &cb);
      if (x1 < cl)
         x1 = cl;
      if (x2 >= cr)
         x2 = cr - 1;
      if (x2 < x1)
         return;
      if (y1 < ct)
         y1 = ct;
      if (y2 >= cb)
         y2 = cb - 1;
      if (y2 < y1)
         return;
   }
   for (; y1 <= y2; y1++)
      drawn |= do_hline(bmp, x1, y1, x2, color);
   if (drawn)
      a4_touch(bmp);
}

/*
 * draw_sprite (+ h/v flips), draw_trans_sprite, rotate_sprite and
 * rotate_scaled_sprite.  Semantics follow Allegro 4.4.1 (giftware licence):
 * c/cspr.h (_linear_draw_sprite, _linear_draw_256_sprite, flips,
 * _linear_draw_trans_sprite, _linear_draw_trans_rgba_sprite), inline/draw.inl
 * (dispatch and rotate_* argument set-up) and rotate.c (_parallelogram_map,
 * _parallelogram_map_standard, _rotate_scale_flip_coordinates,
 * _pivot_scaled_sprite_flip).
 *
 * rotate.c evaluates a few expressions in x87 extended precision (the
 * historical library ran with a 64-bit mantissa); they are reproduced here
 * exactly with integer arithmetic (see x87_quot_trunc and orientation test).
 * The fixed sin/cos of the rotation angle use the host sin()/cos(): an
 * exhaustive check over all 2^24 reduced angles showed every value
 * 65536*sin/cos stays >= 6.5e-8 away from a rounding boundary, far above any
 * libm / fsincos / extended-vs-double difference, so the rounded fixed values
 * are identical.
 *
 * Deviations (documented, unused by the game):
 *  - draw_sprite / flips with a hi/truecolor sprite of another depth than the
 *    target (undefined in Allegro) convert the pixels.
 *  - draw_trans_sprite of an 8 bpp sprite onto an 8 bpp bitmap needs a
 *    color_map in Allegro; it is drawn as a plain masked sprite here, and a
 *    32 bpp sprite onto an 8 bpp bitmap (a NULL vtable entry) draws nothing.
 *  - with clipping switched off, drawing is still limited to the bitmap.
 */
#include <math.h>
#include <stdint.h>
#include "a4_internal.h"

enum { SPR_NORMAL, SPR_256, SPR_CONVERT, SPR_TRANS, SPR_TRANS_256, SPR_RGBA };

static void clip_rect_of(BITMAP *bmp, int *cl, int *ct, int *cr, int *cb)
{
   if (bmp->clip) {
      *cl = bmp->cl; *ct = bmp->ct; *cr = bmp->cr; *cb = bmp->cb;
   }
   else {
      *cl = 0; *ct = 0; *cr = bmp->w; *cb = bmp->h;
   }
}

/* The common cspr.h clipping and (backward) traversal:
 *   sxbeg = max(0, cl - dx), w = min(src->w, cr - dx) - sxbeg, ...
 *   flipped axes are drawn backwards onto dst from dxbeg + w - 1. */
static void sprite_generic(BITMAP *dst, BITMAP *src, int dx, int dy,
                           int h_flip, int v_flip, int kind)
{
   int x, y, w, h, tmp;
   int dxbeg, dybeg, sxbeg, sybeg, xdir, ydir;
   int cl, ct, cr, cb;
   unsigned long table[256];
   unsigned long smask = a4_mask_color(src->depth);
   int depth = dst->depth;

   clip_rect_of(dst, &cl, &ct, &cr, &cb);

   tmp = cl - dx;
   sxbeg = ((tmp < 0) ? 0 : tmp);
   dxbeg = sxbeg + dx;
   tmp = cr - dx;
   w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
   if (w <= 0)
      return;
   if (h_flip) {
      sxbeg = src->w - (sxbeg + w);
      dxbeg += w - 1;
   }

   tmp = ct - dy;
   sybeg = ((tmp < 0) ? 0 : tmp);
   dybeg = sybeg + dy;
   tmp = cb - dy;
   h = ((tmp > src->h) ? src->h : tmp) - sybeg;
   if (h <= 0)
      return;
   if (v_flip) {
      sybeg = src->h - (sybeg + h);
      dybeg += h - 1;
   }
   xdir = h_flip ? -1 : 1;
   ydir = v_flip ? -1 : 1;

   if (kind == SPR_256) {
      int c;
      for (c = 0; c < 256; c++)
         table[c] = (unsigned long)(uint32_t)a4_palette_color_depth(depth, c);
   }

   for (y = 0; y < h; y++) {
      int ddy = dybeg + y * ydir, ssy = sybeg + y;
      for (x = 0; x < w; x++) {
         int ddx = dxbeg + x * xdir;
         unsigned long c = a4_get_raw(src, sxbeg + x, ssy);
         switch (kind) {
            case SPR_NORMAL:
               if (c != smask)
                  a4_put_raw(dst, ddx, ddy, c);
               break;
            case SPR_256:
               /* _linear_draw_256_sprite: index 0 is transparent */
               if (c != 0)
                  a4_put_raw(dst, ddx, ddy, table[c & 0xFF]);
               break;
            case SPR_CONVERT:
               if (c != smask)
                  a4_put_raw(dst, ddx, ddy, a4_convert_color(c, src->depth, depth));
               break;
            case SPR_TRANS:
               /* _linear_draw_trans_sprite: DTS_BLEND(dst, src) with
                * _blender_func<depth> and _blender_alpha */
               if (c != smask)
                  a4_put_raw(dst, ddx, ddy,
                             a4_blend(depth, c, a4_get_raw(dst, ddx, ddy), (unsigned long)a4_blend_a));
               break;
            case SPR_TRANS_256:
               /* 8 bpp source branch of _linear_draw_trans_sprite: the raw
                * index is blended; IS_SPRITE_MASK only matches for 16 bpp
                * (src vtable mask 0) */
               if (depth == 16 && c == 0)
                  break;
               a4_put_raw(dst, ddx, ddy,
                          a4_blend(depth, c, a4_get_raw(dst, ddx, ddy), (unsigned long)a4_blend_a));
               break;
            case SPR_RGBA:
               /* _linear_draw_trans_rgba_sprite15/16/24 */
               if (c != MASK_COLOR_32)
                  a4_put_raw(dst, ddx, ddy,
                             a4_blend_rgba(depth, c, a4_get_raw(dst, ddx, ddy), (unsigned long)a4_blend_a));
               break;
         }
      }
   }
   a4_touch(dst);
}

static int plain_kind(BITMAP *bmp, BITMAP *sprite)
{
   if (sprite->depth == bmp->depth)
      return SPR_NORMAL;
   if (sprite->depth == 8)
      return SPR_256;
   return SPR_CONVERT;
}

/* draw.inl draw_sprite: 8 bpp sprites go through draw_256_sprite */
void draw_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y)
{
   sprite_generic(bmp, sprite, x, y, 0, 0, plain_kind(bmp, sprite));
}

void draw_sprite_h_flip(BITMAP *bmp, BITMAP *sprite, int x, int y)
{
   sprite_generic(bmp, sprite, x, y, 1, 0, plain_kind(bmp, sprite));
}

void draw_sprite_v_flip(BITMAP *bmp, BITMAP *sprite, int x, int y)
{
   sprite_generic(bmp, sprite, x, y, 0, 1, plain_kind(bmp, sprite));
}

/* draw.inl draw_trans_sprite */
void draw_trans_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y)
{
   if (sprite->depth == 32) {
      /* draw_trans_rgba_sprite: for a 32 bpp target that is
       * _linear_draw_trans_sprite32 itself */
      if (bmp->depth == 32)
         sprite_generic(bmp, sprite, x, y, 0, 0, SPR_TRANS);
      else if (bmp->depth != 8)
         sprite_generic(bmp, sprite, x, y, 0, 0, SPR_RGBA);
   }
   else if (sprite->depth == bmp->depth) {
      sprite_generic(bmp, sprite, x, y, 0, 0, bmp->depth == 8 ? SPR_NORMAL : SPR_TRANS);
   }
   else if (sprite->depth == 8) {
      sprite_generic(bmp, sprite, x, y, 0, 0, SPR_TRANS_256);
   }
   else {
      sprite_generic(bmp, sprite, x, y, 0, 0, SPR_CONVERT);
   }
}

/* ------------------------------------------------------------ rotation */

static inline int32_t wrap_sub(int32_t a, int32_t b) { return (int32_t)((uint32_t)a - (uint32_t)b); }
static inline int32_t wrap_add(int32_t a, int32_t b) { return (int32_t)((uint32_t)a + (uint32_t)b); }
static inline int32_t shl16(int32_t a) { return (int32_t)((uint32_t)a << 16); }

/* (fixed)( num / den ) as evaluated by the x87 with a 64-bit mantissa, where
 * num = a * 65536.0 * (65536.0 * w) and
 * den = p * (double)q - r * (double)s   (int32 operands).
 * Both operands are exact in extended precision (|num| = |a*w|*2^32 with
 * |a*w| <= 2^62, |den| <= 2^63), so the result is trunc(round64(num/den)),
 * and an out-of-range or undefined result is the x87 integer indefinite.
 * Computed with portable 64-bit integer long division. */
static fixed x87_quot_trunc(int32_t a, int w, int32_t p, int32_t q, int32_t r, int32_t s)
{
   int64_t pa = (int64_t)p * q, pb = (int64_t)r * s, m = (int64_t)a * w;
   uint64_t d, mm, rem;
   uint32_t t;
   int den_neg, i, e, k;

   /* |den| and its sign without overflow (|pa|, |pb| <= 2^62) */
   if ((pa >= 0) == (pb >= 0)) {
      int64_t dd = pa - pb;
      den_neg = dd < 0;
      d = den_neg ? (uint64_t)0 - (uint64_t)dd : (uint64_t)dd;
   }
   else if (pa >= 0) {
      den_neg = 0;
      d = (uint64_t)pa + ((uint64_t)0 - (uint64_t)pb);
   }
   else {
      den_neg = 1;
      d = ((uint64_t)0 - (uint64_t)pa) + (uint64_t)pb;
   }
   if (d == 0)
      return INT32_MIN;          /* +-inf or NaN */
   if (m == 0)
      return 0;
   mm = m < 0 ? (uint64_t)0 - (uint64_t)m : (uint64_t)m;   /* <= 2^62 */

   /* |num/den| = mm * 2^32 / d >= 2^31  <=>  2 * mm >= d */
   if (2 * mm >= d)
      return INT32_MIN;

   /* t = floor(mm * 2^32 / d) by long division (mm < d here) */
   t = 0;
   rem = mm;
   for (i = 0; i < 32; i++) {
      rem <<= 1;                 /* rem < d <= 2^63 */
      t <<= 1;
      if (rem >= d) {
         rem -= d;
         t |= 1;
      }
   }

   /* round64(q) reaches t+1 iff (t+1) - q <= 2^(e-64), e = floor(log2 t),
    * i.e. (d - rem) * 2^(64-e) <= d; ties go to the even t+1.  Impossible
    * for t < 2 because d <= 2^63. */
   if (rem != 0 && t >= 2) {
      e = 0;
      while ((t >> (e + 1)) != 0)
         e++;
      k = 64 - e;                /* 34..63 */
      if ((d - rem) <= (d >> k)) {
         t++;
         if (t >= 0x80000000u)
            return INT32_MIN;
      }
   }
   return ((m < 0) != den_neg) ? -(fixed)t : (fixed)t;
}

typedef void (*SCANLINE_FN)(BITMAP *bmp, BITMAP *spr, fixed l_bmp_x, int bmp_y_i,
                            fixed r_bmp_x, fixed l_spr_x, fixed l_spr_y,
                            fixed spr_dx, fixed spr_dy);

/* rotate.c SCANLINE_DRAWER(bits_pp): same depth, raw copy, depth mask */
static void draw_scanline_same(BITMAP *bmp, BITMAP *spr, fixed l_bmp_x, int bmp_y_i,
                               fixed r_bmp_x, fixed l_spr_x, fixed l_spr_y,
                               fixed spr_dx, fixed spr_dy)
{
   unsigned long mask = a4_mask_color(bmp->depth);
   int x, xe;
   xe = r_bmp_x >> 16;
   for (x = l_bmp_x >> 16; x <= xe; x++) {
      int sx = l_spr_x >> 16, sy = l_spr_y >> 16;
      if (sx >= 0 && sy >= 0 && sx < spr->w && sy < spr->h) {
         unsigned long c = a4_get_raw(spr, sx, sy);
         if (c != mask)
            a4_put_raw(bmp, x, bmp_y_i, c);
      }
      l_spr_x = wrap_add(l_spr_x, spr_dx);
      l_spr_y = wrap_add(l_spr_y, spr_dy);
   }
}

/* rotate.c SCANLINE_DRAWER_GENERIC(generic_convert): getpixel (-1 outside),
 * compared with the *destination* mask colour, putpixel in solid mode */
static void draw_scanline_convert(BITMAP *bmp, BITMAP *spr, fixed l_bmp_x, int bmp_y_i,
                                  fixed r_bmp_x, fixed l_spr_x, fixed l_spr_y,
                                  fixed spr_dx, fixed spr_dy)
{
   int mask = (int)a4_mask_color(bmp->depth);
   int bd = bmp->depth, sd = spr->depth;
   int cl, ct, cr, cb, x, xe;
   clip_rect_of(bmp, &cl, &ct, &cr, &cb);
   xe = r_bmp_x >> 16;
   for (x = l_bmp_x >> 16; x <= xe; x++) {
      int c = getpixel(spr, l_spr_x >> 16, l_spr_y >> 16);
      if (c != mask && x >= cl && x < cr && bmp_y_i >= ct && bmp_y_i < cb)
         a4_put_raw(bmp, x, bmp_y_i,
                    (unsigned long)(uint32_t)makecol_depth(bd, getr_depth(sd, c),
                                                           getg_depth(sd, c),
                                                           getb_depth(sd, c)));
      l_spr_x = wrap_add(l_spr_x, spr_dx);
      l_spr_y = wrap_add(l_spr_y, spr_dy);
   }
}

/* rotate.c _parallelogram_map (sub_pixel_accuracy = FALSE) */
static void parallelogram_map(BITMAP *bmp, BITMAP *spr, fixed xs[4], fixed ys[4],
                              SCANLINE_FN draw_scanline)
{
   int top_index, right_index, index, i;
   fixed corner_bmp_x[4], corner_bmp_y[4];
   fixed corner_spr_x[4], corner_spr_y[4];
   int clip_bottom_i, l_bmp_y_bottom_i, r_bmp_y_bottom_i;
   fixed clip_left, clip_right;
   fixed extra_scanline_fraction;
   fixed l_spr_x, l_spr_y, l_bmp_x, l_bmp_dx;
   fixed l_spr_dx, l_spr_dy;
   fixed r_bmp_x, r_bmp_dx;
   fixed spr_dx, spr_dy;
   fixed l_spr_x_rounded, l_spr_y_rounded, l_bmp_x_rounded;
   fixed r_bmp_x_rounded;
   int bmp_y_i;
   int right_edge_test;
   int cl, ct, cr, cb;

   top_index = 0;
   if (ys[1] < ys[0])
      top_index = 1;
   if (ys[2] < ys[top_index])
      top_index = 2;
   if (ys[3] < ys[top_index])
      top_index = 3;

   /* orientation test: double products of int32 differences, exact in x87
    * extended precision -> exact 64-bit integer comparison */
   {
      int64_t a = (int64_t)wrap_sub(xs[(top_index + 1) & 3], xs[top_index]) *
                  wrap_sub(ys[(top_index - 1) & 3], ys[top_index]);
      int64_t b = (int64_t)wrap_sub(xs[(top_index - 1) & 3], xs[top_index]) *
                  wrap_sub(ys[(top_index + 1) & 3], ys[top_index]);
      right_index = a > b ? 1 : -1;
   }

   index = top_index;
   for (i = 0; i < 4; i++) {
      corner_bmp_x[i] = xs[index];
      corner_bmp_y[i] = ys[index];
      if (index < 2)
         corner_spr_y[i] = 0;
      else
         corner_spr_y[i] = shl16(spr->h) - 1;
      if ((index == 0) || (index == 3))
         corner_spr_x[i] = 0;
      else
         corner_spr_x[i] = shl16(spr->w) - 1;
      index = (index + right_index) & 3;
   }

#define top_bmp_y    corner_bmp_y[0]
#define right_bmp_y  corner_bmp_y[1]
#define bottom_bmp_y corner_bmp_y[2]
#define left_bmp_y   corner_bmp_y[3]
#define top_bmp_x    corner_bmp_x[0]
#define right_bmp_x  corner_bmp_x[1]
#define bottom_bmp_x corner_bmp_x[2]
#define left_bmp_x   corner_bmp_x[3]
#define top_spr_y    corner_spr_y[0]
#define right_spr_y  corner_spr_y[1]
#define bottom_spr_y corner_spr_y[2]
#define left_spr_y   corner_spr_y[3]
#define top_spr_x    corner_spr_x[0]
#define right_spr_x  corner_spr_x[1]
#define bottom_spr_x corner_spr_x[2]
#define left_spr_x   corner_spr_x[3]

   clip_rect_of(bmp, &cl, &ct, &cr, &cb);
   clip_left = shl16(cl);
   clip_right = shl16(cr) - 1;

   if ((left_bmp_x > clip_right) && (top_bmp_x > clip_right) && (bottom_bmp_x > clip_right))
      return;
   if ((right_bmp_x < clip_left) && (top_bmp_x < clip_left) && (bottom_bmp_x < clip_left))
      return;

   clip_bottom_i = wrap_add(bottom_bmp_y, 0x8000) >> 16;
   if (clip_bottom_i > cb)
      clip_bottom_i = cb;

   bmp_y_i = wrap_add(top_bmp_y, 0x8000) >> 16;
   if (bmp_y_i < ct)
      bmp_y_i = ct;

   if (bmp_y_i >= clip_bottom_i)
      return;

   extra_scanline_fraction = wrap_sub(wrap_add(shl16(bmp_y_i), 0x8000), top_bmp_y);
   l_bmp_dx = fixdiv(wrap_sub(left_bmp_x, top_bmp_x), wrap_sub(left_bmp_y, top_bmp_y));
   l_bmp_x = wrap_add(top_bmp_x, fixmul(extra_scanline_fraction, l_bmp_dx));
   l_spr_dx = fixdiv(wrap_sub(left_spr_x, top_spr_x), wrap_sub(left_bmp_y, top_bmp_y));
   l_spr_x = wrap_add(top_spr_x, fixmul(extra_scanline_fraction, l_spr_dx));
   l_spr_dy = fixdiv(wrap_sub(left_spr_y, top_spr_y), wrap_sub(left_bmp_y, top_bmp_y));
   l_spr_y = wrap_add(top_spr_y, fixmul(extra_scanline_fraction, l_spr_dy));

   l_bmp_y_bottom_i = wrap_add(left_bmp_y, 0x8000) >> 16;
   if (l_bmp_y_bottom_i > clip_bottom_i)
      l_bmp_y_bottom_i = clip_bottom_i;

   r_bmp_dx = fixdiv(wrap_sub(right_bmp_x, top_bmp_x), wrap_sub(right_bmp_y, top_bmp_y));
   r_bmp_x = wrap_add(top_bmp_x, fixmul(extra_scanline_fraction, r_bmp_dx));

   r_bmp_y_bottom_i = wrap_add(right_bmp_y, 0x8000) >> 16;

   spr_dx = x87_quot_trunc(wrap_sub(ys[3], ys[0]), spr->w,
                           wrap_sub(xs[1], xs[0]), wrap_sub(ys[3], ys[0]),
                           wrap_sub(xs[3], xs[0]), wrap_sub(ys[1], ys[0]));
   spr_dy = x87_quot_trunc(wrap_sub(ys[1], ys[0]), spr->h,
                           wrap_sub(xs[3], xs[0]), wrap_sub(ys[1], ys[0]),
                           wrap_sub(xs[1], xs[0]), wrap_sub(ys[3], ys[0]));

   while (1) {
      if (bmp_y_i >= l_bmp_y_bottom_i) {
         if (bmp_y_i >= clip_bottom_i)
            break;
         extra_scanline_fraction = wrap_sub(wrap_add(shl16(bmp_y_i), 0x8000), left_bmp_y);
         l_bmp_dx = fixdiv(wrap_sub(bottom_bmp_x, left_bmp_x), wrap_sub(bottom_bmp_y, left_bmp_y));
         l_bmp_x = wrap_add(left_bmp_x, fixmul(extra_scanline_fraction, l_bmp_dx));
         l_spr_dx = fixdiv(wrap_sub(bottom_spr_x, left_spr_x), wrap_sub(bottom_bmp_y, left_bmp_y));
         l_spr_x = wrap_add(left_spr_x, fixmul(extra_scanline_fraction, l_spr_dx));
         l_spr_dy = fixdiv(wrap_sub(bottom_spr_y, left_spr_y), wrap_sub(bottom_bmp_y, left_bmp_y));
         l_spr_y = wrap_add(left_spr_y, fixmul(extra_scanline_fraction, l_spr_dy));
         l_bmp_y_bottom_i = wrap_add(bottom_bmp_y, 0x8000) >> 16;
         if (l_bmp_y_bottom_i > clip_bottom_i)
            l_bmp_y_bottom_i = clip_bottom_i;
      }

      if (bmp_y_i >= r_bmp_y_bottom_i) {
         extra_scanline_fraction = wrap_sub(wrap_add(shl16(bmp_y_i), 0x8000), right_bmp_y);
         r_bmp_dx = fixdiv(wrap_sub(bottom_bmp_x, right_bmp_x), wrap_sub(bottom_bmp_y, right_bmp_y));
         r_bmp_x = wrap_add(right_bmp_x, fixmul(extra_scanline_fraction, r_bmp_dx));
         r_bmp_y_bottom_i = clip_bottom_i;
      }

      l_bmp_x_rounded = wrap_add(l_bmp_x, 0x8000) & ~0xffff;
      if (l_bmp_x_rounded < clip_left)
         l_bmp_x_rounded = clip_left;

      l_spr_x_rounded = wrap_add(l_spr_x,
                                 fixmul(wrap_sub(wrap_add(l_bmp_x_rounded, 0x7fff), l_bmp_x), spr_dx));
      l_spr_y_rounded = wrap_add(l_spr_y,
                                 fixmul(wrap_sub(wrap_add(l_bmp_x_rounded, 0x7fff), l_bmp_x), spr_dy));

      r_bmp_x_rounded = wrap_sub(r_bmp_x, 0x8000) & ~0xffff;
      if (r_bmp_x_rounded > clip_right)
         r_bmp_x_rounded = clip_right;

      if (l_bmp_x_rounded <= r_bmp_x_rounded) {
         if ((unsigned)(l_spr_x_rounded >> 16) >= (unsigned)spr->w) {
            if (((l_spr_x_rounded < 0) && (spr_dx <= 0)) ||
                ((l_spr_x_rounded > 0) && (spr_dx >= 0))) {
               goto skip_draw;
            }
            else {
               do {
                  l_spr_x_rounded = wrap_add(l_spr_x_rounded, spr_dx);
                  l_bmp_x_rounded = wrap_add(l_bmp_x_rounded, 65536);
                  if (l_bmp_x_rounded > r_bmp_x_rounded)
                     goto skip_draw;
               } while ((unsigned)(l_spr_x_rounded >> 16) >= (unsigned)spr->w);
            }
         }
         right_edge_test = (int32_t)((uint32_t)l_spr_x_rounded +
                                     (uint32_t)(wrap_sub(r_bmp_x_rounded, l_bmp_x_rounded) >> 16) *
                                     (uint32_t)spr_dx);
         if ((unsigned)(right_edge_test >> 16) >= (unsigned)spr->w) {
            if (((right_edge_test < 0) && (spr_dx <= 0)) ||
                ((right_edge_test > 0) && (spr_dx >= 0))) {
               do {
                  r_bmp_x_rounded = wrap_sub(r_bmp_x_rounded, 65536);
                  right_edge_test = wrap_sub(right_edge_test, spr_dx);
                  if (l_bmp_x_rounded > r_bmp_x_rounded)
                     goto skip_draw;
               } while ((unsigned)(right_edge_test >> 16) >= (unsigned)spr->w);
            }
            else {
               goto skip_draw;
            }
         }
         if ((unsigned)(l_spr_y_rounded >> 16) >= (unsigned)spr->h) {
            if (((l_spr_y_rounded < 0) && (spr_dy <= 0)) ||
                ((l_spr_y_rounded > 0) && (spr_dy >= 0))) {
               goto skip_draw;
            }
            else {
               do {
                  l_spr_y_rounded = wrap_add(l_spr_y_rounded, spr_dy);
                  l_bmp_x_rounded = wrap_add(l_bmp_x_rounded, 65536);
                  if (l_bmp_x_rounded > r_bmp_x_rounded)
                     goto skip_draw;
               } while (((unsigned)l_spr_y_rounded >> 16) >= (unsigned)spr->h);
            }
         }
         right_edge_test = (int32_t)((uint32_t)l_spr_y_rounded +
                                     (uint32_t)(wrap_sub(r_bmp_x_rounded, l_bmp_x_rounded) >> 16) *
                                     (uint32_t)spr_dy);
         if ((unsigned)(right_edge_test >> 16) >= (unsigned)spr->h) {
            if (((right_edge_test < 0) && (spr_dy <= 0)) ||
                ((right_edge_test > 0) && (spr_dy >= 0))) {
               do {
                  r_bmp_x_rounded = wrap_sub(r_bmp_x_rounded, 65536);
                  right_edge_test = wrap_sub(right_edge_test, spr_dy);
                  if (l_bmp_x_rounded > r_bmp_x_rounded)
                     goto skip_draw;
               } while ((unsigned)(right_edge_test >> 16) >= (unsigned)spr->h);
            }
            else {
               goto skip_draw;
            }
         }
         if (bmp_y_i >= 0 && bmp_y_i < bmp->h)
            draw_scanline(bmp, spr, l_bmp_x_rounded, bmp_y_i, r_bmp_x_rounded,
                          l_spr_x_rounded, l_spr_y_rounded, spr_dx, spr_dy);
      }
   skip_draw:
      bmp_y_i++;
      l_bmp_x = wrap_add(l_bmp_x, l_bmp_dx);
      l_spr_x = wrap_add(l_spr_x, l_spr_dx);
      l_spr_y = wrap_add(l_spr_y, l_spr_dy);
      r_bmp_x = wrap_add(r_bmp_x, r_bmp_dx);
   }

#undef top_bmp_y
#undef right_bmp_y
#undef bottom_bmp_y
#undef left_bmp_y
#undef top_bmp_x
#undef right_bmp_x
#undef bottom_bmp_x
#undef left_bmp_x
#undef top_spr_y
#undef right_spr_y
#undef bottom_spr_y
#undef left_spr_y
#undef top_spr_x
#undef right_spr_x
#undef bottom_spr_x
#undef left_spr_x
}

/* rotate.c _rotate_scale_flip_coordinates (no h_flip; v_flip unused) */
static void rotate_scale_flip_coordinates(fixed w, fixed h, fixed x, fixed y,
                                          fixed cx, fixed cy, fixed angle,
                                          fixed scale_x, fixed scale_y,
                                          int h_flip, int v_flip,
                                          fixed xs[4], fixed ys[4])
{
   fixed fix_cos, fix_sin;
   int tl, tr, bl, br, tmp;
   double cos_angle, sin_angle, a;
   fixed xofs, yofs;

   angle = angle & 0xffffff;
   if (angle >= 0x800000)
      angle -= 0x1000000;

   a = angle * (3.14159265358979323846 / (double)0x800000);
   sin_angle = sin(a);
   cos_angle = cos(a);

   if (cos_angle >= 0)
      fix_cos = (int)(cos_angle * 0x10000 + 0.5);
   else
      fix_cos = (int)(cos_angle * 0x10000 - 0.5);
   if (sin_angle >= 0)
      fix_sin = (int)(sin_angle * 0x10000 + 0.5);
   else
      fix_sin = (int)(sin_angle * 0x10000 - 0.5);

   if (v_flip) {
      tl = 3; tr = 2; bl = 0; br = 1;
   }
   else {
      tl = 0; tr = 1; bl = 3; br = 2;
   }
   if (h_flip) {
      tmp = tl; tl = tr; tr = tmp;
      tmp = bl; bl = br; br = tmp;
   }

   w = fixmul(w, scale_x);
   h = fixmul(h, scale_y);
   cx = fixmul(cx, scale_x);
   cy = fixmul(cy, scale_y);

   xofs = wrap_add(wrap_sub(x, fixmul(cx, fix_cos)), fixmul(cy, fix_sin));
   yofs = wrap_sub(wrap_sub(y, fixmul(cx, fix_sin)), fixmul(cy, fix_cos));

   xs[tl] = xofs;
   ys[tl] = yofs;
   xs[tr] = wrap_add(xofs, fixmul(w, fix_cos));
   ys[tr] = wrap_add(yofs, fixmul(w, fix_sin));
   xs[bl] = wrap_sub(xofs, fixmul(h, fix_sin));
   ys[bl] = wrap_add(yofs, fixmul(h, fix_cos));

   xs[br] = wrap_sub(wrap_add(xs[tr], xs[bl]), xs[tl]);
   ys[br] = wrap_sub(wrap_add(ys[tr], ys[bl]), ys[tl]);
}

/* rotate.c _pivot_scaled_sprite_flip + _parallelogram_map_standard */
static void pivot_scaled_sprite_flip(BITMAP *bmp, BITMAP *sprite, fixed x, fixed y,
                                     fixed cx, fixed cy, fixed angle, fixed scale, int v_flip)
{
   fixed xs[4], ys[4];

   rotate_scale_flip_coordinates(shl16(sprite->w), shl16(sprite->h), x, y, cx, cy,
                                 angle, scale, scale, 0, v_flip, xs, ys);
   if (bmp->depth != sprite->depth)
      parallelogram_map(bmp, sprite, xs, ys, draw_scanline_convert);
   else
      parallelogram_map(bmp, sprite, xs, ys, draw_scanline_same);
   a4_touch(bmp);
}

/* draw.inl rotate_sprite */
void rotate_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, fixed angle)
{
   pivot_scaled_sprite_flip(bmp, sprite,
                            wrap_add(shl16(x), (sprite->w * 0x10000) / 2),
                            wrap_add(shl16(y), (sprite->h * 0x10000) / 2),
                            sprite->w << 15, sprite->h << 15,
                            angle, 0x10000, 0);
}

/* draw.inl rotate_scaled_sprite */
void rotate_scaled_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, fixed angle, fixed scale)
{
   pivot_scaled_sprite_flip(bmp, sprite,
                            wrap_add(shl16(x), (fixed)((int32_t)((uint32_t)sprite->w * (uint32_t)scale)) / 2),
                            wrap_add(shl16(y), (fixed)((int32_t)((uint32_t)sprite->h * (uint32_t)scale)) / 2),
                            sprite->w << 15, sprite->h << 15,
                            angle, scale, 0);
}

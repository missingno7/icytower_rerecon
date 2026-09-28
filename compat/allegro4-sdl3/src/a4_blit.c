/*
 * blit (with colour-depth conversion), masked_blit, stretch_blit and
 * stretch_sprite.  Semantics follow Allegro 4.4.1 (giftware licence):
 * blit.c (BLIT_CLIP, blit_to_self, blit_from_256, CONVERT_BLIT,
 * get_replacement_mask_color), c/cblit.h (_linear_blit, _linear_masked_blit)
 * and c/cstretch.c (_al_stretch_blit DDA).
 *
 * Deviations (documented, unused by the game):
 *  - COLORCONV_DITHER_PAL / COLORCONV_DITHER_HI conversions are done without
 *    dithering (plain makecol, like the non-dithered Allegro path).
 *  - masked_blit / stretch_blit / stretch_sprite between different depths
 *    are undefined in Allegro (it asserts, then reads the source with the
 *    destination pixel size); here they convert source pixels.
 *  - with clipping switched off, blits are still limited to the bitmap;
 *    stretch sources are read only inside the source bitmap.
 */
#include <stdint.h>
#include <string.h>
#include "a4_internal.h"
#include "a4_dl.h"

/* blit.c get_replacement_mask_color() */
static unsigned long replacement_mask_color(BITMAP *bmp)
{
   int depth = bmp->depth, g = 0;
   unsigned long c;
   if (depth == 8)
      return (unsigned long)makecol8(255, 4, 255);   /* bestfit_color(pal, 63, 1, 63) */
   do
      c = (unsigned long)makecol_depth(depth, 255, ++g, 255);
   while (c == a4_mask_color(depth));
   return c;
}

/* blit.c BLIT_CLIP (dest clip rectangle; whole bitmap if clipping is off) */
static int blit_clip(BITMAP *src, BITMAP *dest, int *s_x, int *s_y, int *d_x, int *d_y,
                     int *w, int *h)
{
   int cl, ct, cr, cb;
   if (dest->clip) {
      cl = dest->cl; ct = dest->ct; cr = dest->cr; cb = dest->cb;
   }
   else {
      cl = 0; ct = 0; cr = dest->w; cb = dest->h;
   }
   if ((*s_x >= src->w) || (*s_y >= src->h) || (*d_x >= cr) || (*d_y >= cb))
      return 0;
   if (*s_x < 0) {
      *w += *s_x;
      *d_x -= *s_x;
      *s_x = 0;
   }
   if (*s_y < 0) {
      *h += *s_y;
      *d_y -= *s_y;
      *s_y = 0;
   }
   if (*s_x + *w > src->w)
      *w = src->w - *s_x;
   if (*s_y + *h > src->h)
      *h = src->h - *s_y;
   if (*d_x < cl) {
      *d_x -= cl;
      *w += *d_x;
      *s_x -= *d_x;
      *d_x = cl;
   }
   if (*d_y < ct) {
      *d_y -= ct;
      *h += *d_y;
      *s_y -= *d_y;
      *d_y = ct;
   }
   if (*d_x + *w > cr)
      *w = cr - *d_x;
   if (*d_y + *h > cb)
      *h = cb - *d_y;
   return (*w > 0) && (*h > 0);
}

/* blit.c blit_from_256 / CONVERT_BLIT_EX (already clipped) */
static void blit_between_formats(BITMAP *src, BITMAP *dest, int s_x, int s_y,
                                 int d_x, int d_y, int w, int h)
{
   int sd = src->depth, dd = dest->depth, x, y;
   int keep = (a4_color_conversion & COLORCONV_KEEP_TRANS) != 0;

   if (sd == 8) {
      unsigned long table[256];
      int c;
      for (c = 0; c < 256; c++)
         table[c] = (unsigned long)(uint32_t)a4_palette_color_depth(dd, c);
      if (keep) {
         unsigned long rc = replacement_mask_color(dest);
         unsigned long dmask = a4_mask_color(dd);
         table[MASK_COLOR_8] = dmask;
         for (c = 0; c < 256; c++)
            if (c != MASK_COLOR_8 && table[c] == dmask)
               table[c] = rc;
      }
      for (y = 0; y < h; y++)
         for (x = 0; x < w; x++)
            a4_put_raw(dest, d_x + x, d_y + y, table[a4_get_raw(src, s_x + x, s_y + y) & 0xFF]);
      return;
   }

   {
      unsigned long smask = a4_mask_color(sd), dmask = a4_mask_color(dd);
      unsigned long rc = keep ? replacement_mask_color(dest) : 0;
      for (y = 0; y < h; y++) {
         for (x = 0; x < w; x++) {
            unsigned long c = a4_get_raw(src, s_x + x, s_y + y);
            if (keep && c == smask) {
               c = dmask;
            }
            else {
               int r = getr_depth(sd, (int)c), g = getg_depth(sd, (int)c), b = getb_depth(sd, (int)c);
               c = (unsigned long)(uint32_t)makecol_depth(dd, r, g, b);
               if (keep && c == dmask)
                  c = rc;
            }
            a4_put_raw(dest, d_x + x, d_y + y, c);
         }
      }
   }
}

/* blit.c blit(); same-depth copies are plain (overlap-safe) row moves, which
 * is what _linear_blit / _linear_blit_backward with memmove produce */
void blit(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h)
{
   int y, bpp;

   a4_dl_blit(src, dest, s_x, s_y, d_x, d_y, w, h, DLB_SOLID);

   if (!blit_clip(src, dest, &s_x, &s_y, &d_x, &d_y, &w, &h))
      return;

   if (src->depth != dest->depth) {
      blit_between_formats(src, dest, s_x, s_y, d_x, d_y, w, h);
      a4_touch(dest);
      return;
   }

   bpp = a4_bpp_bytes(dest->depth);
   if (a4_root(src) == a4_root(dest)) {
      /* blit_to_self: rows bottom-up when the destination lies below */
      int sy = s_y + src->y_ofs, dy = d_y + dest->y_ofs;
      int sx = s_x + src->x_ofs, dx = d_x + dest->x_ofs;
      if (sx == dx && sy == dy)
         return;
      if (dy > sy) {
         for (y = h - 1; y >= 0; y--)
            memmove(dest->line[d_y + y] + (size_t)d_x * bpp,
                    src->line[s_y + y] + (size_t)s_x * bpp, (size_t)w * bpp);
      }
      else {
         for (y = 0; y < h; y++)
            memmove(dest->line[d_y + y] + (size_t)d_x * bpp,
                    src->line[s_y + y] + (size_t)s_x * bpp, (size_t)w * bpp);
      }
   }
   else {
      for (y = 0; y < h; y++)
         memcpy(dest->line[d_y + y] + (size_t)d_x * bpp,
                src->line[s_y + y] + (size_t)s_x * bpp, (size_t)w * bpp);
   }
   a4_touch(dest);
}

/* blit.c masked_blit() -> cblit.h _linear_masked_blit (mask of the dest
 * depth, compared against the raw source pixel) */
void masked_blit(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h)
{
   int x, y;
   unsigned long mask;

   a4_dl_blit(src, dest, s_x, s_y, d_x, d_y, w, h, DLB_MASKED);

   if (!blit_clip(src, dest, &s_x, &s_y, &d_x, &d_y, &w, &h))
      return;
   mask = a4_mask_color(dest->depth);
   for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++) {
         unsigned long c = a4_get_raw(src, s_x + x, s_y + y);
         if (src->depth != dest->depth) {
            if (c == a4_mask_color(src->depth))
               continue;
            c = a4_convert_color(c, src->depth, dest->depth);
         }
         if (c != mask)
            a4_put_raw(dest, d_x + x, d_y + y, c);
      }
   }
   a4_touch(dest);
}

/* cstretch.c _al_stretch_blit(): Bresenham-style DDA, identical pixel
 * selection (expressed in pixels instead of byte offsets) */
static void stretch_blit_ex(BITMAP *src, BITMAP *dst, int sx, int sy, int sw, int sh,
                            int dx, int dy, int dw, int dh, int masked)
{
   int y, yc, syinc, ycdec, ycinc;
   int sxinc, xcdec, xcinc, xcstart, sxpos;
   int dxbeg, dxend, dybeg, dyend, i;
   int cl, ct, cr, cb;
   unsigned long mask = a4_mask_color(dst->depth);
   int convert = src->depth != dst->depth;
   int drawn = 0;

   if ((sw <= 0) || (sh <= 0) || (dw <= 0) || (dh <= 0))
      return;

   if (dst->clip) {
      cl = dst->cl; ct = dst->ct; cr = dst->cr; cb = dst->cb;
   }
   else {
      cl = 0; ct = 0; cr = dst->w; cb = dst->h;
   }
   dybeg = ((dy > ct) ? dy : ct);
   dyend = (((dy + dh) < cb) ? (dy + dh) : cb);
   if (dybeg >= dyend)
      return;
   dxbeg = ((dx > cl) ? dx : cl);
   dxend = (((dx + dw) < cr) ? (dx + dw) : cr);
   if (dxbeg >= dxend)
      return;

   syinc = sh / dh;
   ycdec = sh - (syinc * dh);
   ycinc = dh - ycdec;
   yc = ycinc;

   sxinc = sw / dw;
   xcdec = sw - ((sw / dw) * dw);
   xcinc = dw - xcdec;

   /* get start state (clip) */
   xcstart = xcinc;
   sxpos = sx;
   for (i = 0; i < dxbeg - dx; i++, sxpos += sxinc) {
      if (xcstart <= 0) {
         xcstart += xcinc;
         sxpos++;
      }
      else
         xcstart -= xcdec;
   }

   /* skip clipped lines */
   for (y = dy; y < dybeg; y++, sy += syinc) {
      if (yc <= 0) {
         sy++;
         yc += ycinc;
      }
      else
         yc -= ycdec;
   }

   for (; y < dyend; y++, sy += syinc) {
      if (sy >= 0 && sy < src->h) {
         int xc = xcstart, sp = sxpos, x;
         for (x = dxbeg; x < dxend; x++, sp += sxinc) {
            if (sp >= 0 && sp < src->w) {
               unsigned long c = a4_get_raw(src, sp, sy);
               if (convert) {
                  if (masked && c == a4_mask_color(src->depth))
                     c = mask;
                  else
                     c = a4_convert_color(c, src->depth, dst->depth);
               }
               if (!masked || c != mask) {
                  a4_put_raw(dst, x, y, c);
                  drawn = 1;
               }
            }
            if (xc <= 0) {
               sp++;
               xc += xcinc;
            }
            else
               xc -= xcdec;
         }
      }
      if (yc <= 0) {
         sy++;
         yc += ycinc;
      }
      else
         yc -= ycdec;
   }
   if (drawn)
      a4_touch(dst);
}

void stretch_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y,
                  int source_width, int source_height, int dest_x, int dest_y,
                  int dest_width, int dest_height)
{
   a4_dl_stretch(source, dest, source_x, source_y, source_width, source_height,
                 dest_x, dest_y, dest_width, dest_height, 0);
   stretch_blit_ex(source, dest, source_x, source_y, source_width, source_height,
                   dest_x, dest_y, dest_width, dest_height, 0);
}

void stretch_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, int w, int h)
{
   a4_dl_stretch(sprite, bmp, 0, 0, sprite->w, sprite->h, x, y, w, h, 1);
   stretch_blit_ex(sprite, bmp, 0, 0, sprite->w, sprite->h, x, y, w, h, 1);
}

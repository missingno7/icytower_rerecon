/*
 * Golden scenarios for the software graphics layer: fixed-point maths,
 * blenders, primitives (solid/XOR/trans), blits (incl. overlap and depth
 * conversion), sprites (flips, 8-bit through the palette, translucency,
 * RGBA), stretching and rotation.
 *
 * Compiled both against real Allegro 4.4.1 (GCC 4.4, C89) and against the
 * compat layer, so: C89 declarations, no floating-point expressions whose
 * rounding could differ between x87 and SSE (doubles are built from bit
 * patterns), bitmaps are always cleared/filled before drawing.
 */
#include <string.h>
#include <errno.h>
#include "golden.h"

static char nm[96];

static const char *N1(const char *a, int i)
{
   sprintf(nm, "gfx.%s.%d", a, i);
   return nm;
}

static const char *N0(const char *a)
{
   sprintf(nm, "gfx.%s", a);
   return nm;
}

/* ---------------------------------------------------------------- helpers */

static unsigned int hash3(int x, int y, int s)
{
   unsigned int h = (unsigned int)x * 0x9E3779B1u ^ (unsigned int)y * 0x85EBCA77u ^
                    (unsigned int)s * 0xC2B2AE3Du;
   h ^= h >> 15;
   h *= 0x2C1B3C6Du;
   h ^= h >> 12;
   h *= 0x297A2D39u;
   h ^= h >> 15;
   return h;
}

static unsigned int mask_of(int depth)
{
   switch (depth) {
      case 8: return 0;
      case 15: return 0x7C1F;
      case 16: return 0xF81F;
      default: return 0xFF00FF;
   }
}

static void rawput(BITMAP *b, int x, int y, unsigned int c)
{
   unsigned char *p = b->line[y];
   switch (bitmap_color_depth(b)) {
      case 8: p[x] = (unsigned char)c; break;
      case 15: case 16: ((unsigned short *)p)[x] = (unsigned short)c; break;
      case 24: p += x * 3; p[0] = c & 0xFF; p[1] = (c >> 8) & 0xFF; p[2] = (c >> 16) & 0xFF; break;
      default: ((unsigned int *)p)[x] = c; break;
   }
}

/* Deterministic pattern bitmap.  flags: 1 = sprinkle mask pixels,
 * 2 = diamond shape (mask outside), 4 = keep alpha byte (32 bpp),
 * 8 = smooth gradient instead of noise */
static BITMAP *pattern(int depth, int w, int h, int seed, int flags)
{
   BITMAP *b = create_bitmap_ex(depth, w, h);
   int x, y;
   clear_to_color(b, 0);
   for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++) {
         unsigned int c = hash3(x, y, seed);
         if (flags & 8)
            c = ((unsigned int)(x * 255 / (w > 1 ? w - 1 : 1)) << 16) |
                ((unsigned int)(y * 255 / (h > 1 ? h - 1 : 1)) << 8) |
                ((unsigned int)((x + y) * 7) & 0xFF) | (c & 0xFF000000u);
         switch (depth) {
            case 8: c &= 0xFF; if (c == 0) c = 1; break;
            case 15: c &= 0x7FFF; break;
            case 16: c &= 0xFFFF; break;
            case 24: c &= 0xFFFFFF; break;
            default: if (!(flags & 4)) c &= 0xFFFFFF; break;
         }
         if ((flags & 1) && (hash3(y, x, seed + 99) % 7) == 0)
            c = mask_of(depth);
         if (flags & 2) {
            int ax = 2 * x + 1 - w, ay = 2 * y + 1 - h;
            if (ax < 0) ax = -ax;
            if (ay < 0) ay = -ay;
            if (ax * h + ay * w > w * h)
               c = mask_of(depth);
         }
         rawput(b, x, y, c);
      }
   }
   return b;
}

static BITMAP *canvas(int depth, int w, int h, int seed)
{
   BITMAP *b = create_bitmap_ex(depth, w, h);
   int x, y;
   clear_to_color(b, 0);
   for (y = 0; y < h; y++)
      for (x = 0; x < w; x++) {
         unsigned int c = hash3(x, y, seed);
         if (depth == 32) c &= 0xFFFFFF;
         if (depth == 24) c &= 0xFFFFFF;
         if (depth == 16) c &= 0xFFFF;
         if (depth == 15) c &= 0x7FFF;
         if (depth == 8) c &= 0xFF;
         rawput(b, x, y, c);
      }
   return b;
}

static void put_le32(unsigned char *p, unsigned int v)
{
   p[0] = v & 0xFF; p[1] = (v >> 8) & 0xFF; p[2] = (v >> 16) & 0xFF; p[3] = (v >> 24) & 0xFF;
}

static double dbl_bits(unsigned int hi, unsigned int lo)
{
   unsigned char b[8];
   double d;
   put_le32(b, lo);
   put_le32(b + 4, hi);
   memcpy(&d, b, 8);
   return d;
}

static PALETTE test_pal;

static void make_palette(void)
{
   int i;
   for (i = 0; i < 256; i++) {
      test_pal[i].r = (unsigned char)((i * 7 + 3) & 63);
      test_pal[i].g = (unsigned char)((i * 13 + 5) & 63);
      test_pal[i].b = (unsigned char)((i * 29 + 11) & 63);
      test_pal[i].filler = 0;
   }
   /* entries that expand to the truecolor mask colour (255, 0, 255) */
   test_pal[0].r = 63; test_pal[0].g = 0; test_pal[0].b = 63;
   test_pal[77].r = 63; test_pal[77].g = 0; test_pal[77].b = 63;
   test_pal[200].r = 63; test_pal[200].g = 1; test_pal[200].b = 63;
}

/* ---------------------------------------------------------------- fixed */

static const int fvals[] = {
   0, 1, -1, 2, -2, 3, -3, 0x7FFF, 0x8000, 0x8001, -0x7FFF, -0x8000, -0x8001,
   0xFFFF, 0x10000, 0x10001, -0x10000, -0x10001, 0x18000, -0x18000, 0x12345, -0x12345,
   0x16A0A, 0xB505, 0xAAAA, 0x5555, -0x5555, 0x7FFFFFFF, -0x7FFFFFFF, (int)0x80000000,
   0x00FFFFFF, 0x01000000, -0x01000000, 0x40000000, -0x40000000, 0x3FFF8000,
   12345678, -87654321, 0x7FFF0000, -0x7FFF0000, 0x7FFF8000, 0x00FF0080, 0x00008080,
   0x00010080, 0x00640000, -0x00640000, 0x3FFFFFFF, 0x0000FFFF, 0x7F7F7F7F, 0x0001FFFF
};
#define NFVALS ((int)(sizeof(fvals) / sizeof(fvals[0])))

static void scen_fixed(golden_ctx *g)
{
   static unsigned char buf[4 * 300 * 300];
   static int vals[300];
   int n = 0, i, j, k, er;
   unsigned int s = 12345;

   for (i = 0; i < NFVALS; i++)
      vals[n++] = fvals[i];
   for (i = 0; n < 300; i++) {
      s = s * 1103515245u + 12345u;
      k = (int)(s >> ((i % 23) + 1));
      if (s & 0x80000000u) k = -k;
      vals[n++] = k;
   }

   /* fixmul over all pairs */
   errno = 0; er = 0;
   for (i = 0; i < n; i++)
      for (j = 0; j < n; j++) {
         errno = 0;
         put_le32(buf + 4 * (i * n + j), (unsigned int)fixmul(vals[i], vals[j]));
         if (errno == ERANGE) er++;
      }
   golden_bytes(g, N0("fixmul"), buf, 4L * n * n);
   golden_int(g, N0("fixmul.erange"), er);

   /* fixdiv over all pairs (including division by zero) */
   er = 0;
   for (i = 0; i < n; i++)
      for (j = 0; j < n; j++) {
         errno = 0;
         put_le32(buf + 4 * (i * n + j), (unsigned int)fixdiv(vals[i], vals[j]));
         if (errno == ERANGE) er++;
      }
   golden_bytes(g, N0("fixdiv"), buf, 4L * n * n);
   golden_int(g, N0("fixdiv.erange"), er);

   /* dense small-operand products and quotients (rounding ties) */
   k = 0;
   for (i = -40; i <= 40; i++)
      for (j = -40; j <= 40; j++)
         put_le32(buf + 4 * k++, (unsigned int)fixmul(i * 0x2001 + (i & 3), j * 0x0C03 - (j & 1)));
   golden_bytes(g, N0("fixmul.small"), buf, 4L * k);
   k = 0;
   for (i = -40; i <= 40; i++)
      for (j = -40; j <= 40; j++)
         put_le32(buf + 4 * k++, (unsigned int)fixdiv(i * 0x3001 + 7, j == 0 ? 3 : j * 0x1235));
   golden_bytes(g, N0("fixdiv.small"), buf, 4L * k);

   /* fixtof (double bits) */
   k = 0;
   for (i = 0; i < n; i++) {
      double d = fixtof(vals[i]);
      memcpy(buf + 8 * k++, &d, 8);
   }
   golden_bytes(g, N0("fixtof"), buf, 8L * k);

   /* ftofix on doubles built from bit patterns: exact halves and neighbours */
   k = 0;
   er = 0;
   for (i = 0; i < 64; i++) {
      /* (2m+1)/131072 = exact tie; build via m */
      int m = (int)(hash3(i, 0, 5) >> (i % 17));
      double tie, lo, hi;
      unsigned char b[8];
      unsigned int bh, bl;
      tie = ((double)m * 2.0 + 1.0) / 131072.0;   /* exact: < 2^53, power-of-two divisor */
      if (i & 1)
         tie = -tie;
      memcpy(b, &tie, 8);
      bl = b[0] | (b[1] << 8) | (b[2] << 16) | ((unsigned int)b[3] << 24);
      bh = b[4] | (b[5] << 8) | (b[6] << 16) | ((unsigned int)b[7] << 24);
      lo = dbl_bits(bh - (bl == 0 ? 1 : 0), bl - 1);
      hi = dbl_bits(bh + (bl == 0xFFFFFFFFu ? 1 : 0), bl + 1);
      errno = 0;
      put_le32(buf + 4 * k++, (unsigned int)ftofix(tie));
      put_le32(buf + 4 * k++, (unsigned int)ftofix(lo));
      put_le32(buf + 4 * k++, (unsigned int)ftofix(hi));
      if (errno == ERANGE) er++;
   }
   {
      /* special values: +-0, tiny, 0.5/65536 neighbours, range limits, inf */
      static const unsigned int bits[][2] = {
         { 0x00000000, 0x00000000 }, { 0x80000000, 0x00000000 },
         { 0x00000001, 0x00000000 }, { 0x80100000, 0x00000000 },
         { 0x3EE00000, 0x00000000 }, { 0x3EDFFFFF, 0xFFFFFFFF }, { 0x3EE00000, 0x00000001 },
         { 0xBEE00000, 0x00000000 }, { 0xBEDFFFFF, 0xFFFFFFFF }, { 0xBEE00000, 0x00000001 },
         { 0x40DFFFC0, 0x00000000 }, { 0x40DFFFC0, 0x00000001 }, { 0x40DFFFBF, 0xFFFFFFFF },
         { 0xC0DFFFC0, 0x00000000 }, { 0xC0DFFFC0, 0x00000001 }, { 0xC0DFFFBF, 0xFFFFFFFF },
         { 0x40E00000, 0x00000000 }, { 0xC0E00000, 0x00000000 },
         { 0x7FF00000, 0x00000000 }, { 0xFFF00000, 0x00000000 },
         { 0x3FF00000, 0x00000000 }, { 0x3FEFFFFF, 0xFFFFFFFF }, { 0x40590000, 0x00000000 },
         { 0x3FC5D867, 0xC3ECE2A5 }, { 0x3FB99999, 0x9999999A }, { 0xC00921FB, 0x54442D18 }
      };
      for (i = 0; i < (int)(sizeof(bits) / sizeof(bits[0])); i++) {
         errno = 0;
         put_le32(buf + 4 * k++, (unsigned int)ftofix(dbl_bits(bits[i][0], bits[i][1])));
         if (errno == ERANGE) er++;
      }
   }
   /* k * 2^-16 exactly and +/- 2^-17 around it */
   for (i = -300; i <= 300; i += 7) {
      put_le32(buf + 4 * k++, (unsigned int)ftofix((double)i / 65536.0));
      put_le32(buf + 4 * k++, (unsigned int)ftofix((double)(2 * i + 1) / 131072.0));
      put_le32(buf + 4 * k++, (unsigned int)ftofix((double)i * 1.5));
   }
   golden_bytes(g, N0("ftofix"), buf, 4L * k);
   golden_int(g, N0("ftofix.erange"), er);

   /* trig tables and inline helpers */
   golden_bytes(g, N0("cos_tbl"), _cos_tbl, 512 * 4);
   golden_bytes(g, N0("tan_tbl"), _tan_tbl, 256 * 4);
   golden_bytes(g, N0("acos_tbl"), _acos_tbl, 513 * 4);
   k = 0;
   for (i = -70000; i < 70000; i += 997) {
      fixed a = (fixed)i * 97;
      put_le32(buf + 4 * k++, (unsigned int)fixsin(a));
      put_le32(buf + 4 * k++, (unsigned int)fixcos(a));
      put_le32(buf + 4 * k++, (unsigned int)fixtoi(a));
      put_le32(buf + 4 * k++, (unsigned int)fixfloor(a));
      put_le32(buf + 4 * k++, (unsigned int)itofix(i >> 4));
   }
   golden_bytes(g, N0("trig"), buf, 4L * k);
}

/* ---------------------------------------------------------------- blenders */

static const int alphas[] = { 0, 1, 7, 8, 15, 16, 31, 32, 64, 100, 110, 127, 128, 150, 158,
                              200, 240, 247, 248, 254, 255, 256, 300 };
#define NALPHAS ((int)(sizeof(alphas) / sizeof(alphas[0])))

static void scen_blend(golden_ctx *g)
{
   static const int depths[] = { 15, 16, 24, 32 };
   int di, y;

   for (di = 0; di < 4; di++) {
      int d = depths[di];
      BITMAP *b;

      /* trans blender through hline: one alpha and one colour per row */
      set_color_depth(d);
      b = canvas(d, 96, 2 * NALPHAS, 10 + d);
      drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
      for (y = 0; y < 2 * NALPHAS; y++) {
         int col = (int)hash3(y, d, 3);
         if (d == 32 && (y & 1)) col = (int)((unsigned int)col | 0xFF000000u);
         set_trans_blender(y * 3, y * 5, y * 7, alphas[y % NALPHAS]);
         hline(b, 0, y, 95, col);
      }
      solid_mode();
      golden_bitmap(g, N1("blend.trans.hline", d), b);
      destroy_bitmap(b);

      /* alpha blender through putpixel (32 bpp: per-colour alpha;
       * other depths: _blender_black) */
      b = canvas(d, 64, 32, 20 + d);
      set_alpha_blender();
      drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
      for (y = 0; y < 32; y++) {
         int x;
         for (x = 0; x < 64; x += 3)
            putpixel(b, x, y, (int)((hash3(x, y, 4) & 0x00FFFFFFu) |
                                    ((unsigned int)((y * 8 + x) & 0xFF) << 24)));
      }
      solid_mode();
      golden_bitmap(g, N1("blend.alpha.putpixel", d), b);
      destroy_bitmap(b);
   }

   /* game-style fades: rectfill with alpha 0..255 step 16 at 32 bpp */
   {
      BITMAP *b;
      int a, i = 0;
      set_color_depth(32);
      b = canvas(32, 160, 64, 7);
      drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
      for (a = 0; a < 256; a += 16, i++) {
         set_trans_blender(0, 0, 0, a);
         rectfill(b, i * 10, 0, i * 10 + 9, 63, (i & 1) ? 0x000000 : 0xFFFFFF);
      }
      set_trans_blender(0, 0, 0, 110);
      rectfill(b, 5, 5, 150, 20, makecol32(30, 60, 200));
      set_trans_blender(0, 0, 0, 150);
      rect(b, 3, 30, 157, 60, makecol32(255, 128, 0));
      set_trans_blender(0, 0, 0, 158);
      line(b, 0, 63, 159, 0, makecol32(10, 250, 30));
      vline(b, 80, -5, 70, makecol32(200, 200, 200));
      solid_mode();
      golden_bitmap(g, N0("blend.fade32"), b);
      destroy_bitmap(b);
   }
}

/* ---------------------------------------------------------------- primitives */

static void draw_prims(BITMAP *b, int col_base)
{
   int i;
   /* lines in every octant, from inside and from outside the bitmap */
   for (i = 0; i < 48; i++) {
      int ang = i * 360 / 48;
      static const int ox[16] = { 40, 37, 28, 15, 0, -15, -28, -37, -40, -37, -28, -15, 0, 15, 28, 37 };
      static const int oy[16] = { 0, 15, 28, 37, 40, 37, 28, 15, 0, -15, -28, -37, -40, -37, -28, -15 };
      int k = i % 16, s = 1 + i / 16;
      (void)ang;
      line(b, 50, 40, 50 + ox[k] * s, 40 + oy[k] * s / 2, col_base + i * 0x010203);
      line(b, -20 + i, 90 - i, 130 - i * 2, -10 + i * 3, col_base ^ (i * 0x0A0B0C));
   }
   line(b, 3, 3, 3, 3, col_base);
   line(b, 10, 70, 10, 5, col_base + 1);
   line(b, 5, 75, 95, 75, col_base + 2);
   hline(b, -10, 2, 200, col_base + 3);
   hline(b, 60, 4, 20, col_base + 4);
   hline(b, 0, -1, 99, col_base + 5);
   vline(b, 97, -50, 500, col_base + 6);
   vline(b, 95, 60, 20, col_base + 7);
   vline(b, -1, 0, 50, col_base + 8);
   rect(b, 20, 20, 80, 60, col_base + 9);
   rect(b, 90, 70, 60, 50, col_base + 10);
   rect(b, -5, -5, 200, 200, col_base + 11);
   rect(b, 30, 30, 30, 50, col_base + 12);
   rect(b, 30, 65, 45, 65, col_base + 13);
   rectfill(b, 70, 10, 40, 25, col_base + 14);
   rectfill(b, -30, 60, 10, 90, col_base + 15);
   rectfill(b, 55, 55, 55, 55, col_base + 16);
   putpixel(b, 0, 0, col_base + 17);
   putpixel(b, -1, 5, col_base + 18);
   putpixel(b, 99, 79, col_base + 19);
   putpixel(b, 100, 79, col_base + 20);
}

static void scen_prims(golden_ctx *g)
{
   static const int depths[] = { 8, 15, 16, 24, 32 };
   int di, mode;

   for (di = 0; di < 5; di++) {
      int d = depths[di];
      set_color_depth(d);
      for (mode = 0; mode < 3; mode++) {
         BITMAP *b = canvas(d, 100, 80, 30 + d);
         int base = d == 8 ? 0x21 : d <= 16 ? 0x1234 : 0x123456;
         if (mode == 1) {
            drawing_mode(DRAW_MODE_XOR, NULL, 0, 0);
         }
         else if (mode == 2) {
            if (d == 8) {
               destroy_bitmap(b);
               continue;   /* 8 bpp translucency needs a color_map */
            }
            set_trans_blender(0, 0, 0, 100);
            drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
         }
         draw_prims(b, base);
         /* same again with a narrow clip rectangle */
         set_clip_rect(b, 25, 15, 70, 50);
         draw_prims(b, base ^ 0x5A);
         set_clip_rect(b, 0, 0, b->w - 1, b->h - 1);
         solid_mode();
         sprintf(nm, "gfx.prims.d%d.m%d", d, mode);
         golden_bitmap(g, nm, b);
         destroy_bitmap(b);
      }
   }

   /* full-screen stripe patterns like the game's */
   {
      BITMAP *b;
      int x, y;
      set_color_depth(32);
      b = create_bitmap(640, 480);
      clear_to_color(b, makecol32(1, 2, 3));
      for (x = 0; x < 640; x += 3)
         vline(b, x, 0, 479, makecol32(x & 255, 40, 80));
      for (y = 0; y < 480; y += 4)
         hline(b, 0, y, 639, makecol32(10, y & 255, 200));
      set_clip_rect(b, 100, 50, 539, 429);
      set_trans_blender(0, 0, 0, 158);
      drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
      rectfill(b, -5, -5, 700, 500, makecol32(0, 0, 0));
      solid_mode();
      set_clip_rect(b, 0, 0, 639, 479);
      golden_bitmap(g, N0("prims.stripes"), b);

      /* getpixel incl. out-of-range and sub-bitmaps */
      golden_int(g, N0("getpixel.0"), getpixel(b, 0, 0));
      golden_int(g, N0("getpixel.1"), getpixel(b, 639, 479));
      golden_int(g, N0("getpixel.2"), getpixel(b, 640, 0));
      golden_int(g, N0("getpixel.3"), getpixel(b, -1, 3));
      golden_int(g, N0("getpixel.4"), getpixel(b, 200, 200));
      {
         BITMAP *s = create_sub_bitmap(b, 100, 100, 50, 40);
         golden_int(g, N0("getpixel.sub0"), getpixel(s, 0, 0));
         golden_int(g, N0("getpixel.sub1"), getpixel(s, 49, 39));
         golden_int(g, N0("getpixel.sub2"), getpixel(s, 50, 0));
         golden_int(g, N0("getpixel.sub3"), getpixel(s, -1, 0));
         /* drawing into a sub-bitmap clips to it */
         rectfill(s, -10, -10, 100, 5, makecol32(9, 9, 9));
         line(s, -20, -20, 80, 70, makecol32(250, 0, 0));
         rect(s, 2, 2, 60, 30, makecol32(0, 250, 0));
         destroy_bitmap(s);
      }
      golden_bitmap(g, N0("prims.sub"), b);
      destroy_bitmap(b);
   }
   {
      BITMAP *b8;
      set_color_depth(8);
      b8 = canvas(8, 16, 16, 5);
      golden_int(g, N0("getpixel8.0"), getpixel(b8, 3, 4));
      golden_int(g, N0("getpixel8.1"), getpixel(b8, 15, 15));
      golden_int(g, N0("getpixel8.2"), getpixel(b8, 16, 15));
      destroy_bitmap(b8);
   }
}

/* ---------------------------------------------------------------- blits */

static void scen_blit(golden_ctx *g)
{
   BITMAP *src, *dst, *sub;
   int i;

   set_color_depth(32);
   src = pattern(32, 70, 50, 1, 1);

   /* clipped blits at negative / beyond-edge positions */
   dst = canvas(32, 120, 90, 2);
   blit(src, dst, 0, 0, -57, -8, 70, 50);
   blit(src, dst, 10, 5, 100, 70, 70, 50);
   blit(src, dst, -5, -5, 30, 30, 20, 20);
   blit(src, dst, 60, 40, 10, 60, 30, 30);
   blit(src, dst, 0, 0, 200, 10, 70, 50);
   blit(src, dst, 71, 0, 10, 10, 5, 5);
   set_clip_rect(dst, 40, 20, 79, 59);
   blit(src, dst, 0, 0, 20, 10, 70, 50);
   set_clip_rect(dst, 0, 0, 119, 89);
   golden_bitmap(g, N0("blit.clip"), dst);

   /* sub-bitmap target and source */
   sub = create_sub_bitmap(dst, 13, 17, 40, 30);
   blit(src, sub, 3, 3, -4, -6, 70, 50);
   blit(sub, dst, 0, 0, 80, 50, 40, 30);
   destroy_bitmap(sub);
   golden_bitmap(g, N0("blit.sub"), dst);
   destroy_bitmap(dst);

   /* screen shake: blit(b, b, 0, shake, 0, 0, w, h) both directions */
   for (i = 0; i < 4; i++) {
      static const int sh[4] = { 5, -5, 1, -13 };
      BITMAP *b = canvas(32, 640, 480, 40 + i);
      blit(b, b, 0, sh[i], 0, 0, 640, 480);
      golden_bitmap(g, N1("blit.shake", i), b);
      destroy_bitmap(b);
   }
   /* overlap: horizontal both ways, diagonal both ways, sub-bitmaps */
   for (i = 0; i < 6; i++) {
      static const int o[6][4] = { { 0, 0, 7, 0 }, { 7, 0, 0, 0 }, { 0, 0, 5, 9 },
                                   { 5, 9, 0, 0 }, { 9, 2, 3, 6 }, { 3, 6, 9, 2 } };
      BITMAP *b = canvas(32, 60, 50, 50 + i);
      BITMAP *s1, *s2;
      blit(b, b, o[i][0], o[i][1], o[i][2], o[i][3], 45, 38);
      s1 = create_sub_bitmap(b, 4, 4, 40, 30);
      s2 = create_sub_bitmap(b, 8, 2, 40, 30);
      blit(s1, s2, 0, 0, 1, 3, 40, 30);
      blit(s2, s1, 2, 1, 0, 0, 40, 30);
      destroy_bitmap(s1);
      destroy_bitmap(s2);
      golden_bitmap(g, N1("blit.overlap", i), b);
      destroy_bitmap(b);
   }
   /* 16 and 24 bpp same-depth overlap */
   {
      BITMAP *b16 = canvas(16, 40, 30, 3), *b24 = canvas(24, 40, 30, 4);
      blit(b16, b16, 3, 2, 0, 0, 40, 30);
      blit(b24, b24, 0, 0, 3, 2, 40, 30);
      golden_bitmap(g, N0("blit.overlap16"), b16);
      golden_bitmap(g, N0("blit.overlap24"), b24);
      destroy_bitmap(b16);
      destroy_bitmap(b24);
   }

   /* masked_blit */
   dst = canvas(32, 100, 70, 6);
   masked_blit(src, dst, 0, 0, -10, -10, 70, 50);
   masked_blit(src, dst, 20, 10, 60, 40, 70, 50);
   set_clip_rect(dst, 10, 10, 50, 60);
   masked_blit(src, dst, 0, 0, 5, 20, 70, 50);
   set_clip_rect(dst, 0, 0, 99, 69);
   golden_bitmap(g, N0("masked_blit32"), dst);
   destroy_bitmap(dst);
   {
      BITMAP *s16 = pattern(16, 30, 30, 8, 1), *d16 = canvas(16, 40, 40, 9);
      masked_blit(s16, d16, 0, 0, 15, -5, 30, 30);
      golden_bitmap(g, N0("masked_blit16"), d16);
      destroy_bitmap(s16);
      destroy_bitmap(d16);
   }
   destroy_bitmap(src);
}

static void scen_convert(golden_ctx *g)
{
   static const int depths[] = { 8, 15, 16, 24, 32 };
   int si, di, keep;

   make_palette();
   set_color_depth(32);
   set_palette(test_pal);

   for (keep = 0; keep < 2; keep++) {
      set_color_conversion(keep ? (COLORCONV_TOTAL | COLORCONV_KEEP_TRANS) : COLORCONV_TOTAL);
      for (si = 0; si < 5; si++) {
         BITMAP *src = pattern(depths[si], 64, 24, 70 + si, 1);
         for (di = 0; di < 5; di++) {
            BITMAP *dst;
            if (si == di)
               continue;
            dst = canvas(depths[di], 50, 30, 80 + di);
            blit(src, dst, 3, 2, -2, 4, 64, 24);
            sprintf(nm, "gfx.convert.%d.%d.k%d", depths[si], depths[di], keep);
            golden_bitmap(g, nm, dst);
            destroy_bitmap(dst);
         }
         destroy_bitmap(src);
      }
   }
   set_color_conversion(COLORCONV_TOTAL);

   /* 8 bpp source through a palette selected at a different depth */
   {
      BITMAP *src = pattern(8, 32, 16, 90, 0), *dst = canvas(32, 32, 16, 91);
      PALETTE p2;
      int i;
      for (i = 0; i < 256; i++) {
         p2[i].r = (unsigned char)(63 - test_pal[i].r);
         p2[i].g = test_pal[i].b;
         p2[i].b = test_pal[i].g;
         p2[i].filler = 0;
      }
      set_color_depth(16);
      select_palette(p2);
      set_color_depth(32);
      blit(src, dst, 0, 0, 0, 0, 32, 16);
      golden_bitmap(g, N0("convert.select"), dst);
      set_color_depth(16);
      unselect_palette();
      set_color_depth(32);
      blit(src, dst, 0, 0, 5, 5, 32, 16);
      golden_bitmap(g, N0("convert.unselect"), dst);
      destroy_bitmap(src);
      destroy_bitmap(dst);
   }
}

/* ---------------------------------------------------------------- sprites */

static void scen_sprites(golden_ctx *g)
{
   BITMAP *spr, *dst, *sub;
   static const int pos[][2] = { { -57, -8 }, { -8, 20 }, { 30, 25 }, { 85, -12 }, { 90, 60 },
                                 { 20, 75 }, { -40, 70 }, { 200, 5 }, { 5, -200 } };
   int i, f;

   make_palette();
   set_color_depth(32);
   set_palette(test_pal);

   spr = pattern(32, 37, 23, 11, 3);
   for (f = 0; f < 3; f++) {
      dst = canvas(32, 110, 90, 12);
      for (i = 0; i < 9; i++) {
         if (f == 0) draw_sprite(dst, spr, pos[i][0], pos[i][1]);
         if (f == 1) draw_sprite_h_flip(dst, spr, pos[i][0], pos[i][1]);
         if (f == 2) draw_sprite_v_flip(dst, spr, pos[i][0], pos[i][1]);
      }
      set_clip_rect(dst, 33, 17, 70, 44);
      for (i = 0; i < 9; i++) {
         int x = 20 + (i % 3) * 18, y = 5 + (i / 3) * 15;
         if (f == 0) draw_sprite(dst, spr, x, y);
         if (f == 1) draw_sprite_h_flip(dst, spr, x, y);
         if (f == 2) draw_sprite_v_flip(dst, spr, x, y);
      }
      set_clip_rect(dst, 0, 0, 109, 89);
      sub = create_sub_bitmap(dst, 60, 50, 30, 25);
      if (f == 0) draw_sprite(sub, spr, -5, -3);
      if (f == 1) draw_sprite_h_flip(sub, spr, -5, -3);
      if (f == 2) draw_sprite_v_flip(sub, spr, 10, 8);
      destroy_bitmap(sub);
      golden_bitmap(g, N1("sprite32", f), dst);
      destroy_bitmap(dst);
   }
   destroy_bitmap(spr);

   /* 8 bpp sprite onto 32 bpp through the palette (index 0 transparent) */
   {
      BITMAP *s8 = pattern(8, 29, 19, 13, 3);
      dst = canvas(32, 80, 60, 14);
      for (i = 0; i < 9; i++)
         draw_sprite(dst, s8, pos[i][0] / 2 + 10, pos[i][1] / 2 + 5);
      golden_bitmap(g, N0("sprite8on32"), dst);
      destroy_bitmap(dst);
      /* and onto 8 bpp and 16 bpp */
      set_color_depth(8);
      dst = canvas(8, 60, 40, 15);
      draw_sprite(dst, s8, 40, -5);
      draw_sprite_h_flip(dst, s8, -7, 10);
      draw_sprite_v_flip(dst, s8, 20, 25);
      golden_bitmap(g, N0("sprite8on8"), dst);
      destroy_bitmap(dst);
      set_color_depth(16);
      dst = canvas(16, 60, 40, 16);
      draw_sprite(dst, s8, 40, -5);
      golden_bitmap(g, N0("sprite8on16"), dst);
      destroy_bitmap(dst);
      set_color_depth(32);
      destroy_bitmap(s8);
   }
   /* 15/16/24 bpp sprites */
   {
      static const int dd[3] = { 15, 16, 24 };
      for (i = 0; i < 3; i++) {
         BITMAP *s = pattern(dd[i], 21, 17, 17 + i, 3);
         set_color_depth(dd[i]);
         dst = canvas(dd[i], 50, 40, 18 + i);
         draw_sprite(dst, s, -4, 30);
         draw_sprite_h_flip(dst, s, 35, -6);
         draw_sprite_v_flip(dst, s, 15, 12);
         golden_bitmap(g, N1("sprite", dd[i]), dst);
         destroy_bitmap(dst);
         destroy_bitmap(s);
      }
      set_color_depth(32);
   }

   /* draw_trans_sprite: 32 bpp with the trans blender (game alphas) */
   spr = pattern(32, 31, 27, 21, 3);
   dst = canvas(32, 120, 60, 22);
   {
      static const int ta[6] = { 110, 150, 158, 0, 255, 64 };
      for (i = 0; i < 6; i++) {
         set_trans_blender(0, 0, 0, ta[i]);
         draw_trans_sprite(dst, spr, i * 20 - 10, (i & 1) * 40 - 8);
      }
   }
   golden_bitmap(g, N0("trans32"), dst);
   destroy_bitmap(dst);
   destroy_bitmap(spr);

   /* draw_trans_sprite with set_alpha_blender and per-pixel alpha */
   spr = pattern(32, 40, 30, 23, 4 | 8 | 2);
   dst = canvas(32, 100, 70, 24);
   set_alpha_blender();
   draw_trans_sprite(dst, spr, -12, -9);
   draw_trans_sprite(dst, spr, 30, 20);
   draw_trans_sprite(dst, spr, 75, 50);
   golden_bitmap(g, N0("alpha32"), dst);
   destroy_bitmap(dst);
   {
      static const int dd[3] = { 15, 16, 24 };
      int bl;
      for (bl = 0; bl < 2; bl++)
         for (i = 0; i < 3; i++) {
            set_color_depth(dd[i]);
            dst = canvas(dd[i], 60, 40, 25 + i);
            if (bl == 0)
               set_alpha_blender();
            else
               set_trans_blender(0, 0, 0, 128);
            draw_trans_sprite(dst, spr, -5, 12);
            draw_trans_sprite(dst, spr, 30, -10);
            sprintf(nm, "gfx.rgba.d%d.b%d", dd[i], bl);
            golden_bitmap(g, nm, dst);
            destroy_bitmap(dst);
         }
      set_color_depth(32);
   }
   destroy_bitmap(spr);
   /* same-depth trans sprites at 16 bpp and 8 bpp source onto 32 */
   {
      BITMAP *s16 = pattern(16, 25, 20, 27, 1), *d16 = canvas(16, 50, 40, 28);
      BITMAP *s8 = pattern(8, 20, 10, 29, 1), *d32 = canvas(32, 40, 30, 30);
      set_trans_blender(0, 0, 0, 150);
      draw_trans_sprite(d16, s16, 10, 12);
      draw_trans_sprite(d16, s16, -8, -3);
      draw_trans_sprite(d32, s8, 5, 5);
      set_alpha_blender();
      draw_trans_sprite(d32, s8, 15, 18);
      golden_bitmap(g, N0("trans16"), d16);
      golden_bitmap(g, N0("trans8on32"), d32);
      destroy_bitmap(s16);
      destroy_bitmap(d16);
      destroy_bitmap(s8);
      destroy_bitmap(d32);
   }
}

/* ---------------------------------------------------------------- stretch */

static void scen_stretch(golden_ctx *g)
{
   static const int sz[][2] = { { 70, 50 }, { 35, 25 }, { 140, 100 }, { 23, 71 }, { 1, 1 },
                                { 3, 90 }, { 69, 49 }, { 71, 51 }, { 211, 13 } };
   BITMAP *src, *dst;
   int i;

   set_color_depth(32);
   src = pattern(32, 70, 50, 31, 3);
   for (i = 0; i < 9; i++) {
      dst = canvas(32, 120, 90, 32);
      stretch_sprite(dst, src, -13, -7, sz[i][0], sz[i][1]);
      stretch_sprite(dst, src, 60, 40, sz[i][0], sz[i][1]);
      set_clip_rect(dst, 20, 30, 50, 60);
      stretch_sprite(dst, src, 5, 20, sz[i][0], sz[i][1]);
      set_clip_rect(dst, 0, 0, 119, 89);
      golden_bitmap(g, N1("stretch_sprite", i), dst);
      destroy_bitmap(dst);

      dst = canvas(32, 120, 90, 33);
      stretch_blit(src, dst, 5, 3, 60, 44, -9, 4, sz[i][0], sz[i][1]);
      stretch_blit(src, dst, 0, 0, 70, 50, 70, 60, sz[i][0], sz[i][1]);
      golden_bitmap(g, N1("stretch_blit", i), dst);
      destroy_bitmap(dst);
   }
   destroy_bitmap(src);
   {
      static const int dd[4] = { 8, 15, 16, 24 };
      for (i = 0; i < 4; i++) {
         BITMAP *s;
         set_color_depth(dd[i]);
         s = pattern(dd[i], 30, 20, 34 + i, 1);
         dst = canvas(dd[i], 70, 50, 35 + i);
         stretch_sprite(dst, s, -5, 3, 47, 31);
         stretch_blit(s, dst, 2, 2, 25, 15, 40, 30, 17, 11);
         golden_bitmap(g, N1("stretch", dd[i]), dst);
         destroy_bitmap(dst);
         destroy_bitmap(s);
      }
      set_color_depth(32);
   }
}

/* ---------------------------------------------------------------- rotation */

static void rot_grid(golden_ctx *g, const char *name, BITMAP *spr, int depth,
                     const fixed *angles, int nang, const fixed *scales, int nsc, int seed)
{
   int cols = 8, tw = 64, th = 64, n = nang * (nsc ? nsc : 1), i;
   int rows = (n + cols - 1) / cols;
   BITMAP *c = canvas(depth, cols * tw, rows * th, seed);
   for (i = 0; i < n; i++) {
      BITMAP *t = create_sub_bitmap(c, (i % cols) * tw, (i / cols) * th, tw, th);
      fixed a = angles[i % nang];
      int px = (int)(hash3(i, 1, seed) % 70) - 35 + 16;
      int py = (int)(hash3(i, 2, seed) % 70) - 35 + 16;
      if (i % 3 == 0) {
         px = (tw - spr->w) / 2;
         py = (th - spr->h) / 2;
      }
      if (i % 5 == 4)
         set_clip_rect(t, 10, 12, 50, 47);
      if (nsc)
         rotate_scaled_sprite(t, spr, px, py, a, scales[i / nang]);
      else
         rotate_sprite(t, spr, px, py, a);
      destroy_bitmap(t);
   }
   golden_bitmap(g, name, c);
   destroy_bitmap(c);
}

static void scen_rotate(golden_ctx *g)
{
   static fixed angles[64];
   static const int degs[] = { 0, 1, 5, 16, 32, 45, 63, 64, 65, 90, 100, 127, 128, 129, 150,
                               192, 200, 255, 256, 300, -1, -32, -64, -100, -256, -300, 512, 1000 };
   static const fixed extra[] = { 0x12345, 0x7FFFFF, -0x5A5A5, 0xFFFFFF, 0x1000000, 0x800000,
                                  0x400000, 0x3FFFFF, 0x400001, 0x2AAAA, -0x10, 0x10, 0x8000,
                                  0x5E000, 0xAB67, 0x1D4C1, 0x2ADC0, 0x39B7F, 0x7FFFFFFF,
                                  (fixed)0x80000000, -0x4C0000, 0x55555, 0xC8000, 0x28000,
                                  0x1B4D1, 0x61FA8, 0x187, 0x4E2, 0x00D9F, -0x31B, 0x2A56, 0xFA000,
                                  0x3E800, 0x2D5AC, 0xF0000, 0x71C7 };
   static const fixed scales[] = { 0, 0x100, 0x4000, 0x8000, 0xA3D7, 0xC000, 0xFFFF, 0x10000,
                                   0x14000, 0x18000, 0x1C28F };
   int nang = 0, i;
   BITMAP *spr;

   make_palette();
   set_color_depth(32);
   set_palette(test_pal);

   for (i = 0; i < (int)(sizeof(degs) / sizeof(degs[0])); i++)
      angles[nang++] = itofix(degs[i]);
   for (i = 0; i < (int)(sizeof(extra) / sizeof(extra[0])); i++)
      angles[nang++] = extra[i];

   spr = pattern(32, 37, 23, 41, 3);
   rot_grid(g, N0("rotate.37x23"), spr, 32, angles, nang, NULL, 0, 42);
   destroy_bitmap(spr);
   spr = pattern(32, 32, 32, 43, 1);
   rot_grid(g, N0("rotate.32x32"), spr, 32, angles, nang, NULL, 0, 44);
   destroy_bitmap(spr);
   spr = pattern(32, 8, 5, 45, 0);
   rot_grid(g, N0("rotate.8x5"), spr, 32, angles, nang, NULL, 0, 46);
   destroy_bitmap(spr);
   spr = pattern(32, 1, 1, 47, 0);
   rot_grid(g, N0("rotate.1x1"), spr, 32, angles, 16, NULL, 0, 48);
   destroy_bitmap(spr);

   /* game-like angles: fixsin(...) * 5 wobble */
   {
      fixed wob[32];
      for (i = 0; i < 32; i++)
         wob[i] = fixsin(itofix(i * 8)) * 5;
      spr = pattern(32, 40, 20, 49, 2);
      rot_grid(g, N0("rotate.wobble"), spr, 32, wob, 32, NULL, 0, 50);
      destroy_bitmap(spr);
   }

   /* rotate_scaled_sprite over scales x angles */
   spr = pattern(32, 30, 21, 51, 3);
   rot_grid(g, N0("rotscale.a"), spr, 32, angles, 12, scales, 11, 52);
   rot_grid(g, N0("rotscale.b"), spr, 32, angles + 30, 12, scales, 11, 53);
   destroy_bitmap(spr);

   /* other depths: same-depth drawers and the 8->32 generic convert path */
   {
      static const int dd[4] = { 8, 15, 16, 24 };
      for (i = 0; i < 4; i++) {
         set_color_depth(dd[i]);
         spr = pattern(dd[i], 27, 19, 54 + i, 3);
         rot_grid(g, N1("rotate.d", dd[i]), spr, dd[i], angles, 24, NULL, 0, 55 + i);
         destroy_bitmap(spr);
      }
      set_color_depth(32);
      spr = pattern(8, 27, 19, 60, 3);
      rot_grid(g, N0("rotate.8on32"), spr, 32, angles, 24, NULL, 0, 61);
      destroy_bitmap(spr);
      spr = pattern(16, 27, 19, 62, 3);
      rot_grid(g, N0("rotate.16on32"), spr, 32, angles, 16, NULL, 0, 63);
      destroy_bitmap(spr);
   }
}

void scen_gfx(golden_ctx *g)
{
#ifndef A4COMPAT
   /* Real Allegro started with SYSTEM_NONE keeps its generic default pixel
    * layouts (red at bit 0).  The game ran under the Windows system driver,
    * whose set_gdi_color_format() (src/win/gdi.c) selects the layouts the
    * compat layer implements; reproduce that here. */
   _rgb_r_shift_15 = 10; _rgb_g_shift_15 = 5; _rgb_b_shift_15 = 0;
   _rgb_r_shift_16 = 11; _rgb_g_shift_16 = 5; _rgb_b_shift_16 = 0;
   _rgb_r_shift_24 = 16; _rgb_g_shift_24 = 8; _rgb_b_shift_24 = 0;
   _rgb_r_shift_32 = 16; _rgb_g_shift_32 = 8; _rgb_b_shift_32 = 0;
   _rgb_a_shift_32 = 24;
#endif
   scen_fixed(g);
   scen_blend(g);
   scen_prims(g);
   scen_blit(g);
   scen_convert(g);
   scen_sprites(g);
   scen_stretch(g);
   scen_rotate(g);
   solid_mode();
   set_color_depth(32);
}

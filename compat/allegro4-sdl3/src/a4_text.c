/*
 * FONT objects, the default 8x8 font, text output and metrics, and the
 * Allegro UTF-8 string helpers used by the text and file modules.
 *
 * Semantics follow Allegro 4.4.1 (giftware licence): font.c (vtables,
 * missing-glyph rule), text.c (textout/textprintf), glyph.c (mono glyph
 * blitter), c/cspr.h (_linear_draw_256_sprite, _linear_draw_character),
 * gfx.c (_normal_rectfill) and unicode.c (UTF-8 codec, string helpers).
 * Glyph drawing is implemented here (no dependency on the sprite/blit
 * modules); every drawing entry point calls a4_touch() on its target.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "a4_internal.h"

/* ================================================================== */
/* UTF-8 helpers (ported from unicode.c: utf8_getc/getx/setc/width/   */
/* cwidth, ustrlen, uoffset, ugetat, ustrzcpy, ustrzcat, ustrzncpy,   */
/* ustricmp, utolower, uisspace, do_uconvert to U_ASCII)              */
/* ================================================================== */

/* utf8_getx (unicode.c): note the pointer is left on a bad continuation
 * byte and '^' is returned; arithmetic wraps like Allegro's 32-bit int. */
int a4_ugetx(const char **s)
{
   const unsigned char *p = (const unsigned char *)*s;
   unsigned int c = *p++;
   int n;
   unsigned int t;

   if (c & 0x80) {
      n = 1;
      while (n < 8 && (c & (0x80u >> n)))
         n++;
      c &= (1u << (8 - n)) - 1;
      while (--n > 0) {
         t = *p++;
         if ((!(t & 0x80)) || (t & 0x40)) {
            p--;
            *s = (const char *)p;
            return '^';
         }
         c = (c << 6) | (t & 0x3F);
      }
   }
   *s = (const char *)p;
   return (int)c;
}

/* utf8_getc (unicode.c) */
int a4_ugetc(const char *s)
{
   return a4_ugetx(&s);
}

/* utf8_setc (unicode.c) */
int a4_usetc(char *s, int c)
{
   int size, bits, b, i;
   unsigned int uc = (unsigned int)c;

   if (c < 128) {
      *s = (char)c;
      return 1;
   }
   bits = 7;
   while (bits < 31 && uc >= (1u << bits))
      bits++;
   size = 2;
   b = 11;
   while (b < bits) {
      size++;
      b += 5;
   }
   b -= (7 - size);
   s[0] = (char)(uc >> b);
   for (i = 0; i < size; i++)
      s[0] = (char)(s[0] | (0x80 >> i));
   for (i = 1; i < size; i++) {
      b -= 6;
      s[i] = (char)(0x80 | ((uc >> b) & 0x3F));
   }
   return size;
}

/* utf8_cwidth (unicode.c) */
int a4_ucwidth(int c)
{
   int size, bits, b;
   unsigned int uc = (unsigned int)c;

   if (c < 128)
      return 1;
   bits = 7;
   while (bits < 31 && uc >= (1u << bits))
      bits++;
   size = 2;
   b = 11;
   while (b < bits) {
      size++;
      b += 5;
   }
   return size;
}

/* utf8_width (unicode.c) */
int a4_uwidth(const char *s)
{
   int c = *(const unsigned char *)s;
   int n = 1;
   if (c & 0x80) {
      while (n < 8 && (c & (0x80 >> n)))
         n++;
   }
   return n;
}

int a4_ustrlen(const char *s)
{
   int c = 0;
   while (a4_ugetx(&s))
      c++;
   return c;
}

int a4_ustrsize(const char *s)
{
   const char *orig = s, *last;
   do {
      last = s;
   } while (a4_ugetx(&s) != 0);
   return (int)(last - orig);
}

int a4_uoffset(const char *s, int index)
{
   const char *orig = s, *last;
   if (index < 0)
      index += a4_ustrlen(s);
   while (index-- > 0) {
      last = s;
      if (!a4_ugetx(&s)) {
         s = last;
         break;
      }
   }
   return (int)(s - orig);
}

int a4_ugetat(const char *s, int index)
{
   return a4_ugetc(s + a4_uoffset(s, index));
}

/* utolower (unicode.c), complete table */
int a4_utolower(int c)
{
   if ((c >= 65 && c <= 90) || (c >= 192 && c <= 214) || (c >= 216 && c <= 222) ||
       (c >= 913 && c <= 929) || (c >= 931 && c <= 939) || (c >= 1040 && c <= 1071))
      return c + 32;
   if ((c >= 393 && c <= 394))
      return c + 205;
   if ((c >= 433 && c <= 434))
      return c + 217;
   if ((c >= 904 && c <= 906))
      return c + 37;
   if ((c >= 910 && c <= 911))
      return c + 63;
   if ((c >= 1025 && c <= 1036) || (c >= 1038 && c <= 1039))
      return c + 80;
   if ((c >= 1329 && c <= 1366) || (c >= 4256 && c <= 4293))
      return c + 48;
   if ((c >= 7944 && c <= 7951) || (c >= 7960 && c <= 7965) || (c >= 7976 && c <= 7983) ||
       (c >= 7992 && c <= 7999) || (c >= 8008 && c <= 8013) || (c >= 8040 && c <= 8047) ||
       (c >= 8072 && c <= 8079) || (c >= 8088 && c <= 8095) || (c >= 8104 && c <= 8111) ||
       (c >= 8120 && c <= 8121) || (c >= 8152 && c <= 8153) || (c >= 8168 && c <= 8169))
      return c + -8;
   if ((c >= 8122 && c <= 8123))
      return c + -74;
   if ((c >= 8136 && c <= 8139))
      return c + -86;
   if ((c >= 8154 && c <= 8155))
      return c + -100;
   if ((c >= 8170 && c <= 8171))
      return c + -112;
   if ((c >= 8184 && c <= 8185))
      return c + -128;
   if ((c >= 8186 && c <= 8187))
      return c + -126;
   if ((c >= 8544 && c <= 8559))
      return c + 16;
   if ((c >= 9398 && c <= 9423))
      return c + 26;

   switch (c) {
      case 256: case 258: case 260: case 262: case 264: case 266: case 268: case 270:
      case 272: case 274: case 276: case 278: case 280: case 282: case 284: case 286:
      case 288: case 290: case 292: case 294: case 296: case 298: case 300: case 302:
      case 306: case 308: case 310: case 313: case 315: case 317: case 319: case 321:
      case 323: case 325: case 327: case 330: case 332: case 334: case 336: case 338:
      case 340: case 342: case 344: case 346: case 348: case 350: case 352: case 354:
      case 356: case 358: case 360: case 362: case 364: case 366: case 368: case 370:
      case 372: case 374: case 377: case 379: case 381: case 386: case 388: case 391:
      case 395: case 401: case 408: case 416: case 418: case 420: case 423: case 428:
      case 431: case 435: case 437: case 440: case 444: case 453: case 456: case 459:
      case 461: case 463: case 465: case 467: case 469: case 471: case 473: case 475:
      case 478: case 480: case 482: case 484: case 486: case 488: case 490: case 492:
      case 494: case 498: case 500: case 506: case 508: case 510: case 512: case 514:
      case 516: case 518: case 520: case 522: case 524: case 526: case 528: case 530:
      case 532: case 534: case 994: case 996: case 998: case 1000: case 1002: case 1004:
      case 1006: case 1120: case 1122: case 1124: case 1126: case 1128: case 1130:
      case 1132: case 1134: case 1136: case 1138: case 1140: case 1142: case 1144:
      case 1146: case 1148: case 1150: case 1152: case 1168: case 1170: case 1172:
      case 1174: case 1176: case 1178: case 1180: case 1182: case 1184: case 1186:
      case 1188: case 1190: case 1192: case 1194: case 1196: case 1198: case 1200:
      case 1202: case 1204: case 1206: case 1208: case 1210: case 1212: case 1214:
      case 1217: case 1219: case 1223: case 1227: case 1232: case 1234: case 1236:
      case 1238: case 1240: case 1242: case 1244: case 1246: case 1248: case 1250:
      case 1252: case 1254: case 1256: case 1258: case 1262: case 1264: case 1266:
      case 1268: case 1272:
      case 7680: case 7682: case 7684: case 7686: case 7688: case 7690: case 7692:
      case 7694: case 7696: case 7698: case 7700: case 7702: case 7704: case 7706:
      case 7708: case 7710: case 7712: case 7714: case 7716: case 7718: case 7720:
      case 7722: case 7724: case 7726: case 7728: case 7730: case 7732: case 7734:
      case 7736: case 7738: case 7740: case 7742: case 7744: case 7746: case 7748:
      case 7750: case 7752: case 7754: case 7756: case 7758: case 7760: case 7762:
      case 7764: case 7766: case 7768: case 7770: case 7772: case 7774: case 7776:
      case 7778: case 7780: case 7782: case 7784: case 7786: case 7788: case 7790:
      case 7792: case 7794: case 7796: case 7798: case 7800: case 7802: case 7804:
      case 7806: case 7808: case 7810: case 7812: case 7814: case 7816: case 7818:
      case 7820: case 7822: case 7824: case 7826: case 7828: case 7840: case 7842:
      case 7844: case 7846: case 7848: case 7850: case 7852: case 7854: case 7856:
      case 7858: case 7860: case 7862: case 7864: case 7866: case 7868: case 7870:
      case 7872: case 7874: case 7876: case 7878: case 7880: case 7882: case 7884:
      case 7886: case 7888: case 7890: case 7892: case 7894: case 7896: case 7898:
      case 7900: case 7902: case 7904: case 7906: case 7908: case 7910: case 7912:
      case 7914: case 7916: case 7918: case 7920: case 7922: case 7924: case 7926:
      case 7928:
         return c + 1;
      case 304: return c + -199;
      case 376: return c + -121;
      case 385: return c + 210;
      case 390: return c + 206;
      case 398: return c + 79;
      case 399: return c + 202;
      case 400: return c + 203;
      case 403: return c + 205;
      case 404: return c + 207;
      case 406: case 412: return c + 211;
      case 407: return c + 209;
      case 413: return c + 213;
      case 415: return c + 214;
      case 422: case 425: case 430: return c + 218;
      case 439: return c + 219;
      case 452: case 455: case 458: case 497: return c + 2;
      case 902: return c + 38;
      case 908: return c + 64;
      case 8025: case 8027: case 8029: case 8031: return c + -8;
      case 8124: case 8140: case 8188: return c + -9;
      case 8172: return c + -7;
      default: return c;
   }
}

int a4_uisspace(int c)
{
   return ((c == ' ') || (c == '\t') || (c == '\r') || (c == '\n') || (c == '\f') ||
           (c == '\v') || (c == 0x1680) || ((c >= 0x2000) && (c <= 0x200A)) ||
           (c == 0x2028) || (c == 0x202f) || (c == 0x3000));
}

int a4_ustricmp(const char *s1, const char *s2)
{
   int c1, c2;
   for (;;) {
      c1 = a4_utolower(a4_ugetx(&s1));
      c2 = a4_utolower(a4_ugetx(&s2));
      if (c1 != c2)
         return c1 - c2;
      if (!c1)
         return 0;
   }
}

char *a4_ustrzcpy(char *dest, int size, const char *src)
{
   int pos = 0, c;
   size -= a4_ucwidth(0);
   while ((c = a4_ugetx(&src)) != 0) {
      size -= a4_ucwidth(c);
      if (size < 0)
         break;
      pos += a4_usetc(dest + pos, c);
   }
   a4_usetc(dest + pos, 0);
   return dest;
}

char *a4_ustrzcat(char *dest, int size, const char *src)
{
   int pos, c;
   pos = a4_ustrsize(dest);
   size -= pos + a4_ucwidth(0);
   while ((c = a4_ugetx(&src)) != 0) {
      size -= a4_ucwidth(c);
      if (size < 0)
         break;
      pos += a4_usetc(dest + pos, c);
   }
   a4_usetc(dest + pos, 0);
   return dest;
}

/* ustrzncpy (never called with size == INT_MAX here, so always terminated) */
char *a4_ustrzncpy(char *dest, int size, const char *src, int n)
{
   int pos = 0, len = 0, c;
   size -= a4_ucwidth(0);
   while (((c = a4_ugetx(&src)) != 0) && (len < n)) {
      size -= a4_ucwidth(c);
      if (size < 0)
         break;
      pos += a4_usetc(dest + pos, c);
      len++;
   }
   while (len < n) {
      size -= a4_ucwidth(0);
      if (size < 0)
         break;
      pos += a4_usetc(dest + pos, 0);
      len++;
   }
   a4_usetc(dest + pos, 0);
   return dest;
}

/* do_uconvert(s, U_UTF8, dest, U_ASCII, size) */
char *a4_utoascii(const char *s, char *dest, int size)
{
   int pos = 0, c;
   size -= 1;
   while ((c = a4_ugetx(&s)) != 0) {
      if (c < 0 || c > 255)
         c = '^';
      size -= 1;
      if (size < 0)
         break;
      dest[pos++] = (char)c;
   }
   dest[pos] = 0;
   return dest;
}

/* ================================================================== */
/* fonts                                                              */
/* ================================================================== */

#include "a4_font8x8.inc"

static A4_FONT_RANGE a4_default_ranges[4] = {
   { 0x20, 0x80, &a4_font8x8_glyphs[0] },
   { 0xA1, 0x100, &a4_font8x8_glyphs[96] },
   { 0x100, 0x180, &a4_font8x8_glyphs[191] },
   { 0x20AC, 0x20AD, &a4_font8x8_glyphs[319] }
};

static FONT a4_default_font = { 8, 0, 4, a4_default_ranges, 0xFFFFFF00u };

FONT *font = &a4_default_font;

static uint32_t font_serial = 1;

FONT *a4_font_create(int height, int is_color, int nranges)
{
   FONT *f = (FONT *)calloc(1, sizeof(FONT));
   if (!f)
      return NULL;
   if (nranges > 0) {
      f->ranges = (A4_FONT_RANGE *)calloc((size_t)nranges, sizeof(A4_FONT_RANGE));
      if (!f->ranges) {
         free(f);
         return NULL;
      }
   }
   f->height = height;
   f->is_color = is_color;
   f->nranges = nranges;
   f->serial = font_serial++;
   return f;
}

void destroy_font(FONT *f)
{
   int r, i;
   if (!f || f == &a4_default_font)
      return;
   for (r = 0; r < f->nranges; r++) {
      A4_FONT_RANGE *rg = &f->ranges[r];
      if (!rg->glyphs)
         continue;
      for (i = 0; i < rg->end - rg->begin; i++) {
         free(rg->glyphs[i].mono);
         if (rg->glyphs[i].bmp)
            destroy_bitmap(rg->glyphs[i].bmp);
      }
      free(rg->glyphs);
   }
   free(f->ranges);
   free(f);
}

/* _mono_find_glyph / _color_find_glyph (font.c): allegro_404_char = '^' */
const A4_GLYPH *a4_font_glyph(const FONT *f, int ch)
{
   int r;
   for (r = 0; r < f->nranges; r++) {
      const A4_FONT_RANGE *rg = &f->ranges[r];
      if (ch >= rg->begin && ch < rg->end && rg->glyphs)
         return &rg->glyphs[ch - rg->begin];
   }
   if (ch != '^')
      return a4_font_glyph(f, '^');
   return NULL;
}

static int glyph_width(const FONT *f, const A4_GLYPH *g)
{
   if (!g)
      return 0;
   if (f->is_color)
      return g->bmp ? g->bmp->w : 0;
   return g->w;
}

int text_length(const FONT *f, const char *str)
{
   int ch, w = 0;
   const char *p = str;
   while ((ch = a4_ugetx(&p)) != 0)
      w += glyph_width(f, a4_font_glyph(f, ch));
   return w;
}

int text_height(const FONT *f)
{
   return f->height;
}

/* ------------------------------------------------------------------ */
/* low-level glyph drawing                                            */
/* ------------------------------------------------------------------ */

/* clip rectangle actually used: Allegro does no clipping at all when
 * bmp->clip is off; we then clip to the bitmap to stay memory safe. */
static void clip_box(BITMAP *bmp, int *cl, int *ct, int *cr, int *cb)
{
   if (bmp->clip) {
      *cl = bmp->cl; *ct = bmp->ct; *cr = bmp->cr; *cb = bmp->cb;
   } else {
      *cl = 0; *ct = 0; *cr = bmp->w; *cb = bmp->h;
   }
}

/* hfill with the current drawing mode (c/cgfx.h _linear_hline) */
static void hfill(BITMAP *bmp, int x1, int y, int x2, int color)
{
   int x;
   for (x = x1; x <= x2; x++) {
      if (a4_draw_mode == DRAW_MODE_XOR) {
         a4_put_raw(bmp, x, y, a4_get_raw(bmp, x, y) ^ (unsigned long)color);
      } else if (a4_draw_mode == DRAW_MODE_TRANS && bmp->depth != 8) {
         a4_put_raw(bmp, x, y, a4_blend(bmp->depth, (unsigned long)color, a4_get_raw(bmp, x, y),
                                        (unsigned long)a4_blend_a));
      } else {
         a4_put_raw(bmp, x, y, (unsigned long)color);
      }
   }
}

/* _normal_rectfill (gfx.c) */
static void fill_rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   int t, cl, ct, cr, cb;
   if (y1 > y2) { t = y1; y1 = y2; y2 = t; }
   if (x1 > x2) { t = x1; x1 = x2; x2 = t; }
   clip_box(bmp, &cl, &ct, &cr, &cb);
   if (x1 < cl) x1 = cl;
   if (x2 >= cr) x2 = cr - 1;
   if (x2 < x1) return;
   if (y1 < ct) y1 = ct;
   if (y2 >= cb) y2 = cb - 1;
   if (y2 < y1) return;
   while (y1 <= y2) {
      hfill(bmp, x1, y1, x2, color);
      y1++;
   }
}

/* DRAW_GLYPH (glyph.c): 1 bpp glyph, MSB first, opaque when bg >= 0 */
static void draw_mono_glyph(BITMAP *bmp, const A4_GLYPH *g, int x, int y, int color, int bg)
{
   const unsigned char *data = g->mono;
   int w = g->w, h = g->h;
   int stride = (w + 7) / 8;
   int lgap = 0, d, i, j, px;
   int cl, ct, cr, cb;

   if (!data || w <= 0 || h <= 0)
      return;   /* Allegro's loop never terminates for w == 0 */
   clip_box(bmp, &cl, &ct, &cr, &cb);
   if (y < ct) {
      d = ct - y;
      h -= d;
      if (h <= 0)
         return;
      data += d * stride;
      y = ct;
   }
   if (y + h >= cb) {
      h = cb - y;
      if (h <= 0)
         return;
   }
   if (x < cl) {
      d = cl - x;
      w -= d;
      if (w <= 0)
         return;
      data += d / 8;
      lgap = d & 7;
      x = cl;
   }
   if (x + w >= cr) {
      w = cr - x;
      if (w <= 0)
         return;
   }
   stride -= (lgap + w + 7) / 8;

   while (h--) {
      px = x;
      j = 0;
      i = 0x80 >> lgap;
      d = *(data++);
      for (;;) {
         if (d & i)
            a4_put_raw(bmp, px, y, (unsigned long)color);
         else if (bg >= 0)
            a4_put_raw(bmp, px, y, (unsigned long)bg);
         j++;
         if (j == w)
            break;
         i >>= 1;
         if (!i) {
            i = 0x80;
            d = *(data++);
         }
         px++;
      }
      data += stride;
      y++;
   }
}

/* source/destination clipping shared by the sprite-like blitters
 * (c/cspr.h); returns 0 when nothing is visible */
static int clip_sprite(BITMAP *dst, BITMAP *src, int dx, int dy,
                       int *sxbeg, int *sybeg, int *dxbeg, int *dybeg, int *w, int *h)
{
   int tmp, cl, ct, cr, cb;
   clip_box(dst, &cl, &ct, &cr, &cb);
   tmp = cl - dx;
   *sxbeg = ((tmp < 0) ? 0 : tmp);
   *dxbeg = *sxbeg + dx;
   tmp = cr - dx;
   *w = ((tmp > src->w) ? src->w : tmp) - *sxbeg;
   if (*w <= 0)
      return 0;
   tmp = ct - dy;
   *sybeg = ((tmp < 0) ? 0 : tmp);
   *dybeg = *sybeg + dy;
   tmp = cb - dy;
   *h = ((tmp > src->h) ? src->h : tmp) - *sybeg;
   if (*h <= 0)
      return 0;
   return 1;
}

/* _linear_draw_256_sprite: index 0 is transparent, other indices go
 * through the current palette (palette expansion table) */
static void draw_256_sprite(BITMAP *dst, BITMAP *src, int dx, int dy)
{
   int x, y, w, h, sxbeg, sybeg, dxbeg, dybeg;
   int table[256];
   if (!clip_sprite(dst, src, dx, dy, &sxbeg, &sybeg, &dxbeg, &dybeg, &w, &h))
      return;
   for (x = 0; x < 256; x++)
      table[x] = a4_palette_color_depth(dst->depth, x);
   for (y = 0; y < h; y++) {
      const unsigned char *s = src->line[sybeg + y] + sxbeg;
      for (x = 0; x < w; x++) {
         int c = s[x];
         if (c != 0)
            a4_put_raw(dst, dxbeg + x, dybeg + y, (unsigned long)table[c]);
      }
   }
}

/* _linear_draw_character: non-zero pixels in `color`, zero pixels in bg
 * when bg >= 0 */
static void draw_character(BITMAP *dst, BITMAP *src, int dx, int dy, int color, int bg)
{
   int x, y, w, h, sxbeg, sybeg, dxbeg, dybeg;
   if (!clip_sprite(dst, src, dx, dy, &sxbeg, &sybeg, &dxbeg, &dybeg, &w, &h))
      return;
   for (y = 0; y < h; y++) {
      const unsigned char *s = src->line[sybeg + y] + sxbeg;
      for (x = 0; x < w; x++) {
         if (s[x] != 0)
            a4_put_raw(dst, dxbeg + x, dybeg + y, (unsigned long)color);
         else if (bg >= 0)
            a4_put_raw(dst, dxbeg + x, dybeg + y, (unsigned long)bg);
      }
   }
}

/* masked_blit of a whole same-depth glyph (mask colour of the depth) */
static void masked_glyph(BITMAP *dst, BITMAP *src, int dx, int dy)
{
   int x, y, w, h, sxbeg, sybeg, dxbeg, dybeg;
   unsigned long mask = a4_mask_color(src->depth);
   if (!clip_sprite(dst, src, dx, dy, &sxbeg, &sybeg, &dxbeg, &dybeg, &w, &h))
      return;
   for (y = 0; y < h; y++)
      for (x = 0; x < w; x++) {
         unsigned long c = a4_get_raw(src, sxbeg + x, sybeg + y);
         if (c != mask)
            a4_put_raw(dst, dxbeg + x, dybeg + y, c);
      }
}

/* color_render_char (font.c), for fg/bg already adjusted by color_render */
static int color_render_char(const FONT *f, int ch, int fg, int bg, BITMAP *bmp, int x, int y)
{
   int h = f->height;
   const A4_GLYPH *gl;
   BITMAP *g;

   if (fg < 0 && bg >= 0)
      fill_rect(bmp, x, y, x + glyph_width(f, a4_font_glyph(f, ch)) - 1, y + h - 1, bg);

   gl = a4_font_glyph(f, ch);
   if (!gl || !gl->bmp)
      return 0;
   g = gl->bmp;
   if (g->depth == 8) {
      if (fg < 0)
         draw_256_sprite(bmp, g, x, y + (h - g->h) / 2);
      else
         draw_character(bmp, g, x, y + (h - g->h) / 2, fg, bg);
   } else if (g->depth == bmp->depth) {
      masked_glyph(bmp, g, x, y + (h - g->h) / 2);
   } else {
      /* colour conversion with COLORCONV_KEEP_TRANS, then masked */
      BITMAP *tbmp = create_bitmap_ex(bmp->depth, g->w, g->h);
      if (tbmp) {
         a4_convert_blit(g, tbmp, COLORCONV_KEEP_TRANS);
         masked_glyph(bmp, tbmp, x, y + (h - g->h) / 2);
         destroy_bitmap(tbmp);
      }
   }
   return g->w;
}

static void render(BITMAP *bmp, const FONT *f, const char *str, int x, int y, int color, int bg)
{
   const char *p = str;
   int ch;
   if (!bmp || !f || !str)
      return;
   if (f->is_color) {
      /* color_render */
      if (color < 0 && bg >= 0) {
         fill_rect(bmp, x, y, x + text_length(f, str) - 1, y + text_height(f) - 1, bg);
         bg = -1;
      }
      while ((ch = a4_ugetx(&p)) != 0)
         x += color_render_char(f, ch, color, bg, bmp, x, y);
   } else {
      /* mono_render / mono_render_char */
      while ((ch = a4_ugetx(&p)) != 0) {
         const A4_GLYPH *g = a4_font_glyph(f, ch);
         if (g) {
            draw_mono_glyph(bmp, g, x, y + (f->height - g->h) / 2, color, bg);
            x += g->w;
         }
      }
   }
   a4_touch(bmp);
}

/* ------------------------------------------------------------------ */
/* text.c                                                             */
/* ------------------------------------------------------------------ */

void textout_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int bg)
{
   render(bmp, f, s, x, y, color, bg);
}

void textout_centre_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int bg)
{
   int len = text_length(f, s);
   render(bmp, f, s, x - len / 2, y, color, bg);
}

void textout_right_ex(BITMAP *bmp, const FONT *f, const char *s, int x, int y, int color, int bg)
{
   int len = text_length(f, s);
   render(bmp, f, s, x - len, y, color, bg);
}

/* Allegro formats with uvszprintf() into a 512 byte buffer; for UTF-8
 * text the result renders identically to vsnprintf()'s. */
void textprintf_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int bg, const char *format, ...)
{
   char buf[512];
   va_list ap;
   va_start(ap, format);
   vsnprintf(buf, sizeof(buf), format, ap);
   va_end(ap);
   textout_ex(bmp, f, buf, x, y, color, bg);
}

void textprintf_centre_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int bg, const char *format, ...)
{
   char buf[512];
   va_list ap;
   va_start(ap, format);
   vsnprintf(buf, sizeof(buf), format, ap);
   va_end(ap);
   textout_centre_ex(bmp, f, buf, x, y, color, bg);
}

void textprintf_right_ex(BITMAP *bmp, const FONT *f, int x, int y, int color, int bg, const char *format, ...)
{
   char buf[512];
   va_list ap;
   va_start(ap, format);
   vsnprintf(buf, sizeof(buf), format, ap);
   va_end(ap);
   textout_right_ex(bmp, f, buf, x, y, color, bg);
}

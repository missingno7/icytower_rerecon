/*
 * Bitmaps, colour depth state, makecol/getr, palettes and colour conversion.
 * Semantics follow Allegro 4.4.1 graphics.c / color.c / gfx.c (giftware).
 */
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "a4_internal.h"

int a4_color_depth = 8;
int a4_color_conversion = COLORCONV_TOTAL;
PALETTE a4_current_palette;
int a4_palette_color[256];

static PALETTE prev_palette;
static int prev_palette_color[256];
static uint32_t next_serial = 1;

int _rgb_scale_1[2] = { 0, 255 };
int _rgb_scale_4[16] = { 0, 16, 32, 49, 65, 82, 98, 115, 139, 156, 172, 189, 205, 222, 238, 255 };
int _rgb_scale_5[32] = {
   0,   8,   16,  24,  33,  41,  49,  57,
   66,  74,  82,  90,  99,  107, 115, 123,
   132, 140, 148, 156, 165, 173, 181, 189,
   198, 206, 214, 222, 231, 239, 247, 255
};
int _rgb_scale_6[64] = {
   0,   4,   8,   12,  16,  20,  24,  28,
   32,  36,  40,  44,  48,  52,  56,  60,
   65,  69,  73,  77,  81,  85,  89,  93,
   97,  101, 105, 109, 113, 117, 121, 125,
   130, 134, 138, 142, 146, 150, 154, 158,
   162, 166, 170, 174, 178, 182, 186, 190,
   195, 199, 203, 207, 211, 215, 219, 223,
   227, 231, 235, 239, 243, 247, 251, 255
};

PALETTE desktop_palette = {
   { 63, 63, 63, 0 },   { 63, 0,  0,  0 },   { 0,  63, 0,  0 },   { 63, 63, 0,  0 },
   { 0,  0,  63, 0 },   { 63, 0,  63, 0 },   { 0,  63, 63, 0 },   { 16, 16, 16, 0 },
   { 31, 31, 31, 0 },   { 63, 31, 31, 0 },   { 31, 63, 31, 0 },   { 63, 63, 31, 0 },
   { 31, 31, 63, 0 },   { 63, 31, 63, 0 },   { 31, 63, 63, 0 },   { 0,  0,  0,  0 }
};

/* Allegro default_palette (allegro.c) */
PALETTE a4_default_palette =
{
   { 0,  0,  0,  0 },   { 0,  0,  42, 0 },   { 0,  42, 0,  0 },   { 0,  42, 42, 0 }, 
   { 42, 0,  0,  0 },   { 42, 0,  42, 0 },   { 42, 21, 0,  0 },   { 42, 42, 42, 0 }, 
   { 21, 21, 21, 0 },   { 21, 21, 63, 0 },   { 21, 63, 21, 0 },   { 21, 63, 63, 0 }, 
   { 63, 21, 21, 0 },   { 63, 21, 63, 0 },   { 63, 63, 21, 0 },   { 63, 63, 63, 0 }, 
   { 0,  0,  0,  0 },   { 5,  5,  5,  0 },   { 8,  8,  8,  0 },   { 11, 11, 11, 0 }, 
   { 14, 14, 14, 0 },   { 17, 17, 17, 0 },   { 20, 20, 20, 0 },   { 24, 24, 24, 0 }, 
   { 28, 28, 28, 0 },   { 32, 32, 32, 0 },   { 36, 36, 36, 0 },   { 40, 40, 40, 0 }, 
   { 45, 45, 45, 0 },   { 50, 50, 50, 0 },   { 56, 56, 56, 0 },   { 63, 63, 63, 0 }, 
   { 0,  0,  63, 0 },   { 16, 0,  63, 0 },   { 31, 0,  63, 0 },   { 47, 0,  63, 0 }, 
   { 63, 0,  63, 0 },   { 63, 0,  47, 0 },   { 63, 0,  31, 0 },   { 63, 0,  16, 0 }, 
   { 63, 0,  0,  0 },   { 63, 16, 0,  0 },   { 63, 31, 0,  0 },   { 63, 47, 0,  0 }, 
   { 63, 63, 0,  0 },   { 47, 63, 0,  0 },   { 31, 63, 0,  0 },   { 16, 63, 0,  0 }, 
   { 0,  63, 0,  0 },   { 0,  63, 16, 0 },   { 0,  63, 31, 0 },   { 0,  63, 47, 0 }, 
   { 0,  63, 63, 0 },   { 0,  47, 63, 0 },   { 0,  31, 63, 0 },   { 0,  16, 63, 0 }, 
   { 31, 31, 63, 0 },   { 39, 31, 63, 0 },   { 47, 31, 63, 0 },   { 55, 31, 63, 0 }, 
   { 63, 31, 63, 0 },   { 63, 31, 55, 0 },   { 63, 31, 47, 0 },   { 63, 31, 39, 0 }, 
   { 63, 31, 31, 0 },   { 63, 39, 31, 0 },   { 63, 47, 31, 0 },   { 63, 55, 31, 0 }, 
   { 63, 63, 31, 0 },   { 55, 63, 31, 0 },   { 47, 63, 31, 0 },   { 39, 63, 31, 0 }, 
   { 31, 63, 31, 0 },   { 31, 63, 39, 0 },   { 31, 63, 47, 0 },   { 31, 63, 55, 0 }, 
   { 31, 63, 63, 0 },   { 31, 55, 63, 0 },   { 31, 47, 63, 0 },   { 31, 39, 63, 0 }, 
   { 45, 45, 63, 0 },   { 49, 45, 63, 0 },   { 54, 45, 63, 0 },   { 58, 45, 63, 0 }, 
   { 63, 45, 63, 0 },   { 63, 45, 58, 0 },   { 63, 45, 54, 0 },   { 63, 45, 49, 0 }, 
   { 63, 45, 45, 0 },   { 63, 49, 45, 0 },   { 63, 54, 45, 0 },   { 63, 58, 45, 0 }, 
   { 63, 63, 45, 0 },   { 58, 63, 45, 0 },   { 54, 63, 45, 0 },   { 49, 63, 45, 0 }, 
   { 45, 63, 45, 0 },   { 45, 63, 49, 0 },   { 45, 63, 54, 0 },   { 45, 63, 58, 0 }, 
   { 45, 63, 63, 0 },   { 45, 58, 63, 0 },   { 45, 54, 63, 0 },   { 45, 49, 63, 0 }, 
   { 0,  0,  28, 0 },   { 7,  0,  28, 0 },   { 14, 0,  28, 0 },   { 21, 0,  28, 0 }, 
   { 28, 0,  28, 0 },   { 28, 0,  21, 0 },   { 28, 0,  14, 0 },   { 28, 0,  7,  0 }, 
   { 28, 0,  0,  0 },   { 28, 7,  0,  0 },   { 28, 14, 0,  0 },   { 28, 21, 0,  0 }, 
   { 28, 28, 0,  0 },   { 21, 28, 0,  0 },   { 14, 28, 0,  0 },   { 7,  28, 0,  0 }, 
   { 0,  28, 0,  0 },   { 0,  28, 7,  0 },   { 0,  28, 14, 0 },   { 0,  28, 21, 0 }, 
   { 0,  28, 28, 0 },   { 0,  21, 28, 0 },   { 0,  14, 28, 0 },   { 0,  7,  28, 0 }, 
   { 14, 14, 28, 0 },   { 17, 14, 28, 0 },   { 21, 14, 28, 0 },   { 24, 14, 28, 0 }, 
   { 28, 14, 28, 0 },   { 28, 14, 24, 0 },   { 28, 14, 21, 0 },   { 28, 14, 17, 0 }, 
   { 28, 14, 14, 0 },   { 28, 17, 14, 0 },   { 28, 21, 14, 0 },   { 28, 24, 14, 0 }, 
   { 28, 28, 14, 0 },   { 24, 28, 14, 0 },   { 21, 28, 14, 0 },   { 17, 28, 14, 0 }, 
   { 14, 28, 14, 0 },   { 14, 28, 17, 0 },   { 14, 28, 21, 0 },   { 14, 28, 24, 0 }, 
   { 14, 28, 28, 0 },   { 14, 24, 28, 0 },   { 14, 21, 28, 0 },   { 14, 17, 28, 0 }, 
   { 20, 20, 28, 0 },   { 22, 20, 28, 0 },   { 24, 20, 28, 0 },   { 26, 20, 28, 0 }, 
   { 28, 20, 28, 0 },   { 28, 20, 26, 0 },   { 28, 20, 24, 0 },   { 28, 20, 22, 0 }, 
   { 28, 20, 20, 0 },   { 28, 22, 20, 0 },   { 28, 24, 20, 0 },   { 28, 26, 20, 0 }, 
   { 28, 28, 20, 0 },   { 26, 28, 20, 0 },   { 24, 28, 20, 0 },   { 22, 28, 20, 0 }, 
   { 20, 28, 20, 0 },   { 20, 28, 22, 0 },   { 20, 28, 24, 0 },   { 20, 28, 26, 0 }, 
   { 20, 28, 28, 0 },   { 20, 26, 28, 0 },   { 20, 24, 28, 0 },   { 20, 22, 28, 0 }, 
   { 0,  0,  16, 0 },   { 4,  0,  16, 0 },   { 8,  0,  16, 0 },   { 12, 0,  16, 0 }, 
   { 16, 0,  16, 0 },   { 16, 0,  12, 0 },   { 16, 0,  8,  0 },   { 16, 0,  4,  0 }, 
   { 16, 0,  0,  0 },   { 16, 4,  0,  0 },   { 16, 8,  0,  0 },   { 16, 12, 0,  0 }, 
   { 16, 16, 0,  0 },   { 12, 16, 0,  0 },   { 8,  16, 0,  0 },   { 4,  16, 0,  0 }, 
   { 0,  16, 0,  0 },   { 0,  16, 4,  0 },   { 0,  16, 8,  0 },   { 0,  16, 12, 0 }, 
   { 0,  16, 16, 0 },   { 0,  12, 16, 0 },   { 0,  8,  16, 0 },   { 0,  4,  16, 0 }, 
   { 8,  8,  16, 0 },   { 10, 8,  16, 0 },   { 12, 8,  16, 0 },   { 14, 8,  16, 0 }, 
   { 16, 8,  16, 0 },   { 16, 8,  14, 0 },   { 16, 8,  12, 0 },   { 16, 8,  10, 0 }, 
   { 16, 8,  8,  0 },   { 16, 10, 8,  0 },   { 16, 12, 8,  0 },   { 16, 14, 8,  0 }, 
   { 16, 16, 8,  0 },   { 14, 16, 8,  0 },   { 12, 16, 8,  0 },   { 10, 16, 8,  0 }, 
   { 8,  16, 8,  0 },   { 8,  16, 10, 0 },   { 8,  16, 12, 0 },   { 8,  16, 14, 0 }, 
   { 8,  16, 16, 0 },   { 8,  14, 16, 0 },   { 8,  12, 16, 0 },   { 8,  10, 16, 0 }, 
   { 11, 11, 16, 0 },   { 12, 11, 16, 0 },   { 13, 11, 16, 0 },   { 15, 11, 16, 0 }, 
   { 16, 11, 16, 0 },   { 16, 11, 15, 0 },   { 16, 11, 13, 0 },   { 16, 11, 12, 0 }, 
   { 16, 11, 11, 0 },   { 16, 12, 11, 0 },   { 16, 13, 11, 0 },   { 16, 15, 11, 0 }, 
   { 16, 16, 11, 0 },   { 15, 16, 11, 0 },   { 13, 16, 11, 0 },   { 12, 16, 11, 0 }, 
   { 11, 16, 11, 0 },   { 11, 16, 12, 0 },   { 11, 16, 13, 0 },   { 11, 16, 15, 0 }, 
   { 11, 16, 16, 0 },   { 11, 15, 16, 0 },   { 11, 13, 16, 0 },   { 11, 12, 16, 0 }, 
   { 0,  0,  0,  0 },   { 0,  0,  0,  0 },   { 0,  0,  0,  0 },   { 0,  0,  0,  0 }, 
   { 0,  0,  0,  0 },   { 0,  0,  0,  0 },   { 0,  0,  0,  0 },   { 63, 63, 63, 0 }
};

/* allegro_init() part: desktop palette repeats its 16 entries */
void a4_bitmap_init(void)
{
   int i;
   for (i = 16; i < 256; i++)
      desktop_palette[i] = desktop_palette[i & 15];
   for (i = 0; i < 256; i++)
      a4_palette_color[i] = i;
}

/* called by set_gfx_mode(): Allegro sets the default palette there */
void a4_bitmap_mode_set(void)
{
   int c;
   for (c = 0; c < 256; c++)
      if (a4_color_depth == 8)
         a4_palette_color[c] = c;
   set_palette(a4_default_palette);
   if (a4_color_depth == 8) {
      gui_fg_color = 255;
      gui_bg_color = 0;
   } else {
      gui_fg_color = makecol(0, 0, 0);
      gui_bg_color = makecol(255, 255, 255);
   }
}

/* ---------------------------------------------------------------- bitmaps */

static BITMAP *alloc_header(int height)
{
   int n = height < 2 ? 2 : height;
   return (BITMAP *)calloc(1, sizeof(BITMAP) + sizeof(unsigned char *) * (size_t)n);
}

BITMAP *create_bitmap_ex(int color_depth, int width, int height)
{
   BITMAP *b;
   int i, bpp;
   if (width < 0 || height <= 0)
      return NULL;
   if (color_depth != 8 && color_depth != 15 && color_depth != 16 &&
       color_depth != 24 && color_depth != 32)
      return NULL;
   b = alloc_header(height);
   if (!b)
      return NULL;
   bpp = a4_bpp_bytes(color_depth);
   /* Allegro leaves pixels uninitialised; zero them for determinism. */
   b->dat = calloc(1, (size_t)width * height * bpp + 4);
   if (!b->dat) {
      free(b);
      return NULL;
   }
   b->w = b->cr = width;
   b->h = b->cb = height;
   b->clip = TRUE;
   b->cl = b->ct = 0;
   b->depth = color_depth;
   b->pitch = width * bpp;
   b->serial = next_serial++;
   for (i = 0; i < height; i++)
      b->line[i] = (unsigned char *)b->dat + (size_t)i * b->pitch;
   return b;
}

BITMAP *create_bitmap(int width, int height)
{
   return create_bitmap_ex(a4_color_depth, width, height);
}

BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height)
{
   BITMAP *b, *root;
   int i, bpp;
   if (!parent || x < 0 || y < 0 || x >= parent->w || y >= parent->h || width <= 0 || height <= 0)
      return NULL;
   if (x + width > parent->w) width = parent->w - x;
   if (y + height > parent->h) height = parent->h - y;
   b = alloc_header(height);
   if (!b)
      return NULL;
   root = a4_root(parent);
   bpp = a4_bpp_bytes(parent->depth);
   b->w = b->cr = width;
   b->h = b->cb = height;
   b->clip = TRUE;
   b->cl = b->ct = 0;
   b->depth = parent->depth;
   b->pitch = parent->pitch;
   b->dat = NULL;
   b->parent = root;
   b->x_ofs = x + parent->x_ofs;
   b->y_ofs = y + parent->y_ofs;
   b->serial = next_serial++;
   for (i = 0; i < height; i++)
      b->line[i] = parent->line[y + i] + (size_t)x * bpp;
   return b;
}

void destroy_bitmap(BITMAP *b)
{
   if (!b)
      return;
   if (b == screen)
      return;   /* the screen belongs to set_gfx_mode() */
   free(b->dat);
   free(b);
}

int bitmap_color_depth(BITMAP *bmp) { return bmp ? bmp->depth : 0; }
int is_memory_bitmap(BITMAP *bmp) { (void)bmp; return TRUE; }
void acquire_bitmap(BITMAP *bmp) { (void)bmp; }
void release_bitmap(BITMAP *bmp) { (void)bmp; }
int bitmap_mask_color(BITMAP *bmp) { return (int)a4_mask_color(bmp->depth); }

void a4_touch(BITMAP *b)
{
   b->generation++;
   if (b->parent)
      b->parent->generation++;
}

void set_clip_rect(BITMAP *b, int x1, int y1, int x2, int y2)
{
   x2++;
   y2++;
   b->cl = MID(0, x1, b->w - 1);
   b->ct = MID(0, y1, b->h - 1);
   b->cr = MID(0, x2, b->w);
   b->cb = MID(0, y2, b->h);
}

void putpixel(BITMAP *bmp, int x, int y, int color)
{
   if (bmp->clip && (x < bmp->cl || x >= bmp->cr || y < bmp->ct || y >= bmp->cb))
      return;
   if (x < 0 || y < 0 || x >= bmp->w || y >= bmp->h)
      return;
   if (a4_draw_mode == DRAW_MODE_TRANS && bmp->depth != 8) {
      unsigned long d = a4_get_raw(bmp, x, y);
      a4_put_raw(bmp, x, y, a4_blend(bmp->depth, (unsigned long)color, d, (unsigned long)a4_blend_a));
   } else if (a4_draw_mode == DRAW_MODE_XOR) {
      a4_put_raw(bmp, x, y, a4_get_raw(bmp, x, y) ^ (unsigned long)color);
   } else {
      a4_put_raw(bmp, x, y, (unsigned long)color);
   }
   a4_touch(bmp);
}

int getpixel(BITMAP *bmp, int x, int y)
{
   if (x < 0 || y < 0 || x >= bmp->w || y >= bmp->h)
      return -1;
   return (int)a4_get_raw(bmp, x, y);
}

void clear_to_color(BITMAP *bmp, int color)
{
   int x, y;
   for (y = bmp->ct; y < bmp->cb; y++)
      for (x = bmp->cl; x < bmp->cr; x++)
         a4_put_raw(bmp, x, y, (unsigned long)color);
   a4_touch(bmp);
}

void clear_bitmap(BITMAP *bmp)
{
   clear_to_color(bmp, 0);
}

/* ---------------------------------------------------------------- colour */

void set_color_depth(int depth)
{
   a4_color_depth = depth;
}

int get_color_depth(void) { return a4_color_depth; }
void set_color_conversion(int mode) { a4_color_conversion = mode; }
int get_color_conversion(void) { return a4_color_conversion; }

static int col_diff[3 * 128];

static void bestfit_init(void)
{
   int i;
   for (i = 1; i < 64; i++) {
      int k = i * i;
      col_diff[0 + i] = col_diff[0 + 128 - i] = k * (59 * 59);
      col_diff[128 + i] = col_diff[128 + 128 - i] = k * (30 * 30);
      col_diff[256 + i] = col_diff[256 + 128 - i] = k * (11 * 11);
   }
}

static int bestfit_color(const RGB *pal, int r, int g, int b)
{
   int i, coldiff, lowest, bestfit;
   if (col_diff[1] == 0)
      bestfit_init();
   bestfit = 0;
   lowest = INT_MAX;
   i = ((r == 63) && (g == 0) && (b == 63)) ? 0 : 1;
   while (i < PAL_SIZE) {
      const RGB *rgb = &pal[i];
      coldiff = (col_diff + 0)[(rgb->g - g) & 0x7F];
      if (coldiff < lowest) {
         coldiff += (col_diff + 128)[(rgb->r - r) & 0x7F];
         if (coldiff < lowest) {
            coldiff += (col_diff + 256)[(rgb->b - b) & 0x7F];
            if (coldiff < lowest) {
               bestfit = (int)(rgb - pal);
               if (coldiff == 0)
                  return bestfit;
               lowest = coldiff;
            }
         }
      }
      i++;
   }
   return bestfit;
}

int makecol8(int r, int g, int b) { return bestfit_color(a4_current_palette, r >> 2, g >> 2, b >> 2); }
int makecol15(int r, int g, int b) { return ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3); }
int makecol16(int r, int g, int b) { return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3); }
int makecol24(int r, int g, int b) { return (r << 16) | (g << 8) | b; }
int makecol32(int r, int g, int b) { return (r << 16) | (g << 8) | b; }
int makeacol32(int r, int g, int b, int a) { return (int)(((unsigned)a << 24) | (r << 16) | (g << 8) | b); }

int makecol_depth(int depth, int r, int g, int b)
{
   switch (depth) {
      case 8: return makecol8(r, g, b);
      case 15: return makecol15(r, g, b);
      case 16: return makecol16(r, g, b);
      case 24: return makecol24(r, g, b);
      case 32: return makecol32(r, g, b);
   }
   return 0;
}

int makecol(int r, int g, int b) { return makecol_depth(a4_color_depth, r, g, b); }

int getr8(int c) { return _rgb_scale_6[a4_current_palette[c & 0xFF].r]; }
int getg8(int c) { return _rgb_scale_6[a4_current_palette[c & 0xFF].g]; }
int getb8(int c) { return _rgb_scale_6[a4_current_palette[c & 0xFF].b]; }
int getr15(int c) { return _rgb_scale_5[(c >> 10) & 0x1F]; }
int getg15(int c) { return _rgb_scale_5[(c >> 5) & 0x1F]; }
int getb15(int c) { return _rgb_scale_5[c & 0x1F]; }
int getr16(int c) { return _rgb_scale_5[(c >> 11) & 0x1F]; }
int getg16(int c) { return _rgb_scale_6[(c >> 5) & 0x3F]; }
int getb16(int c) { return _rgb_scale_5[c & 0x1F]; }
int getr24(int c) { return (c >> 16) & 0xFF; }
int getg24(int c) { return (c >> 8) & 0xFF; }
int getb24(int c) { return c & 0xFF; }
int getr32(int c) { return (c >> 16) & 0xFF; }
int getg32(int c) { return (c >> 8) & 0xFF; }
int getb32(int c) { return c & 0xFF; }
int geta32(int c) { return (int)(((unsigned)c >> 24) & 0xFF); }

int getr_depth(int depth, int c)
{
   switch (depth) {
      case 8: return getr8(c);
      case 15: return getr15(c);
      case 16: return getr16(c);
      case 24: return getr24(c);
      case 32: return getr32(c);
   }
   return 0;
}
int getg_depth(int depth, int c)
{
   switch (depth) {
      case 8: return getg8(c);
      case 15: return getg15(c);
      case 16: return getg16(c);
      case 24: return getg24(c);
      case 32: return getg32(c);
   }
   return 0;
}
int getb_depth(int depth, int c)
{
   switch (depth) {
      case 8: return getb8(c);
      case 15: return getb15(c);
      case 16: return getb16(c);
      case 24: return getb24(c);
      case 32: return getb32(c);
   }
   return 0;
}
int getr(int c) { return getr_depth(a4_color_depth, c); }
int getg(int c) { return getg_depth(a4_color_depth, c); }
int getb(int c) { return getb_depth(a4_color_depth, c); }

int a4_palette_color_depth(int depth, int index)
{
   const RGB *p = &a4_current_palette[index & 0xFF];
   if (depth == 8)
      return index & 0xFF;
   return makecol_depth(depth, _rgb_scale_6[p->r], _rgb_scale_6[p->g], _rgb_scale_6[p->b]);
}

unsigned long a4_convert_color(unsigned long c, int from, int to)
{
   if (from == to)
      return c;
   if (from == 8)
      return (unsigned long)a4_palette_color_depth(to, (int)c);
   return (unsigned long)makecol_depth(to, getr_depth(from, (int)c), getg_depth(from, (int)c),
                                       getb_depth(from, (int)c));
}

void set_palette(const RGB *p)
{
   int c;
   for (c = 0; c < PAL_SIZE; c++) {
      a4_current_palette[c] = p[c];
      if (a4_color_depth != 8)
         a4_palette_color[c] = makecol(_rgb_scale_6[p[c].r], _rgb_scale_6[p[c].g], _rgb_scale_6[p[c].b]);
   }
}

void get_palette(RGB *p)
{
   memcpy(p, a4_current_palette, sizeof(PALETTE));
}

void select_palette(const RGB *p)
{
   int c;
   for (c = 0; c < PAL_SIZE; c++) {
      prev_palette[c] = a4_current_palette[c];
      a4_current_palette[c] = p[c];
   }
   if (a4_color_depth != 8) {
      for (c = 0; c < PAL_SIZE; c++) {
         prev_palette_color[c] = a4_palette_color[c];
         a4_palette_color[c] = makecol(_rgb_scale_6[p[c].r], _rgb_scale_6[p[c].g], _rgb_scale_6[p[c].b]);
      }
   }
}

void unselect_palette(void)
{
   int c;
   for (c = 0; c < PAL_SIZE; c++)
      a4_current_palette[c] = prev_palette[c];
   if (a4_color_depth != 8)
      for (c = 0; c < PAL_SIZE; c++)
         a4_palette_color[c] = prev_palette_color[c];
}

void generate_332_palette(RGB *pal)
{
   int c;
   for (c = 0; c < PAL_SIZE; c++) {
      pal[c].r = ((c >> 5) & 7) * 63 / 7;
      pal[c].g = ((c >> 2) & 7) * 63 / 7;
      pal[c].b = (c & 3) * 63 / 3;
   }
   pal[0].r = 63;
   pal[0].g = 0;
   pal[0].b = 63;
   pal[254].r = pal[254].g = pal[254].b = 0;
}

/* _color_load_depth: the depth an image loader should create, honouring the
 * colour-conversion flags (Allegro graphics.c). */
int _color_load_depth(int depth, int hasalpha)
{
   typedef struct { int flag, in_depth, out_depth, hasalpha; } CONV;
   static const CONV conv_table[] = {
      { COLORCONV_8_TO_15,   8,  15, 0 }, { COLORCONV_8_TO_16,   8,  16, 0 },
      { COLORCONV_8_TO_24,   8,  24, 0 }, { COLORCONV_8_TO_32,   8,  32, 0 },
      { COLORCONV_15_TO_8,   15, 8,  0 }, { COLORCONV_15_TO_16,  15, 16, 0 },
      { COLORCONV_15_TO_24,  15, 24, 0 }, { COLORCONV_15_TO_32,  15, 32, 0 },
      { COLORCONV_16_TO_8,   16, 8,  0 }, { COLORCONV_16_TO_15,  16, 15, 0 },
      { COLORCONV_16_TO_24,  16, 24, 0 }, { COLORCONV_16_TO_32,  16, 32, 0 },
      { COLORCONV_24_TO_8,   24, 8,  0 }, { COLORCONV_24_TO_15,  24, 15, 0 },
      { COLORCONV_24_TO_16,  24, 16, 0 }, { COLORCONV_24_TO_32,  24, 32, 0 },
      { COLORCONV_32_TO_8,   32, 8,  0 }, { COLORCONV_32_TO_15,  32, 15, 0 },
      { COLORCONV_32_TO_16,  32, 16, 0 }, { COLORCONV_32_TO_24,  32, 24, 0 },
      { COLORCONV_32A_TO_8,  32, 8,  1 }, { COLORCONV_32A_TO_15, 32, 15, 1 },
      { COLORCONV_32A_TO_16, 32, 16, 1 }, { COLORCONV_32A_TO_24, 32, 24, 1 }
   };
   int i;
   if (depth == a4_color_depth)
      return depth;
   for (i = 0; i < (int)(sizeof(conv_table) / sizeof(conv_table[0])); i++) {
      if (conv_table[i].in_depth == depth && conv_table[i].out_depth == a4_color_depth &&
          ((conv_table[i].hasalpha != 0) == (hasalpha != 0))) {
         if (a4_color_conversion & conv_table[i].flag)
            return a4_color_depth;
         else
            return depth;
      }
   }
   return depth;
}

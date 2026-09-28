/*
 * Golden scenario: text output with the default font and the game fonts
 * (data.dat objects 50..54), as the game draws it: 32 bpp targets, colour
 * -1 and makecol() colours, transparent and solid backgrounds, left /
 * centre / right alignment, clipping.  C89 (built by GCC 4.4 too).
 */
#include <stdio.h>
#include <string.h>
#include "golden.h"

static const char *strings[] = {
   "Icy Tower 1.5.1",
   "0123456789 +-*/:",
   "\xA4",
   "Score: \xA4 100",
   "",
   "abc\x01\x7F~|",
   "\xC3\x85\xC3\x84\xC3\x96 \xE2\x82\xAC",      /* Latin-1 and Euro */
   "\xC3(\xFF\xFE end",                           /* invalid UTF-8 */
   "The Quick Brown Fox; JUMPS over 7 lazy dogs!"
};
#define NSTR ((int)(sizeof(strings) / sizeof(strings[0])))

static const char *rec_sub(const char *name)
{
   static char buf[160];
   sprintf(buf, "%s/sub", name);
   return buf;
}

static void render_block(golden_ctx *g, const char *name, FONT *f, int color, int bg)
{
   int th = text_height(f);
   int lh = th + 3;
   int w = 360, h = lh * NSTR * 3 + 4;
   int i, y = 2;
   BITMAP *b = create_bitmap_ex(32, w, h);
   clear_to_color(b, makecol(12, 34, 56));
   for (i = 0; i < NSTR; i++) {
      textout_ex(b, f, strings[i], 4, y, color, bg);
      y += lh;
      textout_centre_ex(b, f, strings[i], w / 2, y, color, bg);
      y += lh;
      textout_right_ex(b, f, strings[i], w - 4, y, color, bg);
      y += lh;
   }
   golden_bitmap(g, name, b);
   destroy_bitmap(b);
}

static void render_clipped(golden_ctx *g, const char *name, FONT *f, int color, int bg)
{
   BITMAP *b = create_bitmap_ex(32, 120, 40);
   BITMAP *sub;
   clear_to_color(b, makecol(0, 0, 0));
   textout_ex(b, f, "Clip Left", -7, -5, color, bg);
   textout_right_ex(b, f, "Clip Right", 125, 30, color, bg);
   textout_ex(b, f, "Mid", 50, 15, color, bg);
   set_clip_rect(b, 10, 5, 90, 25);
   textout_ex(b, f, "INSIDE rect", 3, 8, color, bg);
   textout_centre_ex(b, f, "\xA4\xA4\xA4", 60, 20, color, bg);
   set_clip_rect(b, 0, 0, 119, 39);
   golden_bitmap(g, name, b);
   destroy_bitmap(b);

   /* sub-bitmap target */
   b = create_bitmap_ex(32, 80, 30);
   clear_to_color(b, makecol(1, 2, 3));
   sub = create_sub_bitmap(b, 10, 5, 50, 20);
   textout_ex(sub, f, "Sub bitmap", -3, 2, color, bg);
   destroy_bitmap(sub);
   golden_bitmap(g, rec_sub(name), b);
   destroy_bitmap(b);
}

static void metrics(golden_ctx *g, const char *prefix, FONT *f)
{
   char name[128];
   int i;
   sprintf(name, "%s/height", prefix);
   golden_int(g, name, text_height(f));
   for (i = 0; i < NSTR; i++) {
      sprintf(name, "%s/len%d", prefix, i);
      golden_int(g, name, text_length(f, strings[i]));
   }
}

static void font_suite(golden_ctx *g, const char *tag, FONT *f)
{
   char name[128];
   static const int colors[3][3] = { { -1, -1, -1 }, { 255, 255, 255 }, { 255, 32, 0 } };
   int c, b;
   metrics(g, tag, f);
   for (c = 0; c < 3; c++) {
      int color = colors[c][0] < 0 ? -1 : makecol(colors[c][0], colors[c][1], colors[c][2]);
      for (b = 0; b < 2; b++) {
         int bg = b ? makecol(0, 0, 64) : -1;
         sprintf(name, "%s/block_c%d_b%d", tag, c, b);
         render_block(g, name, f, color, bg);
         sprintf(name, "%s/clip_c%d_b%d", tag, c, b);
         render_clipped(g, name, f, color, bg);
      }
   }
}

static void printf_suite(golden_ctx *g, const char *tag, FONT *f)
{
   char name[128];
   BITMAP *b = create_bitmap_ex(32, 300, 3 * (text_height(f) + 2) + 2);
   clear_to_color(b, makecol(40, 40, 40));
   textprintf_ex(b, f, 2, 1, -1, -1, "Floor %d  combo %03d", 123, 7);
   textprintf_centre_ex(b, f, 150, text_height(f) + 3, makecol(255, 255, 0), -1, "%s:%5d", "Score", 98765);
   textprintf_right_ex(b, f, 298, 2 * text_height(f) + 5, -1, makecol(90, 0, 0), "%c%c %x", 'O', 'K', 255);
   sprintf(name, "%s/printf", tag);
   golden_bitmap(g, name, b);
   destroy_bitmap(b);
}

static void depth_suite(golden_ctx *g, const char *tag, FONT *f)
{
   /* non-32-bit targets: palette expansion and colour glyph conversion */
   static const int depths[3] = { 15, 16, 24 };
   char name[128];
   int i;
   for (i = 0; i < 3; i++) {
      BITMAP *b = create_bitmap_ex(depths[i], 200, 2 * text_height(f) + 4);
      clear_to_color(b, makecol_depth(depths[i], 20, 40, 60));
      textout_ex(b, f, "Depth test \xA4", 2, 1, -1, -1);
      textout_ex(b, f, "Depth test 2", 2, text_height(f) + 2, makecol_depth(depths[i], 200, 100, 50),
                 makecol_depth(depths[i], 0, 0, 90));
      sprintf(name, "%s/depth%d", tag, depths[i]);
      golden_bitmap(g, name, b);
      destroy_bitmap(b);
   }
}

void scen_text(golden_ctx *g)
{
   DATAFILE *data;
   int i;
   char tag[32];

   set_color_depth(32);
   set_color_conversion(0x00ffffff);

   font_suite(g, "text/default", font);
   printf_suite(g, "text/default", font);
   depth_suite(g, "text/default", font);

   packfile_password("CHEESE");
   data = load_datafile(golden_path(g, "data/data.dat"));
   packfile_password(NULL);
   golden_int(g, "text/data_loaded", data != NULL);
   if (!data)
      return;

   /* as the game: palette entry 0 black, palette selected */
   ((RGB *)data[0].dat)[0].r = ((RGB *)data[0].dat)[0].g = ((RGB *)data[0].dat)[0].b = 0;
   set_palette((RGB *)data[0].dat);

   for (i = 50; i <= 54; i++) {
      sprintf(tag, "text/font%d", i);
      font_suite(g, tag, (FONT *)data[i].dat);
      printf_suite(g, tag, (FONT *)data[i].dat);
      depth_suite(g, tag, (FONT *)data[i].dat);
   }

   /* the colour fonts again with the default palette */
   set_palette(desktop_palette);
   for (i = 50; i <= 52; i++) {
      sprintf(tag, "text/font%d_desk", i);
      render_block(g, tag, (FONT *)data[i].dat, -1, -1);
   }

   unload_datafile(data);
}

#include <string.h>
#include "golden.h"

static void put32(FILE *f, long v)
{
   unsigned char b[4];
   b[0] = (unsigned char)(v & 0xFF); b[1] = (unsigned char)((v >> 8) & 0xFF);
   b[2] = (unsigned char)((v >> 16) & 0xFF); b[3] = (unsigned char)((v >> 24) & 0xFF);
   fwrite(b, 1, 4, f);
}

static void header(golden_ctx *g, const char *name, int kind)
{
   fwrite(name, 1, strlen(name) + 1, g->out);
   put32(g->out, kind);
}

void golden_bitmap(golden_ctx *g, const char *name, BITMAP *bmp)
{
   int y, bpp, depth;
   header(g, name, GOLDEN_BITMAP);
   if (!bmp) { put32(g->out, 0); put32(g->out, 0); put32(g->out, 0); put32(g->out, 0); return; }
   depth = bitmap_color_depth(bmp);
   bpp = depth == 8 ? 1 : depth <= 16 ? 2 : depth == 24 ? 3 : 4;
   put32(g->out, bmp->w); put32(g->out, bmp->h); put32(g->out, depth);
   put32(g->out, (long)bmp->w * bmp->h * bpp);
   for (y = 0; y < bmp->h; y++)
      fwrite(bmp->line[y], 1, (size_t)bmp->w * bpp, g->out);
}

void golden_bytes(golden_ctx *g, const char *name, const void *p, long n)
{
   header(g, name, GOLDEN_BYTES);
   put32(g->out, n);
   if (n > 0) fwrite(p, 1, (size_t)n, g->out);
}

void golden_int(golden_ctx *g, const char *name, long v)
{
   header(g, name, GOLDEN_INT);
   put32(g->out, v);
}

const char *golden_path(golden_ctx *g, const char *rel)
{
   static char buf[4][1024];
   static int n;
   char *b = buf[n++ & 3];
   strcpy(b, g->data_dir);
   strcat(b, "/");
   strcat(b, rel);
   return b;
}

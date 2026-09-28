/*
 * Texture cache (see texcache.h).
 */
#include <stdlib.h>
#include <string.h>
#include "port/render/texcache.h"
#include "port/render/render.h"
#include "a4_dl.h"

#define TC_SIZE 4096            /* open addressing, power of two */
#define TC_MAX_IDLE_FRAMES 600  /* evict textures unused for ~10 s */

typedef struct tc_entry {
   const void *key;             /* BITMAP* or A4_GLYPH* */
   uint32_t serial;
   uint32_t gen[TEXV_COUNT];
   uint32_t pal_ver[TEXV_COUNT];
   SDL_Texture *tex[TEXV_COUNT];
   uint64_t last_used;
   int is_glyph;
} tc_entry;

static SDL_Renderer *g_ren;
static tc_entry g_tab[TC_SIZE];
static int g_count;
static uint64_t g_frame;
static uint32_t g_pal_ver = 1;

static unsigned hash_ptr(const void *p)
{
   uintptr_t v = (uintptr_t)p;
   v ^= v >> 17;
   v *= 0x9E3779B1u;
   return (unsigned)(v ^ (v >> 15)) & (TC_SIZE - 1);
}

static void entry_free(tc_entry *e)
{
   int i;
   for (i = 0; i < TEXV_COUNT; i++)
      if (e->tex[i])
         SDL_DestroyTexture(e->tex[i]);
   memset(e, 0, sizeof(*e));
}

/* open addressing with backward-shift deletion */
static void remove_at(unsigned i)
{
   unsigned j = i;
   entry_free(&g_tab[i]);
   g_count--;
   for (;;) {
      unsigned k;
      j = (j + 1) & (TC_SIZE - 1);
      if (!g_tab[j].key)
         return;
      k = hash_ptr(g_tab[j].key);
      if ((j > i && (k <= i || k > j)) || (j < i && (k <= i && k > j))) {
         g_tab[i] = g_tab[j];
         memset(&g_tab[j], 0, sizeof(g_tab[j]));
         i = j;
      }
   }
}

static tc_entry *lookup(const void *key, int create)
{
   unsigned i = hash_ptr(key);
   while (g_tab[i].key) {
      if (g_tab[i].key == key)
         return &g_tab[i];
      i = (i + 1) & (TC_SIZE - 1);
   }
   if (!create)
      return NULL;
   if (g_count >= TC_SIZE * 3 / 4) {
      /* full: drop everything (rare; textures are re-uploaded on demand) */
      unsigned k;
      for (k = 0; k < TC_SIZE; k++)
         if (g_tab[k].key)
            entry_free(&g_tab[k]);
      g_count = 0;
      i = hash_ptr(key);
   }
   memset(&g_tab[i], 0, sizeof(g_tab[i]));
   g_tab[i].key = key;
   g_count++;
   return &g_tab[i];
}

static void evict_hook(BITMAP *b) { texcache_evict(b); }

void texcache_init(SDL_Renderer *r)
{
   g_ren = r;
   a4_dl_texture_evict = evict_hook;
}

void texcache_shutdown(void)
{
   unsigned i;
   for (i = 0; i < TC_SIZE; i++)
      if (g_tab[i].key)
         entry_free(&g_tab[i]);
   g_count = 0;
   g_ren = NULL;
}

void texcache_evict(BITMAP *b)
{
   unsigned i = hash_ptr(b);
   while (g_tab[i].key) {
      if (g_tab[i].key == b) {
         remove_at(i);
         return;
      }
      i = (i + 1) & (TC_SIZE - 1);
   }
}

void texcache_palette_changed(void) { g_pal_ver++; }

void texcache_filter(SDL_Texture *t)
{
   if (t)
      SDL_SetTextureScaleMode(t, (SDL_ScaleMode)render_scale_mode());
}

/* Fill the RGB of fully transparent pixels from opaque neighbours so linear
 * filtering does not bleed the mask colour (magenta) into sprite edges. */
static void bleed(uint32_t *px, int w, int h)
{
   int x, y;
   uint32_t *src = (uint32_t *)malloc((size_t)w * h * 4);
   if (!src)
      return;
   memcpy(src, px, (size_t)w * h * 4);
   for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++) {
         unsigned r = 0, g = 0, b = 0, n = 0;
         int dx, dy;
         if (src[(size_t)y * w + x] >> 24)
            continue;
         for (dy = -1; dy <= 1; dy++) {
            for (dx = -1; dx <= 1; dx++) {
               int xx = x + dx, yy = y + dy;
               uint32_t q;
               if (xx < 0 || yy < 0 || xx >= w || yy >= h)
                  continue;
               q = src[(size_t)yy * w + xx];
               if (!(q >> 24))
                  continue;
               r += (q >> 16) & 0xFF; g += (q >> 8) & 0xFF; b += q & 0xFF; n++;
            }
         }
         px[(size_t)y * w + x] = n ? (((r / n) << 16) | ((g / n) << 8) | (b / n)) : 0;
      }
   }
   free(src);
}

static SDL_Texture *upload(BITMAP *b, tex_variant v)
{
   int x, y, w = b->w, h = b->h;
   uint32_t *px;
   SDL_Texture *t;
   unsigned long mask = a4_mask_color(b->depth);
   if (w <= 0 || h <= 0)
      return NULL;
   px = (uint32_t *)malloc((size_t)w * h * 4);
   if (!px)
      return NULL;
   for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++) {
         unsigned long c = a4_get_raw(b, x, y);
         uint32_t rgb, a = 255;
         if (b->depth == 32)
            rgb = (uint32_t)c & 0xFFFFFFu;
         else if (b->depth == 8)
            rgb = (uint32_t)a4_palette_color_depth(32, (int)c) & 0xFFFFFFu;
         else
            rgb = (uint32_t)a4_convert_color(c, b->depth, 32) & 0xFFFFFFu;
         switch (v) {
            case TEXV_MASKED:
               if (c == mask) a = 0;
               break;
            case TEXV_ALPHA:
               a = b->depth == 32 ? ((uint32_t)c >> 24) & 0xFF : (c == mask ? 0 : 255);
               break;
            case TEXV_SILHOUETTE:
               a = (c == mask) ? 0 : 255;
               rgb = 0xFFFFFFu;
               break;
            default:
               break;
         }
         px[(size_t)y * w + x] = (a << 24) | rgb;
      }
   }
   if (v == TEXV_MASKED || v == TEXV_ALPHA)
      bleed(px, w, h);
   t = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
   if (t) {
      SDL_UpdateTexture(t, NULL, px, w * 4);
      SDL_SetTextureBlendMode(t, v == TEXV_OPAQUE ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
   }
   free(px);
   return t;
}

SDL_Texture *texcache_get(BITMAP *b, tex_variant v)
{
   tc_entry *e;
   if (!g_ren || !b)
      return NULL;
   e = lookup(b, 1);
   if (e->serial != b->serial) {
      int i;
      for (i = 0; i < TEXV_COUNT; i++) {
         if (e->tex[i]) SDL_DestroyTexture(e->tex[i]);
         e->tex[i] = NULL;
      }
      e->serial = b->serial;
   }
   if (!e->tex[v] || e->gen[v] != b->generation || (b->depth == 8 && e->pal_ver[v] != g_pal_ver)) {
      if (e->tex[v])
         SDL_DestroyTexture(e->tex[v]);
      e->tex[v] = upload(b, v);
      e->gen[v] = b->generation;
      e->pal_ver[v] = g_pal_ver;
   }
   e->last_used = g_frame;
   return e->tex[v];
}

SDL_Texture *texcache_glyph(const A4_GLYPH *g)
{
   tc_entry *e;
   if (!g_ren || !g || !g->mono || g->w <= 0 || g->h <= 0)
      return NULL;
   e = lookup(g, 1);
   e->is_glyph = 1;
   if (!e->tex[0]) {
      int x, y, stride = (g->w + 7) / 8;
      uint32_t *px = (uint32_t *)calloc((size_t)g->w * g->h, 4);
      if (!px)
         return NULL;
      for (y = 0; y < g->h; y++)
         for (x = 0; x < g->w; x++)
            if (g->mono[y * stride + (x >> 3)] & (0x80 >> (x & 7)))
               px[(size_t)y * g->w + x] = 0xFFFFFFFFu;
      e->tex[0] = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, g->w, g->h);
      if (e->tex[0]) {
         SDL_UpdateTexture(e->tex[0], NULL, px, g->w * 4);
         SDL_SetTextureBlendMode(e->tex[0], SDL_BLENDMODE_BLEND);
      }
      free(px);
   }
   e->last_used = g_frame;
   return e->tex[0];
}

void texcache_frame_end(void)
{
   unsigned i;
   g_frame++;
   if (g_frame % 120)
      return;
   for (i = 0; i < TC_SIZE; i++) {
      if (g_tab[i].key && g_frame - g_tab[i].last_used > TC_MAX_IDLE_FRAMES && !g_tab[i].is_glyph) {
         remove_at(i);
         i--;   /* re-examine the slot that received a shifted entry */
      }
   }
}

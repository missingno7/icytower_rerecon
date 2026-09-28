/*
 * Datafile reader, datafile object registry, image file type registry.
 *
 * Ported from Allegro 4.4.1 (giftware licence): datafile.c (object readers,
 * properties, nested files, old V1 format), dataregi.c (type registry),
 * readbmp.c (register_bitmap_file_type/load_bitmap/save_bitmap,
 * _fixup_loaded_bitmap) and blit.c (_blit_between_formats colour
 * conversion, used for load-time depth conversion).
 *
 * Object types: FILE (nested), BMP (all depths, rgba, colour conversion
 * through _color_load_depth), FONT (old fixed/proportional and the 4.x
 * mono/colour range formats), SAMP (create_sample), MIDI (raw chunk bytes:
 * playback is not supported), everything else (PAL, DATA, OGG, unknown)
 * as raw bytes.  RLE and compiled sprites are not supported (raw bytes).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "a4_internal.h"
#include "a4_dl.h"

static int ascii_stricmp(const char *a, const char *b)
{
   for (;; a++, b++) {
      int ca = (unsigned char)*a, cb = (unsigned char)*b;
      if (ca >= 'A' && ca <= 'Z') ca += 32;
      if (cb >= 'A' && cb <= 'Z') cb += 32;
      if (ca != cb || !ca)
         return ca - cb;
   }
}

#define MAX_DATAFILE_TYPES   32
#define V1_DAT_MAGIC         0x616C6C2EL
#define V1_DAT_DATA          0
#define V1_DAT_FONT          1
#define V1_DAT_BITMAP_16     2
#define V1_DAT_BITMAP_256    3
#define V1_DAT_SPRITE_16     4
#define V1_DAT_SPRITE_256    5
#define V1_DAT_PALETTE_16    6
#define V1_DAT_PALETTE_256   7
#define V1_DAT_FONT_8x8      8
#define V1_DAT_FONT_PROP     9
#define V1_DAT_BITMAP        10
#define V1_DAT_PALETTE       11
#define V1_DAT_SAMPLE        12
#define V1_DAT_MIDI          13
#define V1_DAT_RLE_SPRITE    14
#define V1_DAT_FLI           15
#define V1_DAT_C_SPRITE      16
#define V1_DAT_XC_SPRITE     17
#define OLD_FONT_SIZE        95
#define LESS_OLD_FONT_SIZE   224
#define MIDI_TRACKS          32

static void set_errno(int e) { if (allegro_errno) *allegro_errno = e; }

/* ================================================================== */
/* colour conversion (blit.c _blit_between_formats)                   */
/* ================================================================== */

/* get_replacement_mask_color (blit.c) */
static int replacement_mask_color(int depth)
{
   int c, g = 0;
   if (depth == 8)
      return makecol8(255, 4, 255);   /* bestfit_color(pal, 63, 1, 63) */
   do
      c = makecol_depth(depth, 255, ++g, 255);
   while (c == (int)a4_mask_color(depth));
   return c;
}

void a4_convert_blit(BITMAP *src, BITMAP *dst, int conv)
{
   int x, y, w, h;
   int sd = src->depth, dd = dst->depth;
   int keep = (conv & COLORCONV_KEEP_TRANS) != 0;

   w = MIN(src->w, dst->w);
   h = MIN(src->h, dst->h);

   if (sd == dd) {
      for (y = 0; y < h; y++)
         memcpy(dst->line[y], src->line[y], (size_t)w * (size_t)a4_bpp_bytes(sd));
      return;
   }

   if (sd == 8) {
      /* blit_from_256: palette expansion table of the current palette */
      int table[256], c;
      for (c = 0; c < 256; c++)
         table[c] = a4_palette_color_depth(dd, c);
      if (keep) {
         int rc = replacement_mask_color(dd);
         int mask = (int)a4_mask_color(dd);
         table[MASK_COLOR_8] = mask;
         for (c = 0; c < 256; c++)
            if ((c != MASK_COLOR_8) && (table[c] == mask))
               table[c] = rc;
      }
      for (y = 0; y < h; y++)
         for (x = 0; x < w; x++)
            a4_put_raw(dst, x, y, (unsigned long)table[src->line[y][x]]);
      return;
   }

   /* CONVERT_BLIT (dithering flags are not implemented) */
   {
      int rc = keep ? replacement_mask_color(dd) : 0;
      int src_mask = (int)a4_mask_color(sd);
      int dest_mask = (int)a4_mask_color(dd);
      for (y = 0; y < h; y++) {
         for (x = 0; x < w; x++) {
            int c = (int)a4_get_raw(src, x, y);
            if (keep && c == src_mask) {
               c = dest_mask;
            } else {
               c = makecol_depth(dd, getr_depth(sd, c), getg_depth(sd, c), getb_depth(sd, c));
               if (keep && c == dest_mask)
                  c = rc;
            }
            a4_put_raw(dst, x, y, (unsigned long)c);
         }
      }
   }
}

/* ================================================================== */
/* object readers (datafile.c)                                        */
/* ================================================================== */

static void *read_block(PACKFILE *f, int size, int alloc_size)
{
   void *p;
   int n = MAX(size, alloc_size);
   if (n < 0) {
      set_errno(ENOMEM);
      return NULL;
   }
   p = malloc(n > 0 ? (size_t)n : 1);
   if (!p) {
      set_errno(ENOMEM);
      return NULL;
   }
   pack_fread(p, size > 0 ? size : 0, f);
   if (pack_ferror(f)) {
      free(p);
      return NULL;
   }
   return p;
}

/* load_st_data: old 4 bpp Atari ST planar bitmaps */
static void load_st_data(unsigned char *pos, long size, PACKFILE *f)
{
   int c;
   unsigned int d1, d2, d3, d4;
   size /= 8;
   while (size) {
      d1 = (unsigned int)pack_mgetw(f);
      d2 = (unsigned int)pack_mgetw(f);
      d3 = (unsigned int)pack_mgetw(f);
      d4 = (unsigned int)pack_mgetw(f);
      for (c = 0; c < 16; c++) {
         *(pos++) = (unsigned char)(((d1 & 0x8000) >> 15) + ((d2 & 0x8000) >> 14) +
                                    ((d3 & 0x8000) >> 13) + ((d4 & 0x8000) >> 12));
         d1 <<= 1;
         d2 <<= 1;
         d3 <<= 1;
         d4 <<= 1;
      }
      size--;
   }
}

static BITMAP *read_bitmap(PACKFILE *f, int bits, int allowconv)
{
   int x, y, w, h, c, r, g, b, a;
   int destbits, rgba;
   BITMAP *bmp;

   if (bits < 0) {
      bits = -bits;
      rgba = TRUE;
   } else {
      rgba = FALSE;
   }

   if (allowconv)
      destbits = _color_load_depth(bits, rgba);
   else
      destbits = 8;

   w = pack_mgetw(f);
   h = pack_mgetw(f);

   bmp = create_bitmap_ex(MAX(bits, 8), w, h);
   if (!bmp) {
      set_errno(ENOMEM);
      return NULL;
   }

   switch (bits) {
      case 4:
         load_st_data((unsigned char *)bmp->dat, (long)w * h / 2, f);
         break;

      case 8:
         pack_fread(bmp->dat, (long)w * h, f);
         break;

      case 15:
         for (y = 0; y < h; y++) {
            uint16_t *p16 = (uint16_t *)bmp->line[y];
            for (x = 0; x < w; x++) {
               c = pack_igetw(f);
               r = _rgb_scale_5[(c >> 11) & 0x1F];   /* stored as 16 bit */
               g = _rgb_scale_6[(c >> 5) & 0x3F];
               b = _rgb_scale_5[c & 0x1F];
               p16[x] = (uint16_t)makecol15(r, g, b);
            }
         }
         break;

      case 16:
         for (y = 0; y < h; y++) {
            uint16_t *p16 = (uint16_t *)bmp->line[y];
            for (x = 0; x < w; x++) {
               c = pack_igetw(f);
               r = _rgb_scale_5[(c >> 11) & 0x1F];
               g = _rgb_scale_6[(c >> 5) & 0x3F];
               b = _rgb_scale_5[c & 0x1F];
               p16[x] = (uint16_t)makecol16(r, g, b);
            }
         }
         break;

      case 24:
         for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++) {
               r = pack_getc(f);
               g = pack_getc(f);
               b = pack_getc(f);
               if (rgba)
                  pack_getc(f);
               c = makecol24(r, g, b);
               a4_put_raw(bmp, x, y, (unsigned long)(c & 0xFFFFFF));
            }
         }
         break;

      case 32:
         for (y = 0; y < h; y++) {
            uint32_t *p32 = (uint32_t *)bmp->line[y];
            for (x = 0; x < w; x++) {
               r = pack_getc(f);
               g = pack_getc(f);
               b = pack_getc(f);
               if (rgba)
                  a = pack_getc(f);
               else
                  a = 0;
               p32[x] = (uint32_t)makeacol32(r, g, b, a);
            }
         }
         break;
   }

   if (bits != destbits) {
      BITMAP *tmp = bmp;
      bmp = create_bitmap_ex(destbits, w, h);
      if (!bmp) {
         destroy_bitmap(tmp);
         set_errno(ENOMEM);
         return NULL;
      }
      a4_convert_blit(tmp, bmp, a4_color_conversion);
      destroy_bitmap(tmp);
   }
   return bmp;
}

BITMAP *a4_read_bitmap_object(PACKFILE *f, long size)
{
   short bits = (short)pack_mgetw(f);
   (void)size;
   return read_bitmap(f, bits, TRUE);
}

/* ---- fonts --------------------------------------------------------- */

static FONT *read_font_fixed(PACKFILE *pack, int height, int maxchars)
{
   FONT *f;
   A4_FONT_RANGE *rg;
   int i;

   f = a4_font_create(height, 0, 1);
   if (!f) {
      set_errno(ENOMEM);
      return NULL;
   }
   rg = &f->ranges[0];
   rg->begin = ' ';
   rg->end = ' ' + maxchars;
   rg->glyphs = (A4_GLYPH *)calloc((size_t)maxchars, sizeof(A4_GLYPH));
   if (!rg->glyphs) {
      destroy_font(f);
      set_errno(ENOMEM);
      return NULL;
   }
   for (i = 0; i < maxchars; i++) {
      A4_GLYPH *g = &rg->glyphs[i];
      g->mono = (unsigned char *)malloc(height > 0 ? (size_t)height : 1);
      if (!g->mono) {
         destroy_font(f);
         set_errno(ENOMEM);
         return NULL;
      }
      g->w = 8;
      g->h = height;
      pack_fread(g->mono, height, pack);
   }
   return f;
}

static FONT *read_font_prop(PACKFILE *pack, int maxchars)
{
   FONT *f;
   A4_FONT_RANGE *rg;
   int i = 0, h = 0;

   f = a4_font_create(0, 1, 1);
   if (!f) {
      set_errno(ENOMEM);
      return NULL;
   }
   rg = &f->ranges[0];
   rg->begin = ' ';
   rg->end = ' ' + maxchars;
   rg->glyphs = (A4_GLYPH *)calloc((size_t)maxchars, sizeof(A4_GLYPH));
   if (!rg->glyphs) {
      destroy_font(f);
      set_errno(ENOMEM);
      return NULL;
   }
   for (i = 0; i < maxchars; i++) {
      BITMAP *b;
      if (pack_feof(pack))
         break;
      b = read_bitmap(pack, 8, FALSE);
      if (!b) {
         destroy_font(f);
         return NULL;
      }
      rg->glyphs[i].bmp = b;
      rg->glyphs[i].w = b->w;
      rg->glyphs[i].h = b->h;
      if (b->h > h)
         h = b->h;
   }
   while (i < maxchars) {
      BITMAP *b = create_bitmap_ex(8, 8, h);
      if (!b) {
         destroy_font(f);
         set_errno(ENOMEM);
         return NULL;
      }
      clear_bitmap(b);
      rg->glyphs[i].bmp = b;
      rg->glyphs[i].w = b->w;
      rg->glyphs[i].h = b->h;
      i++;
   }
   f->height = h;
   return f;
}

static int read_font_mono(PACKFILE *f, A4_FONT_RANGE *rg, int *hmax)
{
   int max, i;
   rg->begin = (int)pack_mgetl(f);
   rg->end = (int)pack_mgetl(f) + 1;
   max = rg->end - rg->begin;
   if (max < 0) {
      set_errno(ENOMEM);
      return -1;
   }
   rg->glyphs = (A4_GLYPH *)calloc(max > 0 ? (size_t)max : 1, sizeof(A4_GLYPH));
   if (!rg->glyphs) {
      set_errno(ENOMEM);
      return -1;
   }
   for (i = 0; i < max; i++) {
      int w, h, sz;
      w = pack_mgetw(f);
      h = pack_mgetw(f);
      sz = ((w + 7) / 8) * h;
      if (h > *hmax)
         *hmax = h;
      rg->glyphs[i].mono = (unsigned char *)malloc(sz > 0 ? (size_t)sz : 1);
      if (!rg->glyphs[i].mono) {
         set_errno(ENOMEM);
         return -1;
      }
      rg->glyphs[i].w = w;
      rg->glyphs[i].h = h;
      pack_fread(rg->glyphs[i].mono, sz > 0 ? sz : 0, f);
   }
   return 0;
}

static int read_font_color(PACKFILE *pack, A4_FONT_RANGE *rg, int *hmax, int depth)
{
   int max, i;
   rg->begin = (int)pack_mgetl(pack);
   rg->end = (int)pack_mgetl(pack) + 1;
   max = rg->end - rg->begin;
   if (max < 0) {
      set_errno(ENOMEM);
      return -1;
   }
   rg->glyphs = (A4_GLYPH *)calloc(max > 0 ? (size_t)max : 1, sizeof(A4_GLYPH));
   if (!rg->glyphs) {
      set_errno(ENOMEM);
      return -1;
   }
   for (i = 0; i < max; i++) {
      /* no colour conversion for 8 bit glyphs, required for all others */
      BITMAP *b = read_bitmap(pack, depth, depth != 8);
      if (!b) {
         set_errno(ENOMEM);
         return -1;
      }
      rg->glyphs[i].bmp = b;
      rg->glyphs[i].w = b->w;
      rg->glyphs[i].h = b->h;
      if (b->h > *hmax)
         *hmax = b->h;
   }
   return 0;
}

/* read_font: new style (4.x) range based font */
static FONT *read_font(PACKFILE *pack)
{
   FONT *f;
   int num_ranges, height = 0, depth, r;

   num_ranges = pack_mgetw(pack);
   if (num_ranges < 0)
      num_ranges = 0;
   f = a4_font_create(0, 0, num_ranges);
   if (!f) {
      set_errno(ENOMEM);
      return NULL;
   }
   for (r = 0; r < num_ranges; r++) {
      depth = pack_getc(pack);
      if (depth == 1 || depth == 255) {
         f->is_color = 0;
         if (read_font_mono(pack, &f->ranges[r], &height) != 0) {
            destroy_font(f);
            return NULL;
         }
      } else {
         /* older versions of Allegro use 0 for colour fonts */
         if (depth == 0)
            depth = 8;
         f->is_color = 1;
         if (read_font_color(pack, &f->ranges[r], &height, depth) != 0) {
            destroy_font(f);
            return NULL;
         }
      }
   }
   f->height = height;
   return f;
}

static void *load_font_object(PACKFILE *f, long size)
{
   short height = (short)pack_mgetw(f);
   (void)size;
   if (height > 0)
      return read_font_fixed(f, height, LESS_OLD_FONT_SIZE);
   else if (height < 0)
      return read_font_prop(f, LESS_OLD_FONT_SIZE);
   else
      return read_font(f);
}

static RGB *read_palette(PACKFILE *f, int size)
{
   RGB *p;
   int c, x;
   p = (RGB *)malloc(sizeof(PALETTE));
   if (!p) {
      set_errno(ENOMEM);
      return NULL;
   }
   for (c = 0; c < size; c++) {
      p[c].r = (unsigned char)(pack_getc(f) >> 2);
      p[c].g = (unsigned char)(pack_getc(f) >> 2);
      p[c].b = (unsigned char)(pack_getc(f) >> 2);
      p[c].filler = 0;
   }
   x = 0;
   while (c < PAL_SIZE) {
      p[c] = p[x];
      c++;
      x++;
      if (x >= size)
         x = 0;
   }
   return p;
}

static SAMPLE *read_sample(PACKFILE *f)
{
   signed short bits;
   int stereo, freq;
   long len;
   SAMPLE *s;

   bits = (signed short)pack_mgetw(f);
   if (bits < 0) {
      bits = (signed short)-bits;
      stereo = TRUE;
   } else {
      stereo = FALSE;
   }
   freq = pack_mgetw(f);
   len = pack_mgetl(f);
   if (len < 0) {
      set_errno(ENOMEM);
      return NULL;
   }
   s = create_sample(bits, stereo, freq, (int)len);
   if (!s || !s->data) {
      if (s)
         destroy_sample(s);
      set_errno(ENOMEM);
      return NULL;
   }
   s->priority = 128;
   s->loop_start = 0;
   s->loop_end = (unsigned long)len;
   s->param = 0;

   if (bits == 8) {
      pack_fread(s->data, len * (stereo ? 2 : 1), f);
   } else {
      long i, n = len * (stereo ? 2 : 1);
      for (i = 0; i < n; i++)
         ((unsigned short *)s->data)[i] = (unsigned short)pack_igetw(f);
   }
   if (pack_ferror(f)) {
      destroy_sample(s);
      return NULL;
   }
   return s;
}

static void *load_sample_object(PACKFILE *f, long size)
{
   (void)size;
   return read_sample(f);
}

static void destroy_sample_object(void *p) { destroy_sample((SAMPLE *)p); }

/* MIDI: the chunk (divisions + 32 tracks) is kept as raw bytes */
static void *load_midi_object(PACKFILE *f, long size)
{
   return read_block(f, (int)size, 0);
}

static void *load_bitmap_object(PACKFILE *f, long size)
{
   return a4_read_bitmap_object(f, size);
}

static void destroy_bitmap_object(void *p) { destroy_bitmap((BITMAP *)p); }
static void destroy_font_object(void *p) { destroy_font((FONT *)p); }

/* ================================================================== */
/* type registry (dataregi.c)                                         */
/* ================================================================== */

typedef struct DATAFILE_TYPE {
   int type;
   void *(*load)(PACKFILE *f, long size);
   void (*destroy)(void *data);
} DATAFILE_TYPE;

static DATAFILE_TYPE datafile_type[MAX_DATAFILE_TYPES];
static int types_initialised;
static void (*datafile_callback)(DATAFILE *);

static void *load_file_object(PACKFILE *f, long size);
static void unload_file_object(void *p) { unload_datafile((DATAFILE *)p); }

static void register_type(int id, void *(*load)(PACKFILE *f, long size), void (*destroy)(void *data))
{
   int i;
   for (i = 0; i < MAX_DATAFILE_TYPES; i++) {
      if (datafile_type[i].type == id) {
         if (load)
            datafile_type[i].load = load;
         if (destroy)
            datafile_type[i].destroy = destroy;
         return;
      }
   }
   for (i = 0; i < MAX_DATAFILE_TYPES; i++) {
      if (datafile_type[i].type == DAT_END) {
         datafile_type[i].type = id;
         datafile_type[i].load = load;
         datafile_type[i].destroy = destroy;
         return;
      }
   }
}

/* _initialize_datafile_types (datafile.c) */
static void init_types(void)
{
   int i;
   if (types_initialised)
      return;
   types_initialised = 1;
   for (i = 0; i < MAX_DATAFILE_TYPES; i++)
      datafile_type[i].type = DAT_END;
   register_type(DAT_FILE, load_file_object, unload_file_object);
   register_type(DAT_FONT, load_font_object, destroy_font_object);
   register_type(DAT_SAMPLE, load_sample_object, destroy_sample_object);
   register_type(DAT_MIDI, load_midi_object, NULL);
   register_type(DAT_BITMAP, load_bitmap_object, destroy_bitmap_object);
}

void register_datafile_object(int id, void *(*load)(PACKFILE *f, long size),
                              void (*destroy)(void *data))
{
   init_types();
   register_type(id, load, destroy);
}

/* ================================================================== */
/* loading (datafile.c)                                               */
/* ================================================================== */

static int load_object(DATAFILE *obj, PACKFILE *f, int type)
{
   PACKFILE *ff;
   long d;
   int i;

   ff = a4_pack_fopen_chunk(f, FALSE);
   if (!ff)
      return -1;
   d = a4_pack_todo(ff);
   obj->dat = NULL;
   for (i = 0; i < MAX_DATAFILE_TYPES; i++) {
      if (datafile_type[i].type == type) {
         obj->dat = datafile_type[i].load ? datafile_type[i].load(ff, d) : NULL;
         goto Found;
      }
   }
   obj->dat = read_block(ff, (int)d, 0);
 Found:
   a4_pack_fclose_chunk(ff);
   if (!obj->dat)
      return -1;
   obj->type = type;
   obj->size = d;
   return 0;
}

static int load_property(DATAFILE_PROPERTY *prop, PACKFILE *f)
{
   int type, size;
   type = (int)pack_mgetl(f);
   size = (int)pack_mgetl(f);
   prop->type = type;
   prop->dat = (char *)malloc(size >= 0 ? (size_t)size + 1 : 1);
   if (!prop->dat) {
      set_errno(ENOMEM);
      pack_fseek(f, size);
      return -1;
   }
   pack_fread(prop->dat, size > 0 ? size : 0, f);
   prop->dat[size > 0 ? size : 0] = 0;
   return 0;
}

static int add_property(DATAFILE_PROPERTY **list, DATAFILE_PROPERTY *prop)
{
   DATAFILE_PROPERTY *iter, *nl;
   int length = 0;
   if (*list) {
      iter = *list;
      while (iter->type != DAT_END) {
         length++;
         iter++;
      }
   }
   nl = (DATAFILE_PROPERTY *)realloc(*list, sizeof(DATAFILE_PROPERTY) * (size_t)(length + 2));
   if (!nl) {
      set_errno(ENOMEM);
      return -1;
   }
   *list = nl;
   (*list)[length] = *prop;
   (*list)[length + 1].type = DAT_END;
   (*list)[length + 1].dat = NULL;
   return 0;
}

static void destroy_property_list(DATAFILE_PROPERTY *list)
{
   int c;
   for (c = 0; list[c].type != DAT_END; c++)
      free(list[c].dat);
   free(list);
}

static void *load_file_object(PACKFILE *f, long size)
{
   DATAFILE *dat;
   DATAFILE_PROPERTY prop, *list;
   int count, c, type, failed;
   (void)size;

   count = (int)pack_mgetl(f);
   if (count < 0) {
      set_errno(ENOMEM);
      return NULL;
   }
   dat = (DATAFILE *)calloc((size_t)count + 1, sizeof(DATAFILE));
   if (!dat) {
      set_errno(ENOMEM);
      return NULL;
   }
   list = NULL;
   failed = FALSE;

   for (c = 0; c < count;) {
      type = (int)pack_mgetl(f);
      if (type == DAT_PROPERTY) {
         if (load_property(&prop, f) != 0) {
            failed = TRUE;
            break;
         }
         if (add_property(&list, &prop) != 0) {
            free(prop.dat);
            failed = TRUE;
            break;
         }
      } else {
         if (load_object(&dat[c], f, type) != 0) {
            failed = TRUE;
            break;
         }
         dat[c].prop = list;
         list = NULL;
         if (datafile_callback)
            datafile_callback(dat + c);
         c++;
      }
   }

   dat[c].type = DAT_END;
   dat[c].dat = NULL;
   dat[c].size = 0;
   dat[c].prop = NULL;

   if (list)
      destroy_property_list(list);

   if (failed) {
      unload_datafile(dat);
      dat = NULL;
   }
   return dat;
}

/* read_old_datafile: Allegro 1.x/2.x datafiles */
static DATAFILE *read_old_datafile(PACKFILE *f, void (*callback)(DATAFILE *))
{
   DATAFILE *dat;
   int size, type, c;

   size = pack_mgetw(f);
   if (size == EOF)
      return NULL;
   dat = (DATAFILE *)calloc((size_t)size + 1, sizeof(DATAFILE));
   if (!dat) {
      set_errno(ENOMEM);
      return NULL;
   }
   for (c = 0; c <= size; c++)
      dat[c].type = DAT_END;

   set_errno(0);

   for (c = 0; c < size; c++) {
      type = pack_mgetw(f);
      switch (type) {
         case V1_DAT_FONT:
         case V1_DAT_FONT_8x8:
            dat[c].type = DAT_FONT;
            dat[c].dat = read_font_fixed(f, 8, OLD_FONT_SIZE);
            dat[c].size = 0;
            break;
         case V1_DAT_FONT_PROP:
            dat[c].type = DAT_FONT;
            dat[c].dat = read_font_prop(f, OLD_FONT_SIZE);
            dat[c].size = 0;
            break;
         case V1_DAT_BITMAP:
         case V1_DAT_BITMAP_256:
            dat[c].type = DAT_BITMAP;
            dat[c].dat = read_bitmap(f, 8, TRUE);
            dat[c].size = 0;
            break;
         case V1_DAT_BITMAP_16:
            dat[c].type = DAT_BITMAP;
            dat[c].dat = read_bitmap(f, 4, FALSE);
            dat[c].size = 0;
            break;
         case V1_DAT_SPRITE_256:
            dat[c].type = DAT_BITMAP;
            pack_mgetw(f);
            dat[c].dat = read_bitmap(f, 8, TRUE);
            dat[c].size = 0;
            break;
         case V1_DAT_SPRITE_16:
            dat[c].type = DAT_BITMAP;
            pack_mgetw(f);
            dat[c].dat = read_bitmap(f, 4, FALSE);
            dat[c].size = 0;
            break;
         case V1_DAT_PALETTE:
         case V1_DAT_PALETTE_256:
            dat[c].type = DAT_PALETTE;
            dat[c].dat = read_palette(f, PAL_SIZE);
            dat[c].size = sizeof(PALETTE);
            break;
         case V1_DAT_PALETTE_16:
            dat[c].type = DAT_PALETTE;
            dat[c].dat = read_palette(f, 16);
            dat[c].size = 0;
            break;
         case V1_DAT_SAMPLE:
            dat[c].type = DAT_SAMPLE;
            dat[c].dat = read_sample(f);
            dat[c].size = 0;
            break;
         case V1_DAT_MIDI: {
            /* raw: divisions + MIDI_TRACKS x (length, data) */
            int t;
            PACKFILE *mf = f;
            unsigned char *buf = NULL;
            long n = 2, cap = 256;
            buf = (unsigned char *)malloc((size_t)cap);
            if (buf) {
               int div = pack_mgetw(mf);
               buf[0] = (unsigned char)(div >> 8);
               buf[1] = (unsigned char)div;
               for (t = 0; t < MIDI_TRACKS && buf; t++) {
                  long len = pack_mgetl(mf);
                  if (len < 0)
                     len = 0;
                  if (n + 4 + len > cap) {
                     unsigned char *nb;
                     cap = (n + 4 + len) * 2;
                     nb = (unsigned char *)realloc(buf, (size_t)cap);
                     if (!nb) {
                        free(buf);
                        buf = NULL;
                        break;
                     }
                     buf = nb;
                  }
                  buf[n] = (unsigned char)(len >> 24);
                  buf[n + 1] = (unsigned char)(len >> 16);
                  buf[n + 2] = (unsigned char)(len >> 8);
                  buf[n + 3] = (unsigned char)len;
                  n += 4;
                  pack_fread(buf + n, len, mf);
                  n += len;
               }
            }
            if (!buf)
               set_errno(ENOMEM);
            dat[c].type = DAT_MIDI;
            dat[c].dat = buf;
            dat[c].size = 0;
            break;
         }
         case V1_DAT_FLI:
            dat[c].type = DAT_FLI;
            dat[c].size = pack_mgetl(f);
            dat[c].dat = read_block(f, (int)dat[c].size, 0);
            break;
         case V1_DAT_RLE_SPRITE:
         case V1_DAT_C_SPRITE:
         case V1_DAT_XC_SPRITE:
            /* sprite formats are not supported by this layer */
            set_errno(ENOSYS);
            break;
         default:
            dat[c].type = DAT_DATA;
            dat[c].size = pack_mgetl(f);
            dat[c].dat = read_block(f, (int)dat[c].size, 0);
            break;
      }

      if (allegro_errno && *allegro_errno) {
         if (!dat[c].dat)
            dat[c].type = DAT_END;
         unload_datafile(dat);
         return NULL;
      }
      if (callback)
         callback(dat + c);
   }
   return dat;
}

static DATAFILE *load_datafile_callback_impl(const char *filename, void (*callback)(DATAFILE *));

/* Loaded assets are static: their pixels are written directly, so they must
 * never be mistaken for recording canvases (a4_dl.h). */
DATAFILE *load_datafile_callback(const char *filename, void (*callback)(DATAFILE *))
{
   DATAFILE *d;
   a4_dl_begin_static();
   d = load_datafile_callback_impl(filename, callback);
   a4_dl_end_static();
   return d;
}

static DATAFILE *load_datafile_callback_impl(const char *filename, void (*callback)(DATAFILE *))
{
   PACKFILE *f;
   DATAFILE *dat;
   int type;

   if (!filename)
      return NULL;
   init_types();
   f = pack_fopen(filename, F_READ_PACKED);
   if (!f)
      return NULL;

   if (a4_pack_is_datafile_chunk(f))
      type = (a4_packfile_type == DAT_FILE) ? DAT_MAGIC : 0;
   else
      type = (int)pack_mgetl(f);

   if (type == (int)V1_DAT_MAGIC) {
      dat = read_old_datafile(f, callback);
   } else if (type == DAT_MAGIC) {
      datafile_callback = callback;
      dat = (DATAFILE *)load_file_object(f, 0);
      datafile_callback = NULL;
   } else {
      dat = NULL;
   }
   pack_fclose(f);
   return dat;
}

DATAFILE *load_datafile(const char *filename)
{
   return load_datafile_callback(filename, NULL);
}

static void unload_datafile_object(DATAFILE *dat)
{
   int i;
   if (dat->prop)
      destroy_property_list(dat->prop);
   for (i = 0; i < MAX_DATAFILE_TYPES; i++) {
      if (datafile_type[i].type == dat->type) {
         if (dat->dat) {
            if (datafile_type[i].destroy)
               datafile_type[i].destroy(dat->dat);
            else
               free(dat->dat);
         }
         return;
      }
   }
   free(dat->dat);
}

void unload_datafile(DATAFILE *dat)
{
   int i;
   if (dat) {
      init_types();
      for (i = 0; dat[i].type != DAT_END; i++)
         unload_datafile_object(dat + i);
      free(dat);
   }
}

/* ================================================================== */
/* image files (readbmp.c)                                            */
/* ================================================================== */

typedef struct BITMAP_TYPE_INFO {
   char *ext;
   BITMAP *(*load)(const char *filename, RGB *pal);
   int (*save)(const char *filename, BITMAP *bmp, const RGB *pal);
   struct BITMAP_TYPE_INFO *next;
} BITMAP_TYPE_INFO;

static BITMAP_TYPE_INFO *bitmap_type_list;

static void init_image_types(void);

void register_bitmap_file_type(const char *ext,
                               BITMAP *(*load)(const char *filename, RGB *pal),
                               int (*save)(const char *filename, BITMAP *bmp, const RGB *pal))
{
   char tmp[32];
   BITMAP_TYPE_INFO *iter;
   size_t n;

   if (!ext)
      return;
   init_image_types();
   a4_utoascii(ext, tmp, sizeof(tmp));
   if (strlen(tmp) == 0)
      return;
   iter = (BITMAP_TYPE_INFO *)calloc(1, sizeof(BITMAP_TYPE_INFO));
   if (!iter)
      return;
   n = strlen(tmp) + 1;
   iter->ext = (char *)malloc(n);
   if (!iter->ext) {
      free(iter);
      return;
   }
   memcpy(iter->ext, tmp, n);
   iter->load = load;
   iter->save = save;
   if (!bitmap_type_list) {
      bitmap_type_list = iter;
   } else {
      BITMAP_TYPE_INFO *last = bitmap_type_list;
      while (last->next)
         last = last->next;
      last->next = iter;
   }
}

BITMAP *load_bitmap(const char *filename, RGB *pal)
{
   char tmp[32];
   BITMAP_TYPE_INFO *iter;
   if (!filename)
      return NULL;
   init_image_types();
   a4_utoascii(get_extension(filename), tmp, sizeof(tmp));
   for (iter = bitmap_type_list; iter; iter = iter->next) {
      if (ascii_stricmp(iter->ext, tmp) == 0) {
         if (iter->load) {
            BITMAP *b;
            a4_dl_begin_static();
            b = iter->load(filename, pal);
            a4_dl_end_static();
            return b;
         }
         return NULL;
      }
   }
   return NULL;
}

int save_bitmap(const char *filename, BITMAP *bmp, const RGB *pal)
{
   char tmp[32];
   BITMAP_TYPE_INFO *iter;
   if (!filename || !bmp)
      return 1;
   init_image_types();
   a4_utoascii(get_extension(filename), tmp, sizeof(tmp));
   for (iter = bitmap_type_list; iter; iter = iter->next) {
      if (ascii_stricmp(iter->ext, tmp) == 0) {
         if (iter->save)
            return iter->save(filename, bmp, pal);
         return 1;
      }
   }
   return 1;
}

/* _fixup_loaded_bitmap (readbmp.c).  Reduction to 8 bpp uses the given
 * (or current) palette as is: Allegro's generate_optimized_palette() and
 * RGB map are not implemented. */
BITMAP *_fixup_loaded_bitmap(BITMAP *bmp, RGB *pal, int bpp)
{
   BITMAP *b2;
   if (!bmp)
      return NULL;
   b2 = create_bitmap_ex(bpp, bmp->w, bmp->h);
   if (!b2) {
      destroy_bitmap(bmp);
      return NULL;
   }
   if (bpp == 8) {
      if (pal) {
         select_palette(pal);
         a4_convert_blit(bmp, b2, a4_color_conversion);
         unselect_palette();
      } else {
         a4_convert_blit(bmp, b2, a4_color_conversion);
      }
   } else if (bitmap_color_depth(bmp) == 8 && pal) {
      select_palette(pal);
      a4_convert_blit(bmp, b2, a4_color_conversion);
      unselect_palette();
   } else {
      a4_convert_blit(bmp, b2, a4_color_conversion);
   }
   destroy_bitmap(bmp);
   return b2;
}

/* ================================================================== */
/* built-in BMP and PCX image formats (bmp.c, pcx.c)                   */
/* ================================================================== */

#define BI_RGB          0
#define BI_RLE8         1
#define BI_RLE4         2
#define BI_BITFIELDS    3

typedef struct BMINFO {
   unsigned long biWidth;
   long biHeight;
   unsigned short biBitCount;
   unsigned long biCompression;
} BMINFO;

static int bmp_valid(BITMAP *bmp, int line, int pos)
{
   return line >= 0 && line < bmp->h && pos >= 0 && pos < bmp->w;
}

/* read_bmicolors (bmp.c) */
static void read_bmicolors(int bytes, RGB *pal, PACKFILE *f, int win_flag)
{
   int i, j;
   for (i = j = 0; (i + 3 <= bytes && j < PAL_SIZE); j++) {
      pal[j].b = (unsigned char)(pack_getc(f) / 4);
      pal[j].g = (unsigned char)(pack_getc(f) / 4);
      pal[j].r = (unsigned char)(pack_getc(f) / 4);
      i += 3;
      if (win_flag && i < bytes) {
         pack_getc(f);
         i++;
      }
   }
   for (; i < bytes; i++)
      pack_getc(f);
}

/* read_image and the read_*bit_line helpers (bmp.c) */
static void read_bmp_image(PACKFILE *f, BITMAP *bmp, const BMINFO *ih)
{
   int i, line, height, dir, x, j, k;
   unsigned char b[32];
   unsigned long n;

   height = (int)ih->biHeight;
   line = height < 0 ? 0 : height - 1;
   dir = height < 0 ? 1 : -1;
   height = ABS(height);

   for (i = 0; i < height; i++, line += dir) {
      int length = (int)ih->biWidth;
      switch (ih->biBitCount) {
         case 1:
            for (x = 0; x < length; x++) {
               j = x % 32;
               if (j == 0) {
                  n = (unsigned long)pack_mgetl(f);
                  for (k = 0; k < 32; k++) {
                     b[31 - k] = (unsigned char)(n & 1);
                     n = n >> 1;
                  }
               }
               bmp->line[line][x] = b[j];
            }
            break;
         case 4:
            for (x = 0; x < length; x++) {
               j = x % 8;
               if (j == 0) {
                  n = (unsigned long)pack_igetl(f);
                  for (k = 0; k < 4; k++) {
                     int temp = (int)(n & 255);
                     b[k * 2 + 1] = (unsigned char)(temp & 15);
                     temp = temp >> 4;
                     b[k * 2] = (unsigned char)(temp & 15);
                     n = n >> 8;
                  }
               }
               bmp->line[line][x] = b[j];
            }
            break;
         case 8:
            for (x = 0; x < length; x++) {
               j = x % 4;
               if (j == 0) {
                  n = (unsigned long)pack_igetl(f);
                  for (k = 0; k < 4; k++) {
                     b[k] = (unsigned char)(n & 255);
                     n = n >> 8;
                  }
               }
               bmp->line[line][x] = b[j];
            }
            break;
         case 16:
            /* the format is like a 15 bpp bitmap, stored into 16 bpp */
            for (x = 0; x < length; x++) {
               int w = pack_igetw(f);
               ((uint16_t *)bmp->line[line])[x] = (uint16_t)makecol16(
                  _rgb_scale_5[(w >> 10) & 0x1f], _rgb_scale_5[(w >> 5) & 0x1f], _rgb_scale_5[w & 0x1f]);
            }
            x = (x * 2) % 4;
            if (x != 0)
               while (x++ < 4)
                  pack_getc(f);
            break;
         case 24:
            for (x = 0; x < length; x++) {
               int cb = pack_getc(f), cg = pack_getc(f), cr = pack_getc(f);
               a4_put_raw(bmp, x, line, (unsigned long)makecol24(cr & 0xFF, cg & 0xFF, cb & 0xFF) & 0xFFFFFF);
            }
            x = (x * 3) % 4;
            if (x != 0)
               while (x++ < 4)
                  pack_getc(f);
            break;
         case 32:
            /* Allegro writes only three bytes of each pixel (alpha stays 0) */
            for (x = 0; x < length; x++) {
               int cb = pack_getc(f), cg = pack_getc(f), cr = pack_getc(f);
               unsigned long c = (unsigned long)makecol32(cr & 0xFF, cg & 0xFF, cb & 0xFF);
               unsigned char *p = bmp->line[line] + x * 4;
               pack_getc(f);
               p[0] = (unsigned char)c;
               p[1] = (unsigned char)(c >> 8);
               p[2] = (unsigned char)(c >> 16);
            }
            break;
      }
   }
}

static void read_bitfields_image(PACKFILE *f, BITMAP *bmp, const BMINFO *ih)
{
   int k, i, line, height, dir, bpp, bytes_per_pixel;
   unsigned long red, grn, blu, buffer;
   unsigned char raw[4];

   height = (int)ih->biHeight;
   line = height < 0 ? 0 : height - 1;
   dir = height < 0 ? 1 : -1;
   height = ABS(height);
   bpp = bmp->depth;
   bytes_per_pixel = a4_bpp_bytes(bpp);

   for (i = 0; i < height; i++, line += dir) {
      for (k = 0; k < (int)ih->biWidth; k++) {
         int b;
         pack_fread(raw, bytes_per_pixel, f);
         buffer = 0;
         for (b = 0; b < bytes_per_pixel; b++)
            buffer |= (unsigned long)raw[b] << (8 * b);
         if (bpp == 15) {
            red = (buffer >> 10) & 0x1f;
            grn = (buffer >> 5) & 0x1f;
            blu = buffer & 0x1f;
            buffer = (red << 10) | (grn << 5) | blu;
         } else if (bpp == 16) {
            red = (buffer >> 11) & 0x1f;
            grn = (buffer >> 5) & 0x3f;
            blu = buffer & 0x1f;
            buffer = (red << 11) | (grn << 5) | blu;
         } else {
            red = (buffer >> 16) & 0xff;
            grn = (buffer >> 8) & 0xff;
            blu = buffer & 0xff;
            buffer = (red << 16) | (grn << 8) | blu;
         }
         a4_put_raw(bmp, k, line, buffer);
      }
      k = (k * bytes_per_pixel) % 4;
      if (k > 0)
         while (k++ < 4)
            pack_getc(f);
   }
}

/* read_RLE8/RLE4_compressed_image (bmp.c); out-of-range writes are
 * dropped instead of corrupting memory */
static void read_rle_image(PACKFILE *f, BITMAP *bmp, const BMINFO *ih, int rle4)
{
   unsigned char b[8];
   unsigned char count;
   unsigned int val, val0;
   int j, k, pos, line, eolflag, eopicflag = 0;

   line = (int)ih->biHeight - 1;
   while (eopicflag == 0) {
      pos = 0;
      eolflag = 0;
      while ((eolflag == 0) && (eopicflag == 0)) {
         count = (unsigned char)pack_getc(f);
         val = (unsigned char)pack_getc(f);
         if (count > 0) {
            b[1] = (unsigned char)(val & 15);
            b[0] = (unsigned char)((val >> 4) & 15);
            for (j = 0; j < count; j++) {
               if (bmp_valid(bmp, line, pos))
                  bmp->line[line][pos] = rle4 ? b[j % 2] : (unsigned char)val;
               pos++;
            }
         } else {
            switch (val) {
               case 0:
                  eolflag = 1;
                  break;
               case 1:
                  eopicflag = 1;
                  break;
               case 2:
                  count = (unsigned char)pack_getc(f);
                  val = (unsigned char)pack_getc(f);
                  pos += count;
                  line -= (int)val;
                  break;
               default:
                  if (rle4) {
                     for (j = 0; j < (int)val; j++) {
                        if ((j % 4) == 0) {
                           val0 = (unsigned int)pack_igetw(f) & 0xFFFF;
                           for (k = 0; k < 2; k++) {
                              b[2 * k + 1] = (unsigned char)(val0 & 15);
                              val0 = val0 >> 4;
                              b[2 * k] = (unsigned char)(val0 & 15);
                              val0 = val0 >> 4;
                           }
                        }
                        if (bmp_valid(bmp, line, pos))
                           bmp->line[line][pos] = b[j % 4];
                        pos++;
                     }
                  } else {
                     for (j = 0; j < (int)val; j++) {
                        val0 = (unsigned char)pack_getc(f);
                        if (bmp_valid(bmp, line, pos))
                           bmp->line[line][pos] = (unsigned char)val0;
                        pos++;
                     }
                     if (j % 2 == 1)
                        pack_getc(f);
                  }
                  break;
            }
         }
         if (pos - 1 > (int)ih->biWidth)
            eolflag = 1;
      }
      line--;
      if (line < 0)
         eopicflag = 1;
   }
}

/* load_bmp_pf (bmp.c) */
static BITMAP *load_bmp(const char *filename, RGB *pal)
{
   PACKFILE *f;
   BITMAP *bmp = NULL;
   PALETTE tmppal;
   BMINFO ih;
   int want_palette = TRUE, bpp, dest_depth;
   long off_bits, bi_size;

   f = pack_fopen(filename, F_READ);
   if (!f)
      return NULL;
   if (!pal) {
      want_palette = FALSE;
      pal = tmppal;
   }

   if (pack_igetw(f) != 19778)       /* "BM" */
      goto done;
   pack_igetl(f);
   pack_igetw(f);
   pack_igetw(f);
   off_bits = pack_igetl(f);

   bi_size = pack_igetl(f);
   if (bi_size == 40) {
      ih.biWidth = (unsigned long)pack_igetl(f);
      ih.biHeight = pack_igetl(f);
      pack_igetw(f);
      ih.biBitCount = (unsigned short)pack_igetw(f);
      ih.biCompression = (unsigned long)pack_igetl(f);
      pack_igetl(f);
      pack_igetl(f);
      pack_igetl(f);
      pack_igetl(f);
      pack_igetl(f);
      if (ih.biCompression != BI_BITFIELDS)
         read_bmicolors((int)(off_bits - 54), pal, f, 1);
   } else if (bi_size == 12) {
      ih.biWidth = (unsigned short)pack_igetw(f);
      ih.biHeight = (unsigned short)pack_igetw(f);
      pack_igetw(f);
      ih.biBitCount = (unsigned short)pack_igetw(f);
      ih.biCompression = 0;
      read_bmicolors((int)(off_bits - 26), pal, f, 0);
   } else {
      goto done;
   }

   if (ih.biBitCount == 24)
      bpp = 24;
   else if (ih.biBitCount == 16)
      bpp = 16;
   else if (ih.biBitCount == 32)
      bpp = 32;
   else
      bpp = 8;

   if (ih.biCompression == BI_BITFIELDS) {
      unsigned long red_mask = (unsigned long)pack_igetl(f) & 0xFFFFFFFFul;
      unsigned long grn_mask = (unsigned long)pack_igetl(f);
      unsigned long blu_mask = (unsigned long)pack_igetl(f) & 0xFFFFFFFFul;
      (void)grn_mask;
      if ((blu_mask == 0x001f) && (red_mask == 0x7C00))
         bpp = 15;
      else if ((blu_mask == 0x001f) && (red_mask == 0xF800))
         bpp = 16;
      else if ((blu_mask == 0x0000FF) && (red_mask == 0xFF0000))
         bpp = 32;
      else
         goto done;
   }

   dest_depth = _color_load_depth(bpp, FALSE);
   bmp = create_bitmap_ex(bpp, (int)ih.biWidth, (int)ABS(ih.biHeight));
   if (!bmp)
      goto done;
   clear_bitmap(bmp);

   switch (ih.biCompression) {
      case BI_RGB:
         read_bmp_image(f, bmp, &ih);
         break;
      case BI_RLE8:
         read_rle_image(f, bmp, &ih, 0);
         break;
      case BI_RLE4:
         read_rle_image(f, bmp, &ih, 1);
         break;
      case BI_BITFIELDS:
         read_bitfields_image(f, bmp, &ih);
         break;
      default:
         destroy_bitmap(bmp);
         bmp = NULL;
   }

   if (dest_depth != bpp) {
      if ((bpp != 8) && (!want_palette))
         pal = NULL;
      if (bmp)
         bmp = _fixup_loaded_bitmap(bmp, pal, dest_depth);
   }
   if ((bpp != 8) && (dest_depth != 8) && want_palette)
      generate_332_palette(pal);

 done:
   pack_fclose(f);
   return bmp;
}

/* save_bmp_pf (bmp.c): 8 bpp with palette, everything else as 24 bpp */
static int save_bmp(const char *filename, BITMAP *bmp, const RGB *pal)
{
   PACKFILE *f;
   PALETTE tmppal;
   int bf_size, bi_size_image, depth, bpp, filler, c, i, j;

   f = pack_fopen(filename, F_WRITE);
   if (!f)
      return -1;

   depth = bitmap_color_depth(bmp);
   bpp = (depth == 8) ? 8 : 24;
   filler = 3 - ((bmp->w * (bpp / 8) - 1) & 3);
   if (!pal) {
      get_palette(tmppal);
      pal = tmppal;
   }
   if (bpp == 8) {
      bi_size_image = (bmp->w + filler) * bmp->h;
      bf_size = 54 + 256 * 4 + bi_size_image;
   } else {
      bi_size_image = (bmp->w * 3 + filler) * bmp->h;
      bf_size = 54 + bi_size_image;
   }

   set_errno(0);
   pack_iputw(0x4D42, f);
   pack_iputl(bf_size, f);
   pack_iputw(0, f);
   pack_iputw(0, f);
   pack_iputl(bpp == 8 ? 54 + 256 * 4 : 54, f);
   pack_iputl(40, f);
   pack_iputl(bmp->w, f);
   pack_iputl(bmp->h, f);
   pack_iputw(1, f);
   pack_iputw(bpp, f);
   pack_iputl(0, f);
   pack_iputl(bi_size_image, f);
   pack_iputl(0xB12, f);
   pack_iputl(0xB12, f);
   if (bpp == 8) {
      pack_iputl(256, f);
      pack_iputl(256, f);
      for (i = 0; i < 256; i++) {
         pack_putc(_rgb_scale_6[pal[i].b], f);
         pack_putc(_rgb_scale_6[pal[i].g], f);
         pack_putc(_rgb_scale_6[pal[i].r], f);
         pack_putc(0, f);
      }
   } else {
      pack_iputl(0, f);
      pack_iputl(0, f);
   }
   for (i = bmp->h - 1; i >= 0; i--) {
      for (j = 0; j < bmp->w; j++) {
         if (bpp == 8) {
            pack_putc(getpixel(bmp, j, i), f);
         } else {
            c = getpixel(bmp, j, i);
            pack_putc(getb_depth(depth, c), f);
            pack_putc(getg_depth(depth, c), f);
            pack_putc(getr_depth(depth, c), f);
         }
      }
      for (j = 0; j < filler; j++)
         pack_putc(0, f);
   }
   i = (allegro_errno && *allegro_errno) ? -1 : 0;
   pack_fclose(f);
   return i;
}

/* load_pcx_pf (pcx.c): 8 bpp and 24 bpp (3 planes) */
static BITMAP *load_pcx(const char *filename, RGB *pal)
{
   PACKFILE *f;
   BITMAP *b = NULL;
   PALETTE tmppal;
   int want_palette = TRUE;
   int c, width, height, bpp, bytes_per_line, xx, po, x, y, dest_depth;
   signed char ch;

   f = pack_fopen(filename, F_READ);
   if (!f)
      return NULL;
   if (!pal) {
      want_palette = FALSE;
      pal = tmppal;
   }

   pack_getc(f);                    /* manufacturer */
   pack_getc(f);                    /* version */
   pack_getc(f);                    /* encoding */
   if (pack_getc(f) != 8)
      goto done;

   width = -(pack_igetw(f));
   height = -(pack_igetw(f));
   width += pack_igetw(f) + 1;
   height += pack_igetw(f) + 1;
   pack_igetl(f);                   /* DPI */

   for (c = 0; c < 16; c++) {
      pal[c].r = (unsigned char)(pack_getc(f) / 4);
      pal[c].g = (unsigned char)(pack_getc(f) / 4);
      pal[c].b = (unsigned char)(pack_getc(f) / 4);
   }
   pack_getc(f);

   bpp = pack_getc(f) * 8;
   if ((bpp != 8) && (bpp != 24))
      goto done;

   dest_depth = _color_load_depth(bpp, FALSE);
   bytes_per_line = pack_igetw(f);
   for (c = 0; c < 60; c++)
      pack_getc(f);

   b = create_bitmap_ex(bpp, width, height);
   if (!b)
      goto done;

   set_errno(0);
   for (y = 0; y < height; y++) {
      x = xx = 0;
      po = 2;                        /* red byte of the 24 bpp layout */
      while (x < bytes_per_line * bpp / 8) {
         ch = (signed char)pack_getc(f);
         if ((ch & 0xC0) == 0xC0) {
            c = (ch & 0x3F);
            ch = (signed char)pack_getc(f);
         } else {
            c = 1;
         }
         if (bpp == 8) {
            while (c--) {
               if (x < b->w)
                  b->line[y][x] = (unsigned char)ch;
               x++;
            }
         } else {
            while (c--) {
               if (xx < b->w)
                  b->line[y][xx * 3 + po] = (unsigned char)ch;
               x++;
               if (x == bytes_per_line) {
                  xx = 0;
                  po = 1;
               } else if (x == bytes_per_line * 2) {
                  xx = 0;
                  po = 0;
               } else {
                  xx++;
               }
            }
         }
      }
   }

   if (bpp == 8) {
      while ((c = pack_getc(f)) != EOF) {
         if (c == 12) {
            for (c = 0; c < 256; c++) {
               pal[c].r = (unsigned char)(pack_getc(f) / 4);
               pal[c].g = (unsigned char)(pack_getc(f) / 4);
               pal[c].b = (unsigned char)(pack_getc(f) / 4);
            }
            break;
         }
      }
   }

   if (allegro_errno && *allegro_errno) {
      destroy_bitmap(b);
      b = NULL;
      goto done;
   }

   if (dest_depth != bpp) {
      if ((bpp != 8) && (!want_palette))
         pal = NULL;
      b = _fixup_loaded_bitmap(b, pal, dest_depth);
   }
   if ((bpp != 8) && (dest_depth != 8) && want_palette)
      generate_332_palette(pal);

 done:
   pack_fclose(f);
   return b;
}

/* save_pcx_pf (pcx.c) */
static int save_pcx(const char *filename, BITMAP *bmp, const RGB *pal)
{
   PACKFILE *f;
   PALETTE tmppal;
   int c, x, y, runcount, depth, planes, ret;
   signed char runchar, ch;

   f = pack_fopen(filename, F_WRITE);
   if (!f)
      return -1;
   if (!pal) {
      get_palette(tmppal);
      pal = tmppal;
   }
   depth = bitmap_color_depth(bmp);
   planes = (depth == 8) ? 1 : 3;

   set_errno(0);
   pack_putc(10, f);
   pack_putc(5, f);
   pack_putc(1, f);
   pack_putc(8, f);
   pack_iputw(0, f);
   pack_iputw(0, f);
   pack_iputw(bmp->w - 1, f);
   pack_iputw(bmp->h - 1, f);
   pack_iputw(320, f);
   pack_iputw(200, f);
   for (c = 0; c < 16; c++) {
      pack_putc(_rgb_scale_6[pal[c].r], f);
      pack_putc(_rgb_scale_6[pal[c].g], f);
      pack_putc(_rgb_scale_6[pal[c].b], f);
   }
   pack_putc(0, f);
   pack_putc(planes, f);
   pack_iputw(bmp->w, f);
   pack_iputw(1, f);
   pack_iputw(bmp->w, f);
   pack_iputw(bmp->h, f);
   for (c = 0; c < 54; c++)
      pack_putc(0, f);

   for (y = 0; y < bmp->h; y++) {
      runcount = 0;
      runchar = 0;
      for (x = 0; x < bmp->w * planes; x++) {
         if (depth == 8) {
            ch = (signed char)getpixel(bmp, x, y);
         } else if (x < bmp->w) {
            c = getpixel(bmp, x, y);
            ch = (signed char)getr_depth(depth, c);
         } else if (x < bmp->w * 2) {
            c = getpixel(bmp, x - bmp->w, y);
            ch = (signed char)getg_depth(depth, c);
         } else {
            c = getpixel(bmp, x - bmp->w * 2, y);
            ch = (signed char)getb_depth(depth, c);
         }
         if (runcount == 0) {
            runcount = 1;
            runchar = ch;
         } else if ((ch != runchar) || (runcount >= 0x3f)) {
            if ((runcount > 1) || ((runchar & 0xC0) == 0xC0))
               pack_putc(0xC0 | runcount, f);
            pack_putc((unsigned char)runchar, f);
            runcount = 1;
            runchar = ch;
         } else {
            runcount++;
         }
      }
      if ((runcount > 1) || ((runchar & 0xC0) == 0xC0))
         pack_putc(0xC0 | runcount, f);
      pack_putc((unsigned char)runchar, f);
   }

   if (depth == 8) {
      pack_putc(12, f);
      for (c = 0; c < 256; c++) {
         pack_putc(_rgb_scale_6[pal[c].r], f);
         pack_putc(_rgb_scale_6[pal[c].g], f);
         pack_putc(_rgb_scale_6[pal[c].b], f);
      }
   }
   ret = (allegro_errno && *allegro_errno) ? -1 : 0;
   pack_fclose(f);
   return ret;
}

static int image_types_initialised;

/* _register_bitmap_file_type_init (readbmp.c): the built-in types are
 * registered first; LBM and TGA are not implemented */
static void init_image_types(void)
{
   if (image_types_initialised)
      return;
   image_types_initialised = 1;
   register_bitmap_file_type("bmp", load_bmp, save_bmp);
   register_bitmap_file_type("pcx", load_pcx, save_pcx);
}

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

DATAFILE *load_datafile_callback(const char *filename, void (*callback)(DATAFILE *))
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

void register_bitmap_file_type(const char *ext,
                               BITMAP *(*load)(const char *filename, RGB *pal),
                               int (*save)(const char *filename, BITMAP *bmp, const RGB *pal))
{
   char tmp[32];
   BITMAP_TYPE_INFO *iter;
   size_t n;

   if (!ext)
      return;
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
   a4_utoascii(get_extension(filename), tmp, sizeof(tmp));
   for (iter = bitmap_type_list; iter; iter = iter->next) {
      if (ascii_stricmp(iter->ext, tmp) == 0) {
         if (iter->load)
            return iter->load(filename, pal);
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

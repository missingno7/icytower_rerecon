/*
 * Internal state shared between the compat modules.  Not for game code.
 *
 * Module ownership (one .c file per area):
 *   a4_bitmap.c    bitmap lifetime, clipping, putpixel/getpixel, clear, colour
 *                  depth state, makecol/getr..., palettes, colour conversion
 *   a4_draw.c      drawing modes and blenders, hline/vline/line/rect/rectfill
 *   a4_blit.c      blit (incl. depth conversion), masked_blit, stretch_blit,
 *                  stretch_sprite
 *   a4_sprite.c    draw_sprite (+flips), draw_trans_sprite, rotate_sprite,
 *                  rotate_scaled_sprite
 *   a4_fixed.c     fixed-point tables and fixmul/fixdiv/ftofix/fixtof
 *   a4_text.c      FONT, default 8x8 font, text output and metrics
 *   a4_file.c      PACKFILE (plain, LZSS packed, password), file utilities,
 *                  config files
 *   a4_datafile.c  datafile reader, object registry, load_bitmap/save_bitmap
 *   a4_sound.c     SAMPLE, voices, software mixer, WAV loader, MIDI stubs
 *   a4_system.c    allegro_init/exit, set_gfx_mode, screen, messages, alert,
 *                  display switching, close button, vsync
 *   a4_input.c     keyboard, mouse, joystick
 *   a4_timer.c     install_int/rest; main-thread timer service
 *
 * Pixel formats (identical to Allegro 4.4.1 on Windows):
 *   8  bpp  palette index
 *   15 bpp  0RRRRRGGGGGBBBBB        (r shift 10, g 5, b 0)  mask 0x7C1F
 *   16 bpp  RRRRRGGGGGGBBBBB        (r shift 11, g 5, b 0)  mask 0xF81F
 *   24 bpp  bytes B,G,R in memory   (r shift 16, g 8, b 0)  mask 0xFF00FF
 *   32 bpp  0xAARRGGBB little endian(r shift 16, g 8, b 0, a 24) mask 0xFF00FF
 */
#ifndef A4_INTERNAL_H
#define A4_INTERNAL_H

#include "allegro.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- colour state (a4_bitmap.c) ---------------------------------- */
extern int a4_color_depth;          /* set_color_depth(); default 8 like Allegro */
extern int a4_color_conversion;     /* set_color_conversion() */
extern PALETTE a4_current_palette;  /* last set_palette/select_palette */
extern int a4_palette_color[256];   /* palette index -> colour in a4_color_depth */
int a4_palette_color_depth(int depth, int index);  /* same, for another depth */

/* ---- drawing mode / blenders (a4_draw.c) -------------------------- */
enum { A4_BLEND_TRANS = 0, A4_BLEND_ALPHA = 1 };
extern int a4_draw_mode;            /* DRAW_MODE_* */
extern int a4_blend_kind;           /* set_trans_blender vs set_alpha_blender */
extern int a4_blend_r, a4_blend_g, a4_blend_b, a4_blend_a;
/* Blend `src` over `dst` (both in `depth`) with the current blender and
 * factor n (0..255), exactly like Allegro's _blender_func<depth>. */
unsigned long a4_blend(int depth, unsigned long src, unsigned long dst, unsigned long n);

/* ---- bitmaps (a4_bitmap.c) --------------------------------------- */
static inline BITMAP *a4_root(BITMAP *b) { return b->parent ? b->parent : b; }
/* Mark a bitmap (and its root) as modified; used by texture caches and the
 * legacy presenter.  Every drawing primitive must call this for its target. */
void a4_touch(BITMAP *b);
static inline int a4_bpp_bytes(int depth) { return depth == 8 ? 1 : depth <= 16 ? 2 : depth == 24 ? 3 : 4; }
static inline unsigned long a4_mask_color(int depth)
{
   switch (depth) {
      case 8: return MASK_COLOR_8;
      case 15: return MASK_COLOR_15;
      case 16: return MASK_COLOR_16;
      case 24: return MASK_COLOR_24;
      default: return MASK_COLOR_32;
   }
}
/* raw pixel access without clipping (caller checks bounds) */
static inline unsigned long a4_get_raw(BITMAP *b, int x, int y)
{
   unsigned char *p = b->line[y];
   switch (b->depth) {
      case 8: return p[x];
      case 15: case 16: return ((uint16_t *)p)[x];
      case 24: p += x * 3; return (unsigned long)p[0] | ((unsigned long)p[1] << 8) | ((unsigned long)p[2] << 16);
      default: return ((uint32_t *)p)[x];
   }
}
static inline void a4_put_raw(BITMAP *b, int x, int y, unsigned long c)
{
   unsigned char *p = b->line[y];
   switch (b->depth) {
      case 8: p[x] = (unsigned char)c; break;
      case 15: case 16: ((uint16_t *)p)[x] = (uint16_t)c; break;
      case 24: p += x * 3; p[0] = c & 0xFF; p[1] = (c >> 8) & 0xFF; p[2] = (c >> 16) & 0xFF; break;
      default: ((uint32_t *)p)[x] = (uint32_t)c; break;
   }
}
/* Convert a colour between depths the way Allegro's blit does
 * (8 bpp sources go through the current palette). */
unsigned long a4_convert_color(unsigned long c, int from_depth, int to_depth);

/* ---- fonts (a4_text.c; loaded by a4_datafile.c) -------------------- */
typedef struct A4_GLYPH {
   int w, h;
   unsigned char *mono;       /* mono fonts: 1 bpp, MSB first, (w+7)/8 bytes per row */
   BITMAP *bmp;               /* colour fonts: glyph bitmap (magenta/0 = transparent) */
} A4_GLYPH;

typedef struct A4_FONT_RANGE {
   int begin, end;            /* code points [begin, end) */
   A4_GLYPH *glyphs;          /* end - begin entries */
} A4_FONT_RANGE;

struct FONT {
   int height;
   int is_color;              /* 0 = mono glyphs, 1 = colour bitmap glyphs */
   int nranges;
   A4_FONT_RANGE *ranges;
   uint32_t serial;           /* unique id, for texture caches */
};
/* Allocates a zeroed FONT with `nranges` zeroed ranges.  The caller fills
 * ranges[i].begin/end and allocates ranges[i].glyphs (calloc, end-begin
 * entries), each glyph's `mono` (malloc) or `bmp` (create_bitmap_ex);
 * destroy_font() frees all of it. */
FONT *a4_font_create(int height, int is_color, int nranges);
const A4_GLYPH *a4_font_glyph(const FONT *f, int codepoint);  /* falls back to '^' like Allegro */
/* Allegro's UTF-8 decoding rule for text: returns code point, advances *s. */
int a4_ugetx(const char **s);

/* ---- Allegro UTF-8 string helpers (a4_text.c; unicode.c semantics) -- */
int  a4_ugetc(const char *s);                 /* decode without advancing */
int  a4_usetc(char *s, int c);                /* encode, returns bytes written */
int  a4_ucwidth(int c);                       /* encoded size of c */
int  a4_uwidth(const char *s);                /* size of the char at s (lead byte rule) */
int  a4_ustrlen(const char *s);
int  a4_ustrsize(const char *s);              /* bytes, without terminator */
int  a4_uoffset(const char *s, int index);    /* negative index counts from the end */
int  a4_ugetat(const char *s, int index);
int  a4_utolower(int c);
int  a4_uisspace(int c);
int  a4_ustricmp(const char *s1, const char *s2);
char *a4_ustrzcpy(char *dest, int size, const char *src);
char *a4_ustrzcat(char *dest, int size, const char *src);
char *a4_ustrzncpy(char *dest, int size, const char *src, int n);
/* uconvert_toascii(): chars > 255 become '^'; dest holds `size` bytes */
char *a4_utoascii(const char *s, char *dest, int size);

/* ---- packfile internals (a4_file.c) -------------------------------- */
PACKFILE *a4_pack_fopen_chunk(PACKFILE *f, int pack);   /* read side only */
PACKFILE *a4_pack_fclose_chunk(PACKFILE *f);
long a4_pack_todo(PACKFILE *f);                /* bytes left (normal.todo) */
int  a4_pack_is_datafile_chunk(PACKFILE *f);   /* CHUNK && !EXEDAT */
extern int a4_packfile_type;                   /* _packfile_type */

/* ---- datafile helpers (a4_datafile.c) ------------------------------ */
/* read an Allegro datafile BITMAP object body (after type/size header) */
BITMAP *a4_read_bitmap_object(PACKFILE *f, long size);
/* Allegro's _blit_between_formats() for a whole bitmap onto an equally
 * sized bitmap of another depth, honouring COLORCONV_KEEP_TRANS in `conv`
 * (dithering flags are not implemented).  Same-depth copies are plain. */
void a4_convert_blit(BITMAP *src, BITMAP *dst, int conv);

/* ---- sound (a4_sound.c) ------------------------------------------- */
/* Mix `frames` stereo float frames into `out` (called by the audio device
 * thread through port/platform/audio.h). */
void a4_mix(float *out, int frames, int device_freq);

/* ---- service (a4_system.c / a4_timer.c) --------------------------- */
/* Main-thread service point: pump platform events, run due timer
 * callbacks, present the legacy screen when appropriate.  Called from
 * rest(), vsync(), key[] reads, keypressed(), poll_joystick() ... */
void a4_service(void);
void a4_timer_service(void);
extern volatile int a4_close_requested;

#ifdef __cplusplus
}
#endif

#endif

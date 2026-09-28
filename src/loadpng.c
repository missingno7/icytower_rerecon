/* loadpng, Allegro wrapper routines for PNG images.
 *
 * Historical version: Peter Wang's loadpng 1.5 over libpng 1.2 (public
 * domain).  Portable build: the same API implemented on SDL3's built-in PNG
 * codec (SDL_LoadPNG_IO), so the port needs neither libpng nor zlib.
 *
 * Kept from the historical routine: paletted images load as 8-bit bitmaps
 * with the palette in 6-bit VGA units, RGB as 24-bit, RGBA (and paletted
 * images with transparency, which libpng expanded) as 32-bit with alpha;
 * then Allegro converts to the colour depth selected by _color_load_depth().
 * Not reproduced: libpng gamma correction.  It only changed images whose
 * gAMA chunk differs from 1/2.2 by more than 5 %; the game's own images have
 * none.  Documented in docs/port/ARCHITECTURE.md.
 */
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include <allegro.h>
#include "loadpng.h"
#include "port/platform/platform.h"

double _png_screen_gamma = -1.0;
int _png_compression_level = 9;

static BITMAP *surface_to_bitmap(SDL_Surface *s, RGB *pal)
{
    PALETTE tmppal;
    BITMAP *bmp;
    int x, y, bpp, dest_bpp;
    const SDL_Palette *sp = NULL;
    bool palette_alpha = false;

    if (!pal)
        pal = tmppal;
    if (s->format == SDL_PIXELFORMAT_INDEX8)
        sp = SDL_GetSurfacePalette(s);
    if (sp) {
        int i;
        for (i = 0; i < sp->ncolors && i < 256; i++) {
            pal[i].r = sp->colors[i].r >> 2;
            pal[i].g = sp->colors[i].g >> 2;
            pal[i].b = sp->colors[i].b >> 2;
            if (sp->colors[i].a != 255)
                palette_alpha = true;
        }
        for (; i < 256; i++)
            pal[i].r = pal[i].g = pal[i].b = 0;
    } else {
        generate_332_palette(pal);
    }

    if (sp && !palette_alpha) {
        bpp = 8;
        bmp = create_bitmap_ex(8, s->w, s->h);
        if (!bmp)
            return NULL;
        for (y = 0; y < s->h; y++)
            memcpy(bmp->line[y], (Uint8 *)s->pixels + (size_t)y * s->pitch, (size_t)s->w);
    } else {
        SDL_Surface *c = SDL_ConvertSurface(s, SDL_PIXELFORMAT_RGBA32);
        bool has_alpha = palette_alpha || SDL_ISPIXELFORMAT_ALPHA(s->format);
        if (!c)
            return NULL;
        bpp = has_alpha ? 32 : 24;
        bmp = create_bitmap_ex(bpp, s->w, s->h);
        if (!bmp) {
            SDL_DestroySurface(c);
            return NULL;
        }
        for (y = 0; y < s->h; y++) {
            const Uint8 *src = (const Uint8 *)c->pixels + (size_t)y * c->pitch;
            for (x = 0; x < s->w; x++, src += 4) {
                if (bpp == 32)
                    ((uint32_t *)bmp->line[y])[x] =
                        ((uint32_t)src[3] << 24) | ((uint32_t)src[0] << 16) | ((uint32_t)src[1] << 8) | src[2];
                else {
                    bmp->line[y][x * 3 + 0] = src[2];
                    bmp->line[y][x * 3 + 1] = src[1];
                    bmp->line[y][x * 3 + 2] = src[0];
                }
            }
        }
        SDL_DestroySurface(c);
    }

    dest_bpp = _color_load_depth(bpp, bpp == 32);
    if (dest_bpp != bpp)
        bmp = _fixup_loaded_bitmap(bmp, pal, dest_bpp);
    return bmp;
}

BITMAP *load_memory_png(AL_CONST void *buffer, int bufsize, RGB *pal)
{
    SDL_IOStream *io;
    SDL_Surface *s;
    BITMAP *bmp;
    if (!buffer || bufsize < 8)
        return NULL;
    io = SDL_IOFromConstMem(buffer, (size_t)bufsize);
    if (!io)
        return NULL;
    s = SDL_LoadPNG_IO(io, true);
    if (!s)
        return NULL;
    bmp = surface_to_bitmap(s, pal);
    SDL_DestroySurface(s);
    return bmp;
}

BITMAP *load_png_pf(PACKFILE *fp, RGB *pal)
{
    size_t cap = 65536, len = 0;
    unsigned char *buf = malloc(cap);
    long n;
    BITMAP *bmp;
    if (!buf)
        return NULL;
    while ((n = pack_fread(buf + len, (long)(cap - len), fp)) > 0) {
        len += (size_t)n;
        if (len == cap) {
            unsigned char *nb = realloc(buf, cap * 2);
            if (!nb)
                break;
            buf = nb;
            cap *= 2;
        }
    }
    bmp = load_memory_png(buf, (int)len, pal);
    free(buf);
    return bmp;
}

BITMAP *load_png(AL_CONST char *filename, RGB *pal)
{
    PACKFILE *fp;
    BITMAP *bmp;
    fp = pack_fopen(filename, "r");
    if (!fp)
        return NULL;
    bmp = load_png_pf(fp, pal);
    pack_fclose(fp);
    return bmp;
}

/* savepng: save an Allegro bitmap as PNG.
 *
 * Historical version: loadpng 1.5 over libpng 1.2 (public domain).  Portable
 * build: SDL3's built-in PNG writer.  8-bit bitmaps are written paletted,
 * others as RGB.  The historical writer stored 32-bit bitmaps as RGBA using
 * the (unused, zero) alpha byte, which made the game's screenshots fully
 * transparent in most viewers; the port writes RGBA only when the bitmap
 * really carries alpha.
 */
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include <allegro.h>
#include "loadpng.h"
#include "port/platform/platform.h"

static SDL_Surface *bitmap_to_surface(BITMAP *bmp, AL_CONST RGB *pal)
{
    SDL_Surface *s;
    int x, y;
    int depth = bitmap_color_depth(bmp);

    if (depth == 8) {
        SDL_Palette *p;
        int i;
        if (!pal)
            return NULL;
        s = SDL_CreateSurface(bmp->w, bmp->h, SDL_PIXELFORMAT_INDEX8);
        if (!s)
            return NULL;
        p = SDL_CreateSurfacePalette(s);
        for (i = 0; p && i < 256 && i < p->ncolors; i++) {
            SDL_Color c;
            c.r = (Uint8)_rgb_scale_6[pal[i].r];
            c.g = (Uint8)_rgb_scale_6[pal[i].g];
            c.b = (Uint8)_rgb_scale_6[pal[i].b];
            c.a = 255;
            SDL_SetPaletteColors(p, &c, i, 1);
        }
        for (y = 0; y < bmp->h; y++)
            memcpy((Uint8 *)s->pixels + (size_t)y * s->pitch, bmp->line[y], (size_t)bmp->w);
        return s;
    } else {
        bool alpha = false;
        if (depth == 32)
            for (y = 0; y < bmp->h && !alpha; y++)
                for (x = 0; x < bmp->w; x++)
                    if (geta32(getpixel(bmp, x, y))) { alpha = true; break; }
        s = SDL_CreateSurface(bmp->w, bmp->h, alpha ? SDL_PIXELFORMAT_RGBA32 : SDL_PIXELFORMAT_RGB24);
        if (!s)
            return NULL;
        for (y = 0; y < bmp->h; y++) {
            Uint8 *d = (Uint8 *)s->pixels + (size_t)y * s->pitch;
            for (x = 0; x < bmp->w; x++) {
                int c = getpixel(bmp, x, y);
                *d++ = (Uint8)getr_depth(depth, c);
                *d++ = (Uint8)getg_depth(depth, c);
                *d++ = (Uint8)getb_depth(depth, c);
                if (alpha)
                    *d++ = (Uint8)geta32(c);
            }
        }
        return s;
    }
}

int save_png(AL_CONST char *filename, BITMAP *bmp, AL_CONST RGB *pal)
{
    char native[2048];
    SDL_Surface *s;
    bool ok;
    if (!filename || !bmp)
        return -1;
    if (!plat_resolve_write(filename, native, sizeof(native)))
        return -1;
    s = bitmap_to_surface(bmp, pal);
    if (!s)
        return -1;
    ok = SDL_SavePNG(s, native);
    SDL_DestroySurface(s);
    return ok ? 0 : -1;
}

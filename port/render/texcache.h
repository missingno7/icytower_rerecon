/*
 * Texture cache: original game bitmaps as GPU textures, uploaded once at
 * their native resolution (never pre-scaled) and re-uploaded only when the
 * bitmap's pixels change (BITMAP.generation).
 */
#ifndef PORT_TEXCACHE_H
#define PORT_TEXCACHE_H

#include <SDL3/SDL.h>
#include "a4_internal.h"

typedef enum tex_variant {
   TEXV_OPAQUE = 0,     /* every pixel opaque (blit) */
   TEXV_MASKED = 1,     /* mask colour transparent (draw_sprite, masked_blit) */
   TEXV_ALPHA = 2,      /* alpha from the 32-bit source (set_alpha_blender) */
   TEXV_SILHOUETTE = 3, /* white where not masked (solid-colour glyphs) */
   TEXV_COUNT
} tex_variant;

void texcache_init(SDL_Renderer *r);
void texcache_shutdown(void);
SDL_Texture *texcache_get(BITMAP *b, tex_variant v);
/* mono glyph as a white alpha mask */
SDL_Texture *texcache_glyph(const A4_GLYPH *g);
void texcache_evict(BITMAP *b);
/* apply the configured sampling filter to a texture before drawing */
void texcache_filter(SDL_Texture *t);
/* frame bookkeeping: evict textures unused for many frames */
void texcache_frame_end(void);
/* palette changes invalidate 8-bit uploads */
void texcache_palette_changed(void);

#endif

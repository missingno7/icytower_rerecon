/*
 * Replays compat display lists (a4_dl.h) with SDL at output resolution.
 */
#ifndef PORT_DLRENDER_H
#define PORT_DLRENDER_H

#include <SDL3/SDL.h>
#include "a4_dl.h"

/* canvas coordinates -> output pixels: (ox + x*s, oy + y*s) */
typedef struct xform {
   float ox, oy, s;
} xform;

typedef struct dl_ctx dl_ctx;
typedef void (*dl_underlay_fn)(dl_ctx *c, const xform *t, uint64_t gen);

struct dl_ctx {
   SDL_Renderer *r;
   xform t;
   SDL_Rect bounds;          /* output rectangle this list may draw into */
   float alpha;              /* group opacity 0..1 */
   int masked;               /* inside a masked nested list: mask-colour fills are transparent */
   dl_underlay_fn underlay;
   void *user;
};

/* returns 0 when the list is invalid (caller must fall back to pixels) */
int dl_render(dl_ctx *c, const A4_DL *dl);

/* helpers shared with the modern renderer */
SDL_FRect xf_rect(const xform *t, float x, float y, float w, float h);
void dl_draw_text(dl_ctx *c, const xform *t, const FONT *f, const char *s, float x, float y,
                  int color_mode, uint32_t rgb, int32_t bg);
void dl_set_clip(dl_ctx *c, const xform *t, int cl, int ct, int cr, int cb);

#endif

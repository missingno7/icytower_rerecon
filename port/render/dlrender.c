/*
 * Display-list replay at output resolution (see dlrender.h, a4_dl.h).
 *
 * Canvas operations are drawn from the original bitmaps and glyphs as
 * textures, positioned by the canvas->output transform.  Rectangles are
 * rounded edge by edge, so neighbouring operations share pixel edges and
 * tile seams cannot open at fractional scales.
 */
#include <math.h>
#include <string.h>
#include "port/render/dlrender.h"
#include "port/render/texcache.h"

static float rnd(float v) { return floorf(v + 0.5f); }

SDL_FRect xf_rect(const xform *t, float x, float y, float w, float h)
{
   SDL_FRect r;
   float x0 = rnd(t->ox + x * t->s), y0 = rnd(t->oy + y * t->s);
   float x1 = rnd(t->ox + (x + w) * t->s), y1 = rnd(t->oy + (y + h) * t->s);
   r.x = x0;
   r.y = y0;
   r.w = x1 - x0;
   r.h = y1 - y0;
   return r;
}

void dl_set_clip(dl_ctx *c, const xform *t, int cl, int ct, int cr, int cb)
{
   SDL_FRect f = xf_rect(t, (float)cl, (float)ct, (float)(cr - cl), (float)(cb - ct));
   SDL_Rect r, out;
   r.x = (int)f.x;
   r.y = (int)f.y;
   r.w = (int)f.w;
   r.h = (int)f.h;
   if (!SDL_GetRectIntersection(&r, &c->bounds, &out))
      out.w = out.h = 0;
   SDL_SetRenderClipRect(c->r, &out);
}

static void fill(dl_ctx *c, const xform *t, float x, float y, float w, float h, uint32_t rgb, float a)
{
   SDL_FRect r = xf_rect(t, x, y, w, h);
   SDL_SetRenderDrawBlendMode(c->r, a < 1.0f ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
   SDL_SetRenderDrawColor(c->r, (Uint8)(rgb >> 16), (Uint8)(rgb >> 8), (Uint8)rgb, (Uint8)(a * 255.0f + 0.5f));
   SDL_RenderFillRect(c->r, &r);
}

static void draw_tex(dl_ctx *c, SDL_Texture *tex, const SDL_FRect *src, SDL_FRect dst, int flip,
                     float angle, float pivot_x, float pivot_y, uint32_t mod_rgb, float a)
{
   if (!tex)
      return;
   texcache_filter(tex);
   SDL_SetTextureColorMod(tex, (Uint8)(mod_rgb >> 16), (Uint8)(mod_rgb >> 8), (Uint8)mod_rgb);
   SDL_SetTextureAlphaModFloat(tex, a);
   if (a < 1.0f)
      SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
   if (flip || angle != 0.0f) {
      SDL_FPoint pv;
      SDL_FlipMode fm = (SDL_FlipMode)((flip & 1 ? SDL_FLIP_HORIZONTAL : 0) | (flip & 2 ? SDL_FLIP_VERTICAL : 0));
      pv.x = pivot_x;
      pv.y = pivot_y;
      SDL_RenderTextureRotated(c->r, tex, src, &dst, angle, angle != 0.0f ? &pv : NULL, fm);
   } else {
      SDL_RenderTexture(c->r, tex, src, &dst);
   }
}

void dl_draw_text(dl_ctx *c, const xform *t, const FONT *f, const char *s, float x, float y,
                  int color_mode, uint32_t rgb, int32_t bg)
{
   const char *p = s;
   int ch;
   if (!f || !s)
      return;
   if (f->is_color) {
      if (color_mode < 0 && bg >= 0) {
         /* whole run background, then no per-glyph background */
         float w = (float)text_length(f, s);
         fill(c, t, x, y, w, (float)f->height, (uint32_t)bg, c->alpha);
         bg = -1;
      }
      while ((ch = a4_ugetx(&p)) != 0) {
         const A4_GLYPH *g = a4_font_glyph(f, ch);
         BITMAP *gb;
         SDL_FRect src, dst;
         if (!g || !g->bmp)
            continue;
         gb = g->bmp;
         src.x = 0; src.y = 0; src.w = (float)gb->w; src.h = (float)gb->h;
         dst = xf_rect(t, x, y + (float)((f->height - gb->h) / 2), (float)gb->w, (float)gb->h);
         if (gb->depth == 8 && color_mode >= 0) {
            if (bg >= 0)
               fill(c, t, x, y + (float)((f->height - gb->h) / 2), (float)gb->w, (float)gb->h, (uint32_t)bg, c->alpha);
            draw_tex(c, texcache_get(gb, TEXV_SILHOUETTE), &src, dst, 0, 0, 0, 0, rgb, c->alpha);
         } else {
            draw_tex(c, texcache_get(gb, TEXV_MASKED), &src, dst, 0, 0, 0, 0, 0xFFFFFFu, c->alpha);
         }
         x += (float)gb->w;
      }
   } else {
      uint32_t col = color_mode < 0 ? 0xFFFFFFu : rgb;
      while ((ch = a4_ugetx(&p)) != 0) {
         const A4_GLYPH *g = a4_font_glyph(f, ch);
         SDL_FRect src, dst;
         float gy;
         if (!g)
            continue;
         gy = y + (float)((f->height - g->h) / 2);
         if (bg >= 0)
            fill(c, t, x, gy, (float)g->w, (float)g->h, (uint32_t)bg, c->alpha);
         src.x = 0; src.y = 0; src.w = (float)g->w; src.h = (float)g->h;
         dst = xf_rect(t, x, gy, (float)g->w, (float)g->h);
         draw_tex(c, texcache_glyph(g), &src, dst, 0, 0, 0, 0, col, c->alpha);
         x += (float)g->w;
      }
   }
}

static void line_op(dl_ctx *c, const xform *t, const a4_dl_op *op, float a)
{
   int x1 = op->x1, y1 = op->y1, x2 = op->x2, y2 = op->y2;
   if (y1 == y2) {
      if (x2 < x1) { int k = x1; x1 = x2; x2 = k; }
      fill(c, t, (float)x1, (float)y1, (float)(x2 - x1 + 1), 1.0f, op->rgb, a);
   } else if (x1 == x2) {
      if (y2 < y1) { int k = y1; y1 = y2; y2 = k; }
      fill(c, t, (float)x1, (float)y1, 1.0f, (float)(y2 - y1 + 1), op->rgb, a);
   } else {
      /* Bresenham in canvas pixels (debug lines only) */
      int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
      int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
      int err = dx + dy;
      for (;;) {
         fill(c, t, (float)x1, (float)y1, 1.0f, 1.0f, op->rgb, a);
         if (x1 == x2 && y1 == y2)
            break;
         {
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x1 += sx; }
            if (e2 <= dx) { err += dx; y1 += sy; }
         }
      }
   }
}

int dl_render(dl_ctx *c, const A4_DL *dl)
{
   int i;
   xform t;
   if (!dl || !dl->valid)
      return 0;
   t = c->t;
   t.ox += (float)dl->shift_x * t.s;
   t.oy += (float)dl->shift_y * t.s;
   for (i = 0; i < dl->n; i++) {
      const a4_dl_op *op = &dl->ops[i];
      float a = c->alpha;
      if (op->kind != DLOP_UNDERLAY)
         dl_set_clip(c, &t, op->cl, op->ct, op->cr, op->cb);
      switch (op->kind) {
         case DLOP_BITMAP: {
            SDL_FRect src, dst;
            tex_variant v = op->blend == DLB_SOLID ? TEXV_OPAQUE :
                            op->blend == DLB_ALPHA ? TEXV_ALPHA : TEXV_MASKED;
            if (op->blend == DLB_TRANS)
               a *= (float)op->alpha / 255.0f;
            src.x = (float)op->sx; src.y = (float)op->sy; src.w = (float)op->sw; src.h = (float)op->sh;
            dst = xf_rect(&t, op->dx, op->dy, op->dw, op->dh);
            draw_tex(c, texcache_get(op->src, v), &src, dst, op->flip, op->angle,
                     (op->px - op->dx) * t.s, (op->py - op->dy) * t.s, 0xFFFFFFu, a);
            break;
         }
         case DLOP_FILL:
            if (c->masked && op->rgb == 0xFF00FFu)
               break;
            if (op->blend == DLB_TRANS)
               a *= (float)op->alpha / 255.0f;
            fill(c, &t, (float)op->x1, (float)op->y1, (float)(op->x2 - op->x1 + 1),
                 (float)(op->y2 - op->y1 + 1), op->rgb, a);
            break;
         case DLOP_LINE:
            if (op->blend == DLB_TRANS)
               a *= (float)op->alpha / 255.0f;
            line_op(c, &t, op, a);
            break;
         case DLOP_TEXT:
            dl_draw_text(c, &t, op->font, op->text, (float)op->x1, (float)op->y1,
                         op->text_color_mode, op->rgb, op->text_bg);
            break;
         case DLOP_NESTED: {
            dl_ctx sub = *c;
            SDL_FRect r = xf_rect(&t, op->dx, op->dy, op->dw, op->dh);
            SDL_Rect cr, bound;
            SDL_Rect clip;
            SDL_FRect cf = xf_rect(&t, (float)op->cl, (float)op->ct, (float)(op->cr - op->cl), (float)(op->cb - op->ct));
            clip.x = (int)cf.x; clip.y = (int)cf.y; clip.w = (int)cf.w; clip.h = (int)cf.h;
            cr.x = (int)r.x; cr.y = (int)r.y; cr.w = (int)r.w; cr.h = (int)r.h;
            if (!SDL_GetRectIntersection(&cr, &clip, &bound) || !SDL_GetRectIntersection(&bound, &c->bounds, &sub.bounds))
               break;
            sub.t.ox = t.ox + (op->dx - (float)op->sx) * t.s;
            sub.t.oy = t.oy + (op->dy - (float)op->sy) * t.s;
            sub.t.s = t.s;
            if (op->blend == DLB_TRANS)
               sub.alpha = a * (float)op->alpha / 255.0f;
            if (op->blend != DLB_SOLID)
               sub.masked = 1;
            if (!dl_render(&sub, op->child)) {
               /* the nested canvas has no list: its pixels are not
                * recoverable here any more; leave the area untouched */
            }
            break;
         }
         case DLOP_UNDERLAY:
            SDL_SetRenderClipRect(c->r, NULL);
            if (c->underlay)
               c->underlay(c, &t, op->underlay);
            break;
         default:
            break;
      }
   }
   SDL_SetRenderClipRect(c->r, NULL);
   return 1;
}

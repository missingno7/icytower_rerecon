/*
 * Draws the touch controls (port/input/touch.h) over the presented frame:
 * translucent discs with simple icons, brighter while pressed.  Sizes come
 * from the touch layout, which scales with the output height.
 */
#include <math.h>
#include <SDL3/SDL.h>
#include "port/input/touch.h"
#include "port/render/touch_overlay.h"

#define SEGS 40

static void disc(SDL_Renderer *r, float cx, float cy, float rad, SDL_FColor c)
{
   SDL_Vertex v[SEGS + 2];
   int idx[SEGS * 3], i;
   v[0].position.x = cx; v[0].position.y = cy; v[0].color = c;
   for (i = 0; i <= SEGS; i++) {
      float a = (float)i * 6.2831853f / SEGS;
      v[i + 1].position.x = cx + cosf(a) * rad;
      v[i + 1].position.y = cy + sinf(a) * rad;
      v[i + 1].color = c;
   }
   for (i = 0; i < SEGS; i++) {
      idx[i * 3] = 0; idx[i * 3 + 1] = i + 1; idx[i * 3 + 2] = i + 2;
   }
   SDL_RenderGeometry(r, NULL, v, SEGS + 2, idx, SEGS * 3);
}

static void ring(SDL_Renderer *r, float cx, float cy, float rad, float w, SDL_FColor c)
{
   SDL_Vertex v[(SEGS + 1) * 2];
   int idx[SEGS * 6], i;
   for (i = 0; i <= SEGS; i++) {
      float a = (float)i * 6.2831853f / SEGS, ca = cosf(a), sa = sinf(a);
      v[i * 2].position.x = cx + ca * rad; v[i * 2].position.y = cy + sa * rad;
      v[i * 2 + 1].position.x = cx + ca * (rad - w); v[i * 2 + 1].position.y = cy + sa * (rad - w);
      v[i * 2].color = v[i * 2 + 1].color = c;
   }
   for (i = 0; i < SEGS; i++) {
      int b = i * 2;
      idx[i * 6] = b; idx[i * 6 + 1] = b + 1; idx[i * 6 + 2] = b + 2;
      idx[i * 6 + 3] = b + 1; idx[i * 6 + 4] = b + 3; idx[i * 6 + 5] = b + 2;
   }
   SDL_RenderGeometry(r, NULL, v, (SEGS + 1) * 2, idx, SEGS * 6);
}

static void tri(SDL_Renderer *r, float x1, float y1, float x2, float y2, float x3, float y3, SDL_FColor c)
{
   SDL_Vertex v[3];
   v[0].position.x = x1; v[0].position.y = y1;
   v[1].position.x = x2; v[1].position.y = y2;
   v[2].position.x = x3; v[2].position.y = y3;
   v[0].color = v[1].color = v[2].color = c;
   SDL_RenderGeometry(r, NULL, v, 3, NULL, 0);
}

static void thick(SDL_Renderer *r, float x1, float y1, float x2, float y2, float w, SDL_FColor c)
{
   float dx = x2 - x1, dy = y2 - y1, l = sqrtf(dx * dx + dy * dy), nx, ny;
   SDL_Vertex v[4];
   int idx[6] = { 0, 1, 2, 1, 3, 2 };
   if (l <= 0.0f)
      return;
   nx = -dy / l * w * 0.5f;
   ny = dx / l * w * 0.5f;
   v[0].position.x = x1 + nx; v[0].position.y = y1 + ny;
   v[1].position.x = x1 - nx; v[1].position.y = y1 - ny;
   v[2].position.x = x2 + nx; v[2].position.y = y2 + ny;
   v[3].position.x = x2 - nx; v[3].position.y = y2 - ny;
   v[0].color = v[1].color = v[2].color = v[3].color = c;
   SDL_RenderGeometry(r, NULL, v, 4, idx, 6);
}

/* arrow head pointing in (dx, dy) */
static void arrow(SDL_Renderer *r, float cx, float cy, float s, float dx, float dy, SDL_FColor c)
{
   float px = -dy, py = dx;
   tri(r, cx + dx * s, cy + dy * s, cx - dx * s * 0.6f + px * s * 0.8f, cy - dy * s * 0.6f + py * s * 0.8f,
       cx - dx * s * 0.6f - px * s * 0.8f, cy - dy * s * 0.6f - py * s * 0.8f, c);
}

static SDL_FColor col(float r, float g, float b, float a)
{
   SDL_FColor c;
   c.r = r; c.g = g; c.b = b; c.a = a < 0.0f ? 0.0f : a > 1.0f ? 1.0f : a;
   return c;
}

static void widget(SDL_Renderer *r, const touch_view *tv, const touch_widget *w)
{
   float a = tv->opacity * (w->pressed ? 1.9f : 1.0f);
   float x = w->x, y = w->y, rad = w->r, s = rad * 0.42f, lw = rad * 0.14f;
   SDL_FColor fill = col(0.0f, 0.0f, 0.05f, a * 0.55f);
   SDL_FColor line = col(1.0f, 1.0f, 1.0f, a * 1.6f);
   if (w->id == TB_TILT) {
      /* direction indicator only */
      disc(r, x, y, rad, fill);
      if (tv->tilt_dir)
         arrow(r, x, y, s * 1.4f, (float)tv->tilt_dir, 0.0f, line);
      else
         disc(r, x, y, rad * 0.25f, line);
      return;
   }
   disc(r, x, y, rad, fill);
   ring(r, x, y, rad, lw * 0.6f, line);
   switch (w->id) {
      case TB_LEFT: case TB_KLEFT: arrow(r, x, y, s, -1.0f, 0.0f, line); break;
      case TB_RIGHT: case TB_KRIGHT: arrow(r, x, y, s, 1.0f, 0.0f, line); break;
      case TB_UP: arrow(r, x, y, s, 0.0f, -1.0f, line); break;
      case TB_DOWN: arrow(r, x, y, s, 0.0f, 1.0f, line); break;
      case TB_JUMP:   /* double chevron up */
         thick(r, x - s, y + s * 0.2f, x, y - s * 0.8f, lw, line);
         thick(r, x, y - s * 0.8f, x + s, y + s * 0.2f, lw, line);
         thick(r, x - s, y + s * 0.9f, x, y - s * 0.1f, lw, line);
         thick(r, x, y - s * 0.1f, x + s, y + s * 0.9f, lw, line);
         break;
      case TB_PAUSE: {
         SDL_FRect b1, b2;
         b1.x = x - s * 0.7f; b1.y = y - s * 0.8f; b1.w = s * 0.45f; b1.h = s * 1.6f;
         b2 = b1; b2.x = x + s * 0.25f;
         SDL_SetRenderDrawColorFloat(r, line.r, line.g, line.b, line.a);
         SDL_RenderFillRect(r, &b1);
         SDL_RenderFillRect(r, &b2);
         break;
      }
      case TB_OK:     /* check mark */
         thick(r, x - s * 0.9f, y, x - s * 0.2f, y + s * 0.7f, lw, line);
         thick(r, x - s * 0.2f, y + s * 0.7f, x + s, y - s * 0.7f, lw, line);
         break;
      case TB_BACK:   /* arrow left */
         thick(r, x - s * 0.3f, y, x + s * 0.9f, y, lw, line);
         arrow(r, x - s * 0.4f, y, s * 0.6f, -1.0f, 0.0f, line);
         break;
      case TB_SLIDE:  /* <-> hint */
         arrow(r, x - s * 1.2f, y, s * 0.6f, -1.0f, 0.0f, line);
         arrow(r, x + s * 1.2f, y, s * 0.6f, 1.0f, 0.0f, line);
         disc(r, x, y, s * 0.35f, line);
         break;
      default:
         break;
   }
}

void touch_overlay_draw(SDL_Renderer *r)
{
   touch_view tv;
   int w = 0, h = 0, i;
   if (!SDL_GetCurrentRenderOutputSize(r, &w, &h) || w <= 0 || h <= 0)
      return;
   touch_get_view(w, h, &tv);
   if (!tv.visible)
      return;
   SDL_SetRenderClipRect(r, NULL);
   SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
   for (i = 0; i < tv.count; i++)
      widget(r, &tv, &tv.w[i]);
   if (tv.slide_anchor_x >= 0.0f) {
      /* the active drag: anchor ring and thumb, joined */
      float u = (float)h / 100.0f;
      SDL_FColor line = col(1.0f, 1.0f, 1.0f, tv.opacity * 2.0f);
      ring(r, tv.slide_anchor_x, tv.slide_y, 4.0f * u, 0.6f * u, line);
      thick(r, tv.slide_anchor_x, tv.slide_y, tv.slide_thumb_x, tv.slide_y, 0.8f * u, line);
      disc(r, tv.slide_thumb_x, tv.slide_y, 2.5f * u, line);
   }
}

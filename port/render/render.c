/*
 * Render orchestration and frame pacing.
 */
#include <SDL3/SDL.h>
#include "port/render/render.h"
#include "port/render/modern.h"
#include "port/render/capture.h"
#include "port/platform/platform.h"
#include "port/input/touch.h"
#include "port/input/input.h"
#include "port/render/touch_overlay.h"

#ifdef SDL_PLATFORM_ANDROID
#define DEFAULT_WINMODE WINMODE_BORDERLESS   /* the app always covers the screen */
#else
#define DEFAULT_WINMODE WINMODE_WINDOWED
#endif

static render_config g_cfg = {
   DEFAULT_WINMODE, 1280, 720, true, true, true, 0, false,
   true, true, FILTER_NEAREST, true, 0.0f, false
};
static struct SDL_Renderer *g_ren;
static SDL_Texture *g_lift_tex;   /* text_lift_px() */
static uint64_t g_last_frame_ns;
static uint64_t g_frames;

const render_config *render_get_config(void) { return &g_cfg; }
render_config *render_config_mut(void) { return &g_cfg; }

int render_scale_mode(void)
{
   switch (g_cfg.filter) {
      case FILTER_LINEAR: return SDL_SCALEMODE_LINEAR;
      case FILTER_PIXELART: return SDL_SCALEMODE_PIXELART;
      default: return SDL_SCALEMODE_NEAREST;
   }
}

void a4_dl_enable(int on);

void render_on_open(struct SDL_Renderer *r)
{
   g_ren = r;
   /* display lists are only needed by the modern renderer */
   a4_dl_enable(g_cfg.modern);
   modern_on_open(r);
}

void render_on_close(void)
{
   if (g_lift_tex) SDL_DestroyTexture(g_lift_tex);
   g_lift_tex = NULL;
   modern_on_close();
   g_ren = NULL;
}

bool render_frame_due(bool dirty)
{
   uint64_t now = plat_ticks_ns();
   bool animating = g_cfg.modern && modern_scene_active() && g_cfg.interpolation;
   if (capture_pending())
      return true;
   if (touch_take_dirty())
      dirty = true;   /* a touch control changed (press highlight) */
   if (!dirty && !animating)
      return false;
   if (g_cfg.max_fps > 0) {
      uint64_t min_dt = 1000000000ull / (uint64_t)g_cfg.max_fps;
      if (now - g_last_frame_ns < min_dt)
         return false;
   } else if (!g_cfg.vsync && animating) {
      /* uncapped without vsync: still avoid spinning faster than 1000 fps */
      if (now - g_last_frame_ns < 1000000ull)
         return false;
   }
   return true;
}

/* While a soft keyboard covers the lower part of the screen (phones), the
 * frame is drawn lifted so the text field being typed into stays visible:
 * rendered offscreen, then presented shifted up.  Presentation only. */
static float text_lift_px(int w, int h)
{
   int y;
   float field_bottom;
   (void)w;
   if (!input_text_field_covered(&y))
      return 0.0f;
   field_bottom = (float)(y + 24) / 480.0f * (float)h;   /* the canvas spans the height */
   return field_bottom > 0.30f * (float)h ? field_bottom - 0.30f * (float)h : 0.0f;   /* keyboards cover up to ~65% */
}

void render_frame(void)
{
   int w = 0, h = 0;
   float lift;
   if (!g_ren)
      return;
   SDL_GetCurrentRenderOutputSize(g_ren, &w, &h);
   lift = text_lift_px(w, h);
   if (lift > 0.0f) {
      float tw = 0, th = 0;
      if (g_lift_tex)
         SDL_GetTextureSize(g_lift_tex, &tw, &th);
      if (!g_lift_tex || (int)tw != w || (int)th != h) {
         if (g_lift_tex) SDL_DestroyTexture(g_lift_tex);
         g_lift_tex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
      }
      if (!g_lift_tex || !SDL_SetRenderTarget(g_ren, g_lift_tex))
         lift = 0.0f;
   }
   SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
   SDL_RenderClear(g_ren);
   if (!(g_cfg.modern && modern_draw_frame(g_ren)))
      present_draw_canvas(NULL);
   if (lift > 0.0f) {
      SDL_FRect dst;
      SDL_SetRenderTarget(g_ren, NULL);
      SDL_SetRenderClipRect(g_ren, NULL);
      SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
      SDL_RenderClear(g_ren);
      dst.x = 0; dst.y = -lift; dst.w = (float)w; dst.h = (float)h;
      SDL_RenderTexture(g_ren, g_lift_tex, NULL, &dst);
   }
   touch_overlay_draw(g_ren);
   capture_frame(g_ren);
   SDL_RenderPresent(g_ren);
   g_last_frame_ns = plat_ticks_ns();
   g_frames++;
}

uint64_t render_frames_presented(void) { return g_frames; }
uint64_t render_last_frame_ns(void) { return g_last_frame_ns; }

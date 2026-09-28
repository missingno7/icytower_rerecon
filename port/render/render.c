/*
 * Render orchestration and frame pacing.
 */
#include <SDL3/SDL.h>
#include "port/render/render.h"
#include "port/render/modern.h"
#include "port/render/capture.h"
#include "port/platform/platform.h"

static render_config g_cfg = {
   WINMODE_WINDOWED, 1280, 720, true, true, true, 0, false,
   true, true, FILTER_NEAREST, true, 0.0f, false
};
static struct SDL_Renderer *g_ren;
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
   modern_on_close();
   g_ren = NULL;
}

bool render_frame_due(bool dirty)
{
   uint64_t now = plat_ticks_ns();
   bool animating = g_cfg.modern && modern_scene_active() && g_cfg.interpolation;
   if (capture_pending())
      return true;
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

void render_frame(void)
{
   if (!g_ren)
      return;
   SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
   SDL_RenderClear(g_ren);
   if (!(g_cfg.modern && modern_draw_frame(g_ren)))
      present_draw_canvas(NULL);
   capture_frame(g_ren);
   SDL_RenderPresent(g_ren);
   g_last_frame_ns = plat_ticks_ns();
   g_frames++;
}

uint64_t render_frames_presented(void) { return g_frames; }
uint64_t render_last_frame_ns(void) { return g_last_frame_ns; }

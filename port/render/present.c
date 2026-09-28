/*
 * Window, SDL renderer and presentation of the legacy canvas.
 */
#include <string.h>
#include "port/render/present.h"
#include "port/render/render.h"
#include "port/platform/plat_internal.h"
#include "a4_internal.h"

static SDL_Window *g_win;
static SDL_Renderer *g_ren;
static SDL_Texture *g_canvas_tex;
static BITMAP *g_canvas;
static int g_canvas_w = 640, g_canvas_h = 480;
static uint32_t g_uploaded_gen = 0xFFFFFFFFu, g_uploaded_serial;
static bool g_need_repaint = true;
static SDL_FRect g_canvas_dst;
static char g_title[256] = "Icy Tower";
static bool g_handler_installed;

static void on_event(const SDL_Event *ev)
{
   switch (ev->type) {
      case SDL_EVENT_WINDOW_EXPOSED:
      case SDL_EVENT_WINDOW_RESIZED:
      case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
      case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
      case SDL_EVENT_WINDOW_RESTORED:
      case SDL_EVENT_WINDOW_MAXIMIZED:
      case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
      case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
         g_need_repaint = true;
         break;
      default:
         break;
   }
}

static void apply_window_mode(bool fullscreen_request)
{
   const render_config *rc = render_get_config();
   bool fs = fullscreen_request;
   if (!g_win)
      return;
   if (fs) {
      /* borderless desktop fullscreen by default; exclusive mode only when
       * explicitly configured (SDL picks the closest display mode). */
      if (rc->window_mode == WINMODE_FULLSCREEN && rc->exclusive_fullscreen) {
         SDL_DisplayMode mode;
         SDL_DisplayID d = SDL_GetDisplayForWindow(g_win);
         if (SDL_GetClosestFullscreenDisplayMode(d, rc->width, rc->height, 0.0f, true, &mode))
            SDL_SetWindowFullscreenMode(g_win, &mode);
      } else {
         SDL_SetWindowFullscreenMode(g_win, NULL);
      }
      SDL_SetWindowFullscreen(g_win, true);
   } else {
      SDL_SetWindowFullscreen(g_win, false);
   }
   g_need_repaint = true;
}

bool present_open(int canvas_w, int canvas_h, bool fullscreen)
{
   const render_config *rc = render_get_config();
   SDL_WindowFlags flags = 0;
   if (g_win)
      return true;
   g_canvas_w = canvas_w;
   g_canvas_h = canvas_h;
   if (rc->resizable)
      flags |= SDL_WINDOW_RESIZABLE;
   if (rc->high_dpi)
      flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
   g_win = SDL_CreateWindow(g_title, rc->width, rc->height, flags | SDL_WINDOW_HIDDEN);
   if (!g_win)
      return false;
   SDL_SetWindowMinimumSize(g_win, 320, 240);
   g_ren = SDL_CreateRenderer(g_win, NULL);
   if (!g_ren) {
      SDL_DestroyWindow(g_win);
      g_win = NULL;
      return false;
   }
   SDL_SetRenderVSync(g_ren, rc->vsync ? 1 : SDL_RENDERER_VSYNC_DISABLED);
   g_canvas_tex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING,
                                    canvas_w, canvas_h);
   if (!g_handler_installed) {
      plat_add_event_handler(on_event);
      g_handler_installed = true;
   }
   apply_window_mode(fullscreen);
   SDL_ShowWindow(g_win);
   render_on_open(g_ren);
   g_need_repaint = true;
   return true;
}

void present_close(void)
{
   render_on_close();
   if (g_canvas_tex) SDL_DestroyTexture(g_canvas_tex);
   if (g_ren) SDL_DestroyRenderer(g_ren);
   if (g_win) SDL_DestroyWindow(g_win);
   g_canvas_tex = NULL;
   g_ren = NULL;
   g_win = NULL;
}

bool present_is_open(void) { return g_win != NULL; }

void present_set_title(const char *title)
{
   SDL_strlcpy(g_title, title ? title : "", sizeof(g_title));
   if (g_win)
      SDL_SetWindowTitle(g_win, g_title);
}

void present_set_fullscreen(bool fullscreen)
{
   apply_window_mode(fullscreen);
}

void present_set_canvas(BITMAP *canvas)
{
   g_canvas = canvas;
   g_uploaded_gen = 0xFFFFFFFFu;
   g_need_repaint = true;
}

SDL_Renderer *present_renderer(void) { return g_ren; }
SDL_Window *present_window(void) { return g_win; }

void present_output_size(int *w, int *h)
{
   int ow = g_canvas_w, oh = g_canvas_h;
   if (g_ren)
      SDL_GetCurrentRenderOutputSize(g_ren, &ow, &oh);
   if (w) *w = ow;
   if (h) *h = oh;
}

/* Upload the legacy canvas if it changed.  Returns the texture. */
SDL_Texture *present_canvas_texture(void)
{
   if (!g_canvas || !g_canvas_tex)
      return g_canvas_tex;
   if (g_canvas->generation != g_uploaded_gen || g_canvas->serial != g_uploaded_serial) {
      if (g_canvas->depth == 32) {
         SDL_UpdateTexture(g_canvas_tex, NULL, g_canvas->line[0], g_canvas->pitch);
      } else {
         /* other depths: convert row by row */
         void *pixels;
         int pitch, x, y;
         if (SDL_LockTexture(g_canvas_tex, NULL, &pixels, &pitch)) {
            for (y = 0; y < g_canvas->h && y < g_canvas_h; y++) {
               uint32_t *row = (uint32_t *)((uint8_t *)pixels + (size_t)y * pitch);
               for (x = 0; x < g_canvas->w && x < g_canvas_w; x++)
                  row[x] = (uint32_t)a4_convert_color(a4_get_raw(g_canvas, x, y), g_canvas->depth, 32);
            }
            SDL_UnlockTexture(g_canvas_tex);
         }
      }
      g_uploaded_gen = g_canvas->generation;
      g_uploaded_serial = g_canvas->serial;
      g_need_repaint = true;
   }
   return g_canvas_tex;
}

/* aspect-correct placement of a w x h canvas in the output */
void present_fit_rect(int cw, int ch, int ow, int oh, SDL_FRect *dst)
{
   float s = (float)ow / (float)cw;
   if ((float)oh / (float)ch < s)
      s = (float)oh / (float)ch;
   if (render_get_config()->integer_scaling && s >= 1.0f)
      s = (float)(int)s;
   dst->w = (float)cw * s;
   dst->h = (float)ch * s;
   dst->x = (float)(int)(((float)ow - dst->w) * 0.5f);
   dst->y = (float)(int)(((float)oh - dst->h) * 0.5f);
}

void present_draw_canvas(SDL_FRect *dst_out)
{
   int ow, oh;
   SDL_FRect dst;
   SDL_Texture *t = present_canvas_texture();
   present_output_size(&ow, &oh);
   present_fit_rect(g_canvas_w, g_canvas_h, ow, oh, &dst);
   g_canvas_dst = dst;
   if (t) {
      SDL_SetTextureScaleMode(t, render_scale_mode());
      SDL_RenderTexture(g_ren, t, NULL, &dst);
   }
   if (dst_out)
      *dst_out = dst;
}

static bool canvas_changed(void)
{
   return g_canvas && (g_canvas->generation != g_uploaded_gen || g_canvas->serial != g_uploaded_serial);
}

bool present_service(bool force)
{
   if (!g_ren)
      return false;
   if (!render_frame_due(force || canvas_changed() || g_need_repaint))
      return false;
   g_need_repaint = false;
   render_frame();
   return true;
}

void present_service_idle(void)
{
   static uint64_t last_check;
   uint64_t now;
   if (!g_ren || !(canvas_changed() || g_need_repaint))
      return;
   now = SDL_GetTicksNS();
   if (now - last_check < 50000000ull)
      return;
   last_check = now;
   if (now - render_last_frame_ns() < 50000000ull)
      return;
   g_need_repaint = false;
   render_frame();
}

void present_window_to_canvas(float wx, float wy, int *cx, int *cy)
{
   float rx = wx, ry = wy;
   if (g_ren)
      SDL_RenderCoordinatesFromWindow(g_ren, wx, wy, &rx, &ry);
   if (g_canvas_dst.w > 0 && g_canvas_dst.h > 0) {
      rx = (rx - g_canvas_dst.x) * (float)g_canvas_w / g_canvas_dst.w;
      ry = (ry - g_canvas_dst.y) * (float)g_canvas_h / g_canvas_dst.h;
   }
   if (cx) *cx = (int)rx;
   if (cy) *cy = (int)ry;
}

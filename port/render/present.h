/*
 * Presentation: the OS window, the SDL renderer and the mapping from the
 * legacy 640x480 canvas (Allegro `screen`) to physical pixels.
 *
 * The presenter knows the physical output size; game code never does.  Two
 * render paths exist (see docs/port/ARCHITECTURE.md):
 *   faithful  the historical 640x480 frame, uploaded once per change and
 *             scaled to the window (aspect-correct, letter/pillar-boxed);
 *   modern    port/render/modern.c draws the world and HUD directly at
 *             output resolution from simulation snapshots; non-converted
 *             screens (menus) still show the legacy canvas, mapped by the UI
 *             transform.
 */
#ifndef PORT_PRESENT_H
#define PORT_PRESENT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct BITMAP;
struct SDL_Renderer;
struct SDL_Window;

typedef enum present_filter { FILTER_NEAREST = 0, FILTER_LINEAR = 1, FILTER_PIXELART = 2 } present_filter;
typedef enum present_window_mode { WINMODE_WINDOWED = 0, WINMODE_FULLSCREEN = 1, WINMODE_BORDERLESS = 2 } present_window_mode;

/* open/close the window; canvas is the legacy logical size (640x480) */
bool present_open(int canvas_w, int canvas_h, bool fullscreen);
void present_close(void);
bool present_is_open(void);
void present_set_title(const char *title);
void present_set_fullscreen(bool fullscreen);

/* legacy canvas: `screen` bitmap presented by the faithful path */
void present_set_canvas(struct BITMAP *canvas);
/* Called from waits (scheduler, rest, vsync): produces a frame when the
 * canvas changed, the window needs repainting or the modern renderer is
 * animating, subject to pacing.  `force` presents unconditionally.
 * Returns true when a frame was presented. */
bool present_service(bool force);
/* Called from input polling inside game logic: only repaints a changed
 * canvas that has not been shown for a while (keeps busy-wait screens
 * alive) and never renders animation frames mid-tick. */
void present_service_idle(void);

/* map window coordinates (SDL mouse units) to canvas coordinates */
void present_window_to_canvas(float wx, float wy, int *cx, int *cy);

/* access for the modern renderer */
struct SDL_Renderer *present_renderer(void);
struct SDL_Window *present_window(void);
void present_output_size(int *w, int *h);   /* drawable pixels */

#ifdef __cplusplus
}
#endif

#endif

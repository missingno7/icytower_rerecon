/*
 * Render orchestration: configuration, frame pacing and the choice between
 * the faithful path (legacy canvas) and the modern path (scene snapshots
 * drawn at output resolution).  See docs/port/ARCHITECTURE.md.
 */
#ifndef PORT_RENDER_H
#define PORT_RENDER_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL_rect.h>
#include "port/render/present.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SDL_Renderer;

typedef struct render_config {
   /* [display] */
   present_window_mode window_mode;
   int  width, height;          /* initial window size (logical units) */
   bool resizable;
   bool high_dpi;
   bool vsync;
   int  max_fps;                /* 0 = uncapped (vsync still applies) */
   bool exclusive_fullscreen;   /* fullscreen: change display mode instead of borderless */
   /* [render] */
   bool modern;                 /* modern renderer vs faithful 640x480 canvas */
   bool widescreen;             /* modern: widen the camera to the output aspect */
   present_filter filter;       /* texture sampling */
   bool interpolation;          /* render between simulation ticks */
   float ui_scale;              /* 0 = automatic */
   bool integer_scaling;        /* faithful/canvas: integer multiples only */
} render_config;

const render_config *render_get_config(void);
render_config *render_config_mut(void);
/* SDL_ScaleMode for the configured filter */
int render_scale_mode(void);

void render_on_open(struct SDL_Renderer *r);
void render_on_close(void);
/* true when a frame should be produced now (pacing, max_fps, dirtiness) */
bool render_frame_due(bool dirty);
/* produce and present one frame */
void render_frame(void);
/* number of frames presented so far (tests, stats) */
uint64_t render_frames_presented(void);
uint64_t render_last_frame_ns(void);

/* helpers shared by the renderers (present.c) */
struct SDL_Texture *present_canvas_texture(void);
void present_draw_canvas(SDL_FRect *dst_out);
void present_fit_rect(int cw, int ch, int ow, int oh, SDL_FRect *dst);

#ifdef __cplusplus
}
#endif

#endif

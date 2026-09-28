/*
 * Modern renderer: draws the game world and HUD directly at output
 * resolution from simulation snapshots (port/sim/snapshot.h), with a
 * widescreen camera, anchored UI and optional interpolation.
 */
#ifndef PORT_MODERN_H
#define PORT_MODERN_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SDL_Renderer;

void modern_on_open(struct SDL_Renderer *r);
void modern_on_close(void);
/* true while a snapshot-driven scene (gameplay) is being shown */
bool modern_scene_active(void);
/* draw one frame; returns false when the caller should draw the legacy
 * canvas instead (no scene active) */
bool modern_draw_frame(struct SDL_Renderer *r);

#ifdef __cplusplus
}
#endif

#endif

/*
 * Modern renderer (Phase 3+).  Placeholder until the snapshot pipeline is in
 * place: reports no active scene so the legacy canvas is shown.
 */
#include "port/render/modern.h"

void modern_on_open(struct SDL_Renderer *r) { (void)r; }
void modern_on_close(void) { }
bool modern_scene_active(void) { return false; }
bool modern_draw_frame(struct SDL_Renderer *r) { (void)r; return false; }

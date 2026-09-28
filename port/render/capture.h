/*
 * Debug/validation capture: save presented frames as PNG and quit
 * automatically, so rendering can be checked at any resolution without a
 * person at the keyboard.
 *
 *   --capture DIR            output directory (created)
 *   --capture-ms LIST        capture the first frame presented after each
 *                            listed wall-clock time (ms since start), e.g. 3000,8000
 *   --capture-ticks LIST     same, in simulation ticks (after the scheduler
 *                            is active)
 *   --exit-after-ms N        request quit after N ms
 *   --exit-after-ticks N     request quit after N simulation ticks
 */
#ifndef PORT_CAPTURE_H
#define PORT_CAPTURE_H

#include <stdbool.h>
#include <stdint.h>

struct SDL_Renderer;

void capture_configure(int argc, char **argv);
/* called by the renderer right before SDL_RenderPresent */
void capture_frame(struct SDL_Renderer *r);
/* true once an exit condition fired (polled by the platform pump) */
bool capture_exit_requested(void);

#endif

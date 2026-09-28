/* Internal SDL-facing hooks shared by port/ subsystems (not for game code). */
#ifndef PORT_PLAT_INTERNAL_H
#define PORT_PLAT_INTERNAL_H

#include <SDL3/SDL.h>
#include "port/platform/platform.h"

/* Event consumers registered by subsystems (input, window/presenter).
 * Each is called for every polled event, in registration order. */
typedef void (*plat_event_fn)(const SDL_Event *ev);
void plat_add_event_handler(plat_event_fn fn);

#endif

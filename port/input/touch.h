/*
 * Touch controls (phones, tablets, touch screens).
 *
 * Gameplay (a gameplay frame was captured in the last ~150 ms) uses one of
 * the configurable schemes and produces the game's control flags (LEFT 1,
 * RIGHT 2, FIRE 16 = jump, PAUSE 64), exactly like a keyboard would, so
 * replays and the simulation are unaffected by the input device:
 *
 *   ZONES  left side: two zones, outer = left, inner = right (rock the thumb);
 *          right side: jump
 *   SLIDE  left side: drag left/right from where the thumb went down (the
 *          anchor follows, so reversing is quick); right side: jump
 *   TILT   tilt the device to run, tap anywhere to jump
 *   OFF    no touch controls (keyboard / gamepad)
 *
 * Everywhere else (menus, results, pause and exit prompts, replays) a small
 * remote is shown instead: arrows, OK and Back, sent as key presses so every
 * historical screen works as with a keyboard.  A pause button is always
 * available during gameplay.
 *
 * Every press lasts at least one simulation tick (short taps are latched).
 */
#ifndef PORT_TOUCH_H
#define PORT_TOUCH_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* values match the in-game "Touch" selection order */
enum { TOUCH_ZONES = 0, TOUCH_SLIDE, TOUCH_TILT, TOUCH_OFF, TOUCH_SCHEME_COUNT };

void touch_init(void);
void touch_set_scheme(int scheme);
int  touch_scheme(void);
const char *touch_scheme_name(int scheme);      /* "zones", "slide", ... */
int  touch_scheme_from_name(const char *name);  /* -1 if unknown */
void touch_set_opacity(float a);                /* 0..1, overlay strength */
void touch_set_left_handed(bool on);            /* mirror the layout */

/* game glue: a gameplay frame was just captured (replay = watching one) */
void touch_note_gameplay(bool replay);

/* control flags from touch for the game's Tcontrol (0 outside gameplay) */
int  touch_control_flags(void);

/* ---- overlay (drawn by port/render/touch_overlay.c) ------------------ */
typedef enum { TB_NONE, TB_LEFT, TB_RIGHT, TB_JUMP, TB_PAUSE, TB_SLIDE,
               TB_UP, TB_DOWN, TB_KLEFT, TB_KRIGHT, TB_OK, TB_BACK, TB_TILT, TB_COUNT } touch_button;
typedef struct touch_widget {
   touch_button id;
   float x, y, r;        /* centre and radius in output pixels */
   bool pressed;
} touch_widget;
typedef struct touch_view {
   bool visible;         /* a touch device is in use */
   bool gameplay;        /* gameplay layout, else the remote */
   int scheme;
   float opacity;
   int count;
   touch_widget w[12];
   /* SLIDE: anchor and thumb of the active drag (x < 0: none) */
   float slide_anchor_x, slide_thumb_x, slide_y;
   int tilt_dir;         /* TILT: -1, 0, 1 */
} touch_view;
/* layout for an output of w x h pixels (also used for hit tests) */
void touch_get_view(int w, int h, touch_view *out);
/* true when the overlay changed since the last call (redraw needed) */
bool touch_take_dirty(void);

#ifdef __cplusplus
}
#endif

#endif

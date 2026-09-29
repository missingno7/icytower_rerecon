/*
 * Portable input layer.
 *
 * Physical devices (keyboard, mouse, SDL gamepads with hot-plugging, later
 * touch) are collected here.  Two consumers read it:
 *   - the Allegro compatibility layer (key[], readkey, mouse_*, joy[]), which
 *     keeps the historical control code working unchanged;
 *   - the logical action layer (input_action_*), which new code uses instead
 *     of physical key codes.  Touch controls feed the same actions.
 */
#ifndef PORT_INPUT_H
#define PORT_INPUT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void input_init(void);
/* debug: scripted key presses "MS+CODE,MS-CODE,..." (Allegro scancodes) */
void input_set_script(const char *spec);

/* ---- keyboard, in Allegro 4.4.1 scancode space (KEY_A .. KEY_CAPSLOCK) */
#define INPUT_KEY_MAX 128
extern volatile char input_key[INPUT_KEY_MAX];
extern volatile int input_key_shifts;
/* key queue: entries are (allegro_scancode << 8) | ascii */
bool input_keybuf_empty(void);
int  input_keybuf_pop(void);
void input_keybuf_push(int scancode, int ascii);
void input_keybuf_clear(void);
void input_release_all_keys(void);
/* a key pressed/released by a virtual device (touch remote): same path as a
 * keyboard key, including the minimum hold time and the key buffer */
void input_virtual_key(int allegro_scancode, int ascii, bool down);
/* soft keyboard / text input while the game reads a typed string; the
 * field's top in 640x480 canvas coordinates lets the renderer keep it above
 * an on-screen keyboard */
void input_text_input(bool on, int canvas_y);
/* true while a soft keyboard covers the screen for a field at *canvas_y */
bool input_text_field_covered(int *canvas_y);

/* ---- mouse, in legacy 640x480 canvas coordinates */
extern volatile int input_mouse_x, input_mouse_y, input_mouse_b;
void input_set_cursor(int kind);    /* A4 MOUSE_CURSOR_* / A4_MOUSE_CURSOR_HAND */
void input_show_cursor(bool show);

/* ---- gamepad (first connected SDL gamepad; hot-plug aware) */
typedef struct input_pad_state {
   bool connected;
   int  num_buttons;          /* exposed Allegro-style buttons */
   bool button[32];
   bool left, right, up, down;/* digital direction from stick or d-pad */
   int  axis_x, axis_y;       /* -128..128 */
} input_pad_state;
const input_pad_state *input_pad(void);

/* ---- game control flags ------------------------------------------------
 * The game's Tcontrol flag byte (LEFT 1, RIGHT 2, UP 4, DOWN 8, FIRE 16,
 * ENTER 32, PAUSE 64) is produced here from every device: keyboard (the
 * player's key bindings, in Allegro scancodes, as stored in profiles),
 * gamepads (the gamepad.txt mapping; Start additionally pauses) and future
 * touch controls.  Replays record these flags once per simulation step, so
 * they are independent of device and refresh rate. */
typedef struct input_bindings {
   int key_left, key_right, key_up, key_down, key_fire, key_enter, key_pause;
   int use_pad;
   int pad_up, pad_down, pad_left, pad_right;   /* flag bits for directions */
   int pad_button[32];                          /* flag bits per button */
} input_bindings;
int input_control_flags(const input_bindings *b, int pad_only);
/* touch (and other virtual) controls OR their flags in here (Android) */
void input_set_virtual_flags(int flags);

/* ---- logical actions (for modernised code, replay-safe) */
typedef enum input_action {
   ACTION_LEFT, ACTION_RIGHT, ACTION_UP, ACTION_DOWN,
   ACTION_JUMP, ACTION_PAUSE, ACTION_CONFIRM, ACTION_BACK,
   ACTION_SCREENSHOT, ACTION_FULLSCREEN,
   ACTION_COUNT
} input_action;
bool input_action_down(input_action a);

#ifdef __cplusplus
}
#endif

#endif

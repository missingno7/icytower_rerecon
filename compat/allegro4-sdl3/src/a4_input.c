/*
 * Keyboard, mouse and joystick.
 * OWNER: integration (supervisor).  Temporary minimal version.
 */
#include "a4_internal.h"

static volatile char key_array[KEY_MAX];
volatile int key_shifts;
volatile int mouse_x, mouse_y, mouse_z, mouse_b;
JOYSTICK_INFO joy[MAX_JOYSTICKS];
int num_joysticks;

volatile char *a4_key_state(void) { return key_array; }
int install_keyboard(void) { return 0; }
void remove_keyboard(void) { }
int keypressed(void) { return 0; }
int readkey(void) { return 0; }
void clear_keybuf(void) { }
void simulate_keypress(int keycode) { (void)keycode; }
int install_mouse(void) { return 0; }
void show_mouse(BITMAP *bmp) { (void)bmp; }
void select_mouse_cursor(int cursor) { (void)cursor; }
void enable_hardware_cursor(void) { }
int install_joystick(int type) { (void)type; return -1; }
int poll_joystick(void) { return 0; }

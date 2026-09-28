/*
 * Keyboard, mouse and joystick API over port/input.
 *
 * Allegro 4 updated key[] and the key buffer from a background thread, so
 * the game busy-waits on them (`while (key[KEY_F1]);`).  Here every access
 * services the platform first (pumps SDL events on the main thread, runs due
 * timers, presents), rate limited to keep tight loops cheap.
 */
#include "a4_internal.h"
#include "port/input/input.h"
#include "port/platform/platform.h"

volatile int key_shifts;
volatile int mouse_x, mouse_y, mouse_z, mouse_b;
JOYSTICK_INFO joy[MAX_JOYSTICKS];
int num_joysticks;

static uint64_t last_service_ns;

static void service_throttled(void)
{
   uint64_t now = plat_ticks_ns();
   if (now - last_service_ns >= 500000u) {   /* 0.5 ms */
      last_service_ns = now;
      a4_service();
   }
}

volatile char *a4_key_state(void)
{
   service_throttled();
   key_shifts = input_key_shifts;
   return input_key;
}

int install_keyboard(void)
{
   input_init();
   return 0;
}

void remove_keyboard(void) { }

int keypressed(void)
{
   service_throttled();
   return !input_keybuf_empty();
}

int readkey(void)
{
   while (input_keybuf_empty()) {
      a4_service_wait();
      if (plat_quit_requested() || plat_headless())
         return 0;
      plat_sleep_ns(1000000u);
   }
   return input_keybuf_pop();
}

void clear_keybuf(void)
{
   a4_service();
   input_keybuf_clear();
}

void simulate_keypress(int keycode)
{
   input_keybuf_push(keycode >> 8, keycode & 0xFF);
}

/* ---------------------------------------------------------------- mouse */

void a4_mouse_refresh(void)
{
   mouse_x = input_mouse_x;
   mouse_y = input_mouse_y;
   mouse_b = input_mouse_b;
}

int install_mouse(void)
{
   input_init();
   return 3;
}

void show_mouse(BITMAP *bmp)
{
   input_show_cursor(bmp != NULL);
}

void select_mouse_cursor(int cursor)
{
   input_set_cursor(cursor);
}

void enable_hardware_cursor(void) { }

/* ---------------------------------------------------------------- joystick */

static void fill_joy(void)
{
   const input_pad_state *p = input_pad();
   JOYSTICK_INFO *j = &joy[0];
   int b;
   j->flags = p->connected ? 1 : 0;
   j->num_sticks = 1;
   j->num_buttons = p->num_buttons;
   j->stick[0].num_axis = 2;
   j->stick[0].name = "stick";
   j->stick[0].axis[0].pos = p->axis_x;
   j->stick[0].axis[0].d1 = p->left;
   j->stick[0].axis[0].d2 = p->right;
   j->stick[0].axis[0].name = "x";
   j->stick[0].axis[1].pos = p->axis_y;
   j->stick[0].axis[1].d1 = p->up;
   j->stick[0].axis[1].d2 = p->down;
   j->stick[0].axis[1].name = "y";
   for (b = 0; b < MAX_JOYSTICK_BUTTONS; b++) {
      j->button[b].b = b < p->num_buttons ? p->button[b] : 0;
      j->button[b].name = "button";
   }
}

int install_joystick(int type)
{
   (void)type;
   input_init();
   num_joysticks = 1;
   fill_joy();
   /* Always report success: the port supports gamepad hot-plugging, so the
    * game must keep polling even if no pad is connected at start-up. */
   return 0;
}

int poll_joystick(void)
{
   service_throttled();
   fill_joy();
   return 0;
}

/*
 * SDL3 input backend: keyboard (mapped to Allegro 4.4.1 scancodes), mouse,
 * gamepads (SDL gamepad API, hot-plug) and logical actions.
 */
#include <string.h>
#include "port/input/input.h"
#include "port/platform/plat_internal.h"
#include "port/render/present.h"

volatile char input_key[INPUT_KEY_MAX];
volatile int input_key_shifts;
volatile int input_mouse_x, input_mouse_y, input_mouse_b;

#define KEYBUF_SIZE 64
static int keybuf[KEYBUF_SIZE];
static int keybuf_head, keybuf_tail;

static SDL_Gamepad *g_pad;
static input_pad_state g_pad_state;
static SDL_Cursor *g_cursor_arrow, *g_cursor_hand;
static bool g_inited;

/* Allegro KEY_* values (compat/allegro4-sdl3/include/allegro.h) */
static int map_scancode(SDL_Scancode sc)
{
   if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z)
      return 1 + (sc - SDL_SCANCODE_A);
   if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_9)
      return 28 + (sc - SDL_SCANCODE_1);
   if (sc == SDL_SCANCODE_0)
      return 27;
   if (sc >= SDL_SCANCODE_KP_1 && sc <= SDL_SCANCODE_KP_9)
      return 38 + (sc - SDL_SCANCODE_KP_1);
   if (sc == SDL_SCANCODE_KP_0)
      return 37;
   if (sc >= SDL_SCANCODE_F1 && sc <= SDL_SCANCODE_F12)
      return 47 + (sc - SDL_SCANCODE_F1);
   switch (sc) {
      case SDL_SCANCODE_ESCAPE: return 59;
      case SDL_SCANCODE_GRAVE: return 60;
      case SDL_SCANCODE_MINUS: return 61;
      case SDL_SCANCODE_EQUALS: return 62;
      case SDL_SCANCODE_BACKSPACE: return 63;
      case SDL_SCANCODE_TAB: return 64;
      case SDL_SCANCODE_LEFTBRACKET: return 65;
      case SDL_SCANCODE_RIGHTBRACKET: return 66;
      case SDL_SCANCODE_RETURN: return 67;
      case SDL_SCANCODE_SEMICOLON: return 68;
      case SDL_SCANCODE_APOSTROPHE: return 69;
      case SDL_SCANCODE_BACKSLASH: return 70;
      case SDL_SCANCODE_NONUSBACKSLASH: return 71;
      case SDL_SCANCODE_COMMA: return 72;
      case SDL_SCANCODE_PERIOD: return 73;
      case SDL_SCANCODE_SLASH: return 74;
      case SDL_SCANCODE_SPACE: return 75;
      case SDL_SCANCODE_INSERT: return 76;
      case SDL_SCANCODE_DELETE: return 77;
      case SDL_SCANCODE_HOME: return 78;
      case SDL_SCANCODE_END: return 79;
      case SDL_SCANCODE_PAGEUP: return 80;
      case SDL_SCANCODE_PAGEDOWN: return 81;
      case SDL_SCANCODE_LEFT: return 82;
      case SDL_SCANCODE_RIGHT: return 83;
      case SDL_SCANCODE_UP: return 84;
      case SDL_SCANCODE_DOWN: return 85;
      case SDL_SCANCODE_KP_DIVIDE: return 86;
      case SDL_SCANCODE_KP_MULTIPLY: return 87;
      case SDL_SCANCODE_KP_MINUS: return 88;
      case SDL_SCANCODE_KP_PLUS: return 89;
      case SDL_SCANCODE_KP_PERIOD: return 90;
      case SDL_SCANCODE_KP_ENTER: return 91;
      case SDL_SCANCODE_PRINTSCREEN: return 92;
      case SDL_SCANCODE_PAUSE: return 93;
      case SDL_SCANCODE_INTERNATIONAL3: return 95;
      case SDL_SCANCODE_INTERNATIONAL2: return 96;
      case SDL_SCANCODE_LSHIFT: return 115;
      case SDL_SCANCODE_RSHIFT: return 116;
      case SDL_SCANCODE_LCTRL: return 117;
      case SDL_SCANCODE_RCTRL: return 118;
      case SDL_SCANCODE_LALT: return 119;
      case SDL_SCANCODE_RALT: return 120;
      case SDL_SCANCODE_LGUI: return 121;
      case SDL_SCANCODE_RGUI: return 122;
      case SDL_SCANCODE_APPLICATION: return 123;
      case SDL_SCANCODE_SCROLLLOCK: return 124;
      case SDL_SCANCODE_NUMLOCKCLEAR: return 125;
      case SDL_SCANCODE_CAPSLOCK: return 126;
      default: return 0;
   }
}

static int ascii_for(const SDL_KeyboardEvent *k, int allegro_sc)
{
   SDL_Keycode kc;
   switch (allegro_sc) {
      case 67: case 91: return 13;   /* enter */
      case 63: return 8;             /* backspace */
      case 64: return 9;             /* tab */
      case 59: return 27;            /* esc */
      case 77: return 127;           /* del */
      default: break;
   }
   if (k->mod & (SDL_KMOD_CTRL | SDL_KMOD_ALT)) {
      kc = SDL_GetKeyFromScancode(k->scancode, SDL_KMOD_NONE, false);
      if (k->mod & SDL_KMOD_CTRL && kc >= 'a' && kc <= 'z')
         return (int)(kc - 'a' + 1);
      return 0;
   }
   kc = SDL_GetKeyFromScancode(k->scancode, k->mod, false);
   if (kc >= 32 && kc < 256)
      return (int)kc;
   return 0;
}

bool input_keybuf_empty(void) { return keybuf_head == keybuf_tail; }

int input_keybuf_pop(void)
{
   int v;
   if (keybuf_head == keybuf_tail)
      return 0;
   v = keybuf[keybuf_tail];
   keybuf_tail = (keybuf_tail + 1) % KEYBUF_SIZE;
   return v;
}

void input_keybuf_push(int scancode, int ascii)
{
   int next = (keybuf_head + 1) % KEYBUF_SIZE;
   if (next == keybuf_tail)
      return;
   keybuf[keybuf_head] = (scancode << 8) | (ascii & 0xFF);
   keybuf_head = next;
}

void input_keybuf_clear(void) { keybuf_head = keybuf_tail = 0; }

void input_release_all_keys(void)
{
   int i;
   for (i = 0; i < INPUT_KEY_MAX; i++)
      input_key[i] = 0;
   input_key_shifts = 0;
}

static void update_shifts(SDL_Keymod mod)
{
   int s = 0;
   if (mod & SDL_KMOD_SHIFT) s |= 0x0001;
   if (mod & SDL_KMOD_CTRL) s |= 0x0002;
   if (mod & SDL_KMOD_ALT) s |= 0x0004;
   if (mod & SDL_KMOD_NUM) s |= 0x0200;
   if (mod & SDL_KMOD_CAPS) s |= 0x0400;
   input_key_shifts = s;
}

/* ---------------------------------------------------------------- pads */

static void pad_open_first(void)
{
   int n = 0;
   SDL_JoystickID *ids;
   if (g_pad)
      return;
   ids = SDL_GetGamepads(&n);
   if (ids && n > 0)
      g_pad = SDL_OpenGamepad(ids[0]);
   SDL_free(ids);
}

static void pad_refresh(void)
{
   static const SDL_GamepadButton order[] = {
      SDL_GAMEPAD_BUTTON_SOUTH, SDL_GAMEPAD_BUTTON_EAST, SDL_GAMEPAD_BUTTON_WEST,
      SDL_GAMEPAD_BUTTON_NORTH, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
      SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, SDL_GAMEPAD_BUTTON_BACK,
      SDL_GAMEPAD_BUTTON_START, SDL_GAMEPAD_BUTTON_LEFT_STICK,
      SDL_GAMEPAD_BUTTON_RIGHT_STICK, SDL_GAMEPAD_BUTTON_GUIDE
   };
   int i, n = (int)(sizeof(order) / sizeof(order[0]));
   int ax, ay;
   memset(&g_pad_state, 0, sizeof(g_pad_state));
   if (!g_pad || !SDL_GamepadConnected(g_pad))
      return;
   g_pad_state.connected = true;
   g_pad_state.num_buttons = n;
   for (i = 0; i < n; i++)
      g_pad_state.button[i] = SDL_GetGamepadButton(g_pad, order[i]);
   /* triggers act as extra buttons, like DirectInput pads exposed them */
   g_pad_state.button[n] = SDL_GetGamepadAxis(g_pad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > 16000;
   g_pad_state.button[n + 1] = SDL_GetGamepadAxis(g_pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 16000;
   g_pad_state.num_buttons = n + 2;
   ax = SDL_GetGamepadAxis(g_pad, SDL_GAMEPAD_AXIS_LEFTX);
   ay = SDL_GetGamepadAxis(g_pad, SDL_GAMEPAD_AXIS_LEFTY);
   g_pad_state.axis_x = ax / 256;
   g_pad_state.axis_y = ay / 256;
   /* Allegro's digital thresholds are at half deflection */
   g_pad_state.left = ax < -16384 || SDL_GetGamepadButton(g_pad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
   g_pad_state.right = ax > 16384 || SDL_GetGamepadButton(g_pad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
   g_pad_state.up = ay < -16384 || SDL_GetGamepadButton(g_pad, SDL_GAMEPAD_BUTTON_DPAD_UP);
   g_pad_state.down = ay > 16384 || SDL_GetGamepadButton(g_pad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
}

const input_pad_state *input_pad(void)
{
   pad_refresh();
   return &g_pad_state;
}

/* ---------------------------------------------------------------- events */

static void on_event(const SDL_Event *ev)
{
   int sc, cx, cy;
   switch (ev->type) {
      case SDL_EVENT_KEY_DOWN:
         sc = map_scancode(ev->key.scancode);
         update_shifts(ev->key.mod);
         if (!sc)
            break;
         if (!ev->key.repeat)
            input_key[sc] = 1;
         input_keybuf_push(sc, ascii_for(&ev->key, sc));
         break;
      case SDL_EVENT_KEY_UP:
         sc = map_scancode(ev->key.scancode);
         update_shifts(ev->key.mod);
         if (sc)
            input_key[sc] = 0;
         break;
      case SDL_EVENT_WINDOW_FOCUS_LOST:
         input_release_all_keys();
         break;
      case SDL_EVENT_MOUSE_MOTION:
         present_window_to_canvas(ev->motion.x, ev->motion.y, &cx, &cy);
         input_mouse_x = cx;
         input_mouse_y = cy;
         break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      case SDL_EVENT_MOUSE_BUTTON_UP: {
         int bit = ev->button.button == SDL_BUTTON_LEFT ? 1 :
                   ev->button.button == SDL_BUTTON_RIGHT ? 2 :
                   ev->button.button == SDL_BUTTON_MIDDLE ? 4 : 0;
         if (ev->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            input_mouse_b |= bit;
         else
            input_mouse_b &= ~bit;
         present_window_to_canvas(ev->button.x, ev->button.y, &cx, &cy);
         input_mouse_x = cx;
         input_mouse_y = cy;
         break;
      }
      case SDL_EVENT_GAMEPAD_ADDED:
         pad_open_first();
         break;
      case SDL_EVENT_GAMEPAD_REMOVED:
         if (g_pad && SDL_GetGamepadID(g_pad) == ev->gdevice.which) {
            SDL_CloseGamepad(g_pad);
            g_pad = NULL;
            pad_open_first();
         }
         break;
      default:
         break;
   }
}

void input_init(void)
{
   if (g_inited)
      return;
   g_inited = true;
   plat_add_event_handler(on_event);
   if (!plat_headless())
      pad_open_first();
}

void input_set_cursor(int kind)
{
   if (plat_headless())
      return;
   if (kind == 100) {
      if (!g_cursor_hand)
         g_cursor_hand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
      if (g_cursor_hand)
         SDL_SetCursor(g_cursor_hand);
   } else {
      if (!g_cursor_arrow)
         g_cursor_arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
      if (g_cursor_arrow)
         SDL_SetCursor(g_cursor_arrow);
   }
}

void input_show_cursor(bool show)
{
   if (plat_headless())
      return;
   if (show)
      SDL_ShowCursor();
   else
      SDL_HideCursor();
}

/* ---------------------------------------------------------------- actions */

bool input_action_down(input_action a)
{
   const input_pad_state *p = input_pad();
   switch (a) {
      case ACTION_LEFT: return input_key[82] || p->left;
      case ACTION_RIGHT: return input_key[83] || p->right;
      case ACTION_UP: return input_key[84] || p->up;
      case ACTION_DOWN: return input_key[85] || p->down;
      case ACTION_JUMP: return input_key[75] || p->button[0];
      case ACTION_PAUSE: return input_key[16] || (p->num_buttons > 7 && p->button[7]);
      case ACTION_CONFIRM: return input_key[67] || input_key[91] || p->button[0];
      case ACTION_BACK: return input_key[59] || p->button[1];
      case ACTION_SCREENSHOT: return input_key[47];
      case ACTION_FULLSCREEN: return input_key[119] && input_key[67];
      default: return false;
   }
}

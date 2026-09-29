/*
 * SDL3 input backend: keyboard (mapped to Allegro 4.4.1 scancodes), mouse,
 * gamepads (SDL gamepad API, hot-plug) and logical actions.
 */
#include <string.h>
#include <stdlib.h>
#include "port/input/input.h"
#include "port/input/touch.h"
#include "port/platform/plat_internal.h"
#include "port/render/present.h"

volatile char input_key[INPUT_KEY_MAX];
volatile int input_key_shifts;
volatile int input_mouse_x, input_mouse_y, input_mouse_b;

#define KEYBUF_SIZE 64
static int keybuf[KEYBUF_SIZE];
static int keybuf_head, keybuf_tail;

static void run_script(void);
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
      case SDL_SCANCODE_AC_BACK: return 59;   /* Android back button: Esc (pause / back) */
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

/* one-shot CTRL_PAUSE when the app is backgrounded (mobile).  SDL delivers
 * the lifecycle events only to event watchers, on the OS UI thread, while
 * the game thread is about to be frozen; the watcher just sets this flag and
 * the game pauses on its next control poll after it resumes. */
static SDL_AtomicInt g_pause_request;

static bool SDLCALL lifecycle_watch(void *user, SDL_Event *ev)
{
   (void)user;
   if (ev->type == SDL_EVENT_WILL_ENTER_BACKGROUND)
      SDL_SetAtomicInt(&g_pause_request, 1);
   return true;
}

/* The game polls key *state* in its wait loops and 50 Hz ticks.  A press
 * whose down and up arrive in the same event pump (touch taps, injected or
 * very quick key taps) would never be seen, so every press stays down for
 * at least KEY_MIN_HOLD_NS, which is longer than one simulation tick. */
#define KEY_MIN_HOLD_NS 30000000ull
static uint64_t g_key_down_ns[INPUT_KEY_MAX];
static unsigned char g_key_release_pending[INPUT_KEY_MAX];

static void apply_pending_releases(uint64_t now)
{
   int i;
   for (i = 0; i < INPUT_KEY_MAX; i++)
      if (g_key_release_pending[i] && now - g_key_down_ns[i] >= KEY_MIN_HOLD_NS) {
         g_key_release_pending[i] = 0;
         input_key[i] = 0;
      }
}

void input_virtual_key(int sc, int ascii, bool down)
{
   if (sc <= 0 || sc >= INPUT_KEY_MAX)
      return;
   if (down) {
      input_key[sc] = 1;
      g_key_down_ns[sc] = SDL_GetTicksNS();
      g_key_release_pending[sc] = 0;
      input_keybuf_push(sc, ascii);
   } else if (SDL_GetTicksNS() - g_key_down_ns[sc] < KEY_MIN_HOLD_NS) {
      g_key_release_pending[sc] = 1;
   } else {
      input_key[sc] = 0;
   }
}

/* While the game reads a typed string (get_string) the platform's text
 * input is on: phones show their soft keyboard.  Printable characters then
 * arrive as text events; key-down events keep only their key state and the
 * special keys (Enter, Backspace, Esc), so hardware keys are not doubled. */
static bool g_text_input;

void input_text_input(bool on)
{
#ifdef SDL_PLATFORM_ANDROID
   SDL_Window *w = present_window();
   if (on == g_text_input || !w)
      return;
   g_text_input = on;
   if (on)
      SDL_StartTextInput(w);
   else
      SDL_StopTextInput(w);
#else
   (void)on;   /* desktop: every key already arrives as a key event */
#endif
}

static void on_event(const SDL_Event *ev)
{
   int sc, cx, cy;
   run_script();
   switch (ev->type) {
      case SDL_EVENT_USER:   /* once per pump */
         apply_pending_releases(SDL_GetTicksNS());
         break;
      case SDL_EVENT_KEY_DOWN:
         sc = map_scancode(ev->key.scancode);
         update_shifts(ev->key.mod);
         if (!sc)
            break;
         if (!ev->key.repeat) {
            input_key[sc] = 1;
            g_key_down_ns[sc] = SDL_GetTicksNS();
            g_key_release_pending[sc] = 0;
         }
         {
            int a = ascii_for(&ev->key, sc);
            /* Allegro never buffers the modifier and lock keys (KEY_LSHIFT..);
               a buffered Shift reached get_string() as character 0 and cut
               every name typed with a capital letter */
            if (sc < 115 && !(g_text_input && a >= 32 && a < 127))
               input_keybuf_push(sc, a);
         }
         break;
      case SDL_EVENT_TEXT_INPUT: {
         const unsigned char *t = (const unsigned char *)ev->text.text;
         for (; t && *t; t++)
            if (*t >= 32 && *t < 127)
               input_keybuf_push(0, *t);   /* plain character (scancode 0) */
         break;
      }
      case SDL_EVENT_KEY_UP:
         sc = map_scancode(ev->key.scancode);
         update_shifts(ev->key.mod);
         if (sc) {
            if (SDL_GetTicksNS() - g_key_down_ns[sc] < KEY_MIN_HOLD_NS)
               g_key_release_pending[sc] = 1;
            else
               input_key[sc] = 0;
         }
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

/* ---------------------------------------------------------------- script */
/* --keys "MS+CODE,MS-CODE,...": press (+) / release (-) Allegro scancode
 * CODE at MS milliseconds after start; for automated end-to-end tests. */
#define MAX_SCRIPT 512
static struct { uint64_t ms; int code; int down; } g_script[MAX_SCRIPT];
static int g_script_n, g_script_pos;
static uint64_t g_script_t0;

void input_set_script(const char *spec)
{
   const char *p = spec;
   g_script_t0 = SDL_GetTicksNS();
   while (p && *p && g_script_n < MAX_SCRIPT) {
      char *e;
      unsigned long long ms = strtoull(p, &e, 10);
      int down;
      if (*e != '+' && *e != '-')
         break;
      down = *e == '+';
      g_script[g_script_n].ms = ms;
      g_script[g_script_n].down = down;
      g_script[g_script_n].code = (int)strtol(e + 1, &e, 10);
      g_script_n++;
      p = *e == ',' ? e + 1 : NULL;
   }
}

static void run_script(void)
{
   uint64_t ms;
   if (g_script_pos >= g_script_n)
      return;
   ms = (SDL_GetTicksNS() - g_script_t0) / 1000000u;
   while (g_script_pos < g_script_n && g_script[g_script_pos].ms <= ms) {
      int c = g_script[g_script_pos].code;
      if (c > 0 && c < INPUT_KEY_MAX) {
         input_key[c] = (char)g_script[g_script_pos].down;
         if (g_script[g_script_pos].down)
            input_keybuf_push(c, c == 67 ? 13 : c == 75 ? ' ' : c == 59 ? 27 : 0);
      }
      g_script_pos++;
   }
}

void input_init(void)
{
   if (g_inited)
      return;
   g_inited = true;
   plat_add_event_handler(on_event);
   SDL_AddEventWatch(lifecycle_watch, NULL);
   touch_init();
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
#ifdef SDL_PLATFORM_ANDROID
   show = false;   /* touch device: the game's mouse cursor is never shown */
#endif
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

/* ---------------------------------------------------------------- control */

static int g_virtual_flags;

void input_set_virtual_flags(int flags) { g_virtual_flags = flags; }

int input_control_flags(const input_bindings *b, int pad_only)
{
   int f = 0, i;
   if (b->use_pad) {
      const input_pad_state *p = input_pad();
      if (p->up) f |= b->pad_up;
      if (p->down) f |= b->pad_down;
      if (p->left) f |= b->pad_left;
      if (p->right) f |= b->pad_right;
      for (i = 0; i < p->num_buttons && i < 32; i++)
         if (p->button[i]) f |= b->pad_button[i];
      /* Start (button 7) pauses, as players expect on a gamepad */
      if (p->num_buttons > 7 && p->button[7])
         f |= 0x40;
   }
   if (!pad_only) {
#define K(code) ((code) > 0 && (code) < INPUT_KEY_MAX && input_key[(code)])
      if (K(b->key_up)) f |= 0x04;
      if (K(b->key_down)) f |= 0x08;
      if (K(b->key_left)) f |= 0x01;
      if (K(b->key_right)) f |= 0x02;
      if (K(b->key_fire)) f |= 0x10;
      if (K(b->key_enter)) f |= 0x20;
      if (K(b->key_pause)) f |= 0x40;
#undef K
      f |= g_virtual_flags;
      f |= touch_control_flags();
   }
   if (SDL_GetAtomicInt(&g_pause_request) && SDL_SetAtomicInt(&g_pause_request, 0))
      f |= 0x40;   /* back from the background: pause, as the pause key does */
   return f;
}

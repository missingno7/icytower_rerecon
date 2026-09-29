/*
 * Touch controls: see touch.h.
 *
 * Fingers are classified when they go down (by the layout of that moment)
 * and keep their role until they lift, so a thumb holding "right" is not
 * reinterpreted when a menu appears.  Gameplay flags are computed on demand
 * from the fingers; remote buttons are sent as virtual key presses.
 */
#include <math.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "port/input/touch.h"
#include "port/input/input.h"
#include "port/platform/plat_internal.h"

#define MAX_FINGERS 10
#define GAMEPLAY_HOLD_NS 150000000ull    /* gameplay context lasts this long after a frame */
#define JUMP_MIN_NS 30000000ull          /* a jump tap is held at least one tick */

/* Allegro 4 scancodes used by the remote */
enum { SC_ENTER = 67, SC_ESC = 59, SC_SPACE = 75, SC_LEFT = 82, SC_RIGHT = 83, SC_UP = 84, SC_DOWN = 85 };

typedef struct finger {
   bool used;
   SDL_FingerID id;
   touch_button role;
   float x, y;                 /* output pixels */
   float anchor_x;             /* SLIDE */
   int key;                    /* remote: scancode held by this finger */
} finger;

static finger g_f[MAX_FINGERS];
static int g_scheme = TOUCH_ZONES;
static float g_opacity = 0.35f;
static bool g_left_handed;
static bool g_seen;             /* a touch device has been used (Android: always) */
static bool g_dirty = true;
static uint64_t g_gameplay_ns;  /* last gameplay frame */
static bool g_replay;
static uint64_t g_jump_until;
static bool g_pause_pending;
static int g_out_w = 1280, g_out_h = 720;
static SDL_Sensor *g_accel;
static int g_tilt_dir;

static const char *const k_names[TOUCH_SCHEME_COUNT] = { "zones", "slide", "tilt", "off" };

const char *touch_scheme_name(int s) { return s >= 0 && s < TOUCH_SCHEME_COUNT ? k_names[s] : "zones"; }

int touch_scheme_from_name(const char *n)
{
   int i;
   for (i = 0; n && i < TOUCH_SCHEME_COUNT; i++)
      if (!SDL_strcasecmp(n, k_names[i]))
         return i;
   return -1;
}

static bool in_gameplay(void)
{
   return !g_replay && g_gameplay_ns && SDL_GetTicksNS() - g_gameplay_ns < GAMEPLAY_HOLD_NS;
}

void touch_note_gameplay(bool replay)
{
   bool was = in_gameplay();
   g_gameplay_ns = SDL_GetTicksNS();
   g_replay = replay;
   if (was != in_gameplay())
      g_dirty = true;
}

static void tilt_open(bool on)
{
   if (on && !g_accel) {
      SDL_SensorID *ids;
      int n = 0, i;
      if (!SDL_WasInit(SDL_INIT_SENSOR))
         SDL_InitSubSystem(SDL_INIT_SENSOR);
      ids = SDL_GetSensors(&n);
      for (i = 0; ids && i < n && !g_accel; i++)
         if (SDL_GetSensorTypeForID(ids[i]) == SDL_SENSOR_ACCEL)
            g_accel = SDL_OpenSensor(ids[i]);
      SDL_free(ids);
      if (!g_accel)
         SDL_Log("touch: no accelerometer, tilt controls unavailable");
   } else if (!on && g_accel) {
      SDL_CloseSensor(g_accel);
      g_accel = NULL;
      g_tilt_dir = 0;
   }
}

void touch_set_scheme(int s)
{
   if (s < 0 || s >= TOUCH_SCHEME_COUNT)
      s = TOUCH_ZONES;
   g_scheme = s;
   tilt_open(s == TOUCH_TILT && g_seen);
   g_dirty = true;
}

int touch_scheme(void) { return g_scheme; }
void touch_set_opacity(float a) { g_opacity = a < 0.05f ? 0.05f : a > 1.0f ? 1.0f : a; g_dirty = true; }
void touch_set_left_handed(bool on) { g_left_handed = on; g_dirty = true; }
bool touch_take_dirty(void)
{
   /* the gameplay context also ends by itself when frames stop (pause) */
   static bool last_gameplay;
   bool d = g_dirty, gp = in_gameplay();
   if (gp != last_gameplay) {
      last_gameplay = gp;
      d = true;
   }
   g_dirty = false;
   return d && g_seen;
}

/* ---------------------------------------------------------------- layout */

static void add(touch_view *v, touch_button id, float x, float y, float r)
{
   touch_widget *w;
   if (v->count >= (int)(sizeof(v->w) / sizeof(v->w[0])))
      return;
   w = &v->w[v->count++];
   w->id = id;
   w->x = g_left_handed ? (float)g_out_w - x : x;
   w->y = y;
   w->r = r;
   w->pressed = false;
}

static void layout(int W, int H, bool gameplay, touch_view *v)
{
   float u = (float)H / 100.0f;   /* 1% of the height: scales with the screen */
   memset(v, 0, sizeof(*v));
   g_out_w = W;
   g_out_h = H;
   v->visible = g_seen && (g_scheme != TOUCH_OFF || !gameplay);
   v->gameplay = gameplay;
   v->scheme = g_scheme;
   v->opacity = g_opacity;
   v->slide_anchor_x = -1.0f;
   v->tilt_dir = g_tilt_dir;
   if (gameplay) {
      if (g_scheme == TOUCH_ZONES) {
         add(v, TB_LEFT, (float)W * 0.075f, (float)H * 0.74f, 10.0f * u);
         add(v, TB_RIGHT, (float)W * 0.19f, (float)H * 0.74f, 10.0f * u);
         add(v, TB_JUMP, (float)W * 0.88f, (float)H * 0.74f, 13.0f * u);
      } else if (g_scheme == TOUCH_SLIDE) {
         add(v, TB_SLIDE, (float)W * 0.13f, (float)H * 0.74f, 13.0f * u);
         add(v, TB_JUMP, (float)W * 0.88f, (float)H * 0.74f, 13.0f * u);
      } else if (g_scheme == TOUCH_TILT) {
         add(v, TB_TILT, (float)W * 0.5f, (float)H * 0.9f, 5.0f * u);
      }
      if (g_scheme != TOUCH_OFF)
         add(v, TB_PAUSE, (float)W - 8.0f * u, 8.0f * u, 5.5f * u);
   } else {
      float cx = 17.0f * u, cy = (float)H - 17.0f * u, d = 10.0f * u, r = 6.0f * u;
      add(v, TB_UP, cx, cy - d, r);
      add(v, TB_DOWN, cx, cy + d, r);
      add(v, TB_KLEFT, cx - d, cy, r);
      add(v, TB_KRIGHT, cx + d, cy, r);
      add(v, TB_OK, (float)W - 15.0f * u, (float)H - 17.0f * u, 10.0f * u);
      add(v, TB_BACK, (float)W - 8.0f * u, 8.0f * u, 5.5f * u);
   }
}

/* which role a finger going down at (x, y) takes */
static touch_button classify(float x, float y, bool gameplay, const touch_view *v)
{
   int i;
   float W = (float)g_out_w;
   float mx = g_left_handed ? W - x : x;   /* position in the right-handed layout */
   for (i = 0; i < v->count; i++) {
      const touch_widget *w = &v->w[i];
      float dx = x - w->x, dy = y - w->y, rr = w->r * 1.25f;   /* generous hit area */
      if (w->id == TB_PAUSE || w->id == TB_BACK || !gameplay)
         if (dx * dx + dy * dy <= rr * rr)
            return w->id;
   }
   if (!gameplay || g_scheme == TOUCH_OFF)
      return TB_NONE;
   switch (g_scheme) {
      case TOUCH_ZONES:
         /* big zones: whole left side (outer part = left), right side = jump */
         if (mx < W * 0.38f)
            return mx < W * 0.1325f ? TB_LEFT : TB_RIGHT;
         if (mx > W * 0.62f)
            return TB_JUMP;
         return TB_NONE;
      case TOUCH_SLIDE:
         if (mx < W * 0.45f)
            return TB_SLIDE;
         if (mx > W * 0.55f)
            return TB_JUMP;
         return TB_NONE;
      case TOUCH_TILT:
         return TB_JUMP;   /* anywhere */
      default:
         return TB_NONE;
   }
}

static int remote_key(touch_button b)
{
   switch (b) {
      case TB_UP: return SC_UP;
      case TB_DOWN: return SC_DOWN;
      case TB_KLEFT: return SC_LEFT;
      case TB_KRIGHT: return SC_RIGHT;
      case TB_OK: return g_replay ? SC_SPACE : SC_ENTER;   /* replays: pause/resume */
      case TB_BACK: return SC_ESC;
      default: return 0;
   }
}

static int remote_ascii(int sc)
{
   return sc == SC_ENTER ? 13 : sc == SC_ESC ? 27 : sc == SC_SPACE ? ' ' : 0;
}

/* ---------------------------------------------------------------- events */

static finger *find(SDL_FingerID id)
{
   int i;
   for (i = 0; i < MAX_FINGERS; i++)
      if (g_f[i].used && g_f[i].id == id)
         return &g_f[i];
   return NULL;
}

static void release(finger *f)
{
   if (f->key)
      input_virtual_key(f->key, 0, false);
   f->used = false;
   f->key = 0;
   g_dirty = true;
}

static void on_event(const SDL_Event *ev)
{
   finger *f;
   touch_view v;
   float x, y;
   int i;
   switch (ev->type) {
      case SDL_EVENT_FINGER_DOWN:
         if (!g_seen) {
            g_seen = true;
            tilt_open(g_scheme == TOUCH_TILT);
         }
         x = ev->tfinger.x * (float)g_out_w;
         y = ev->tfinger.y * (float)g_out_h;
         touch_get_view(g_out_w, g_out_h, &v);
         for (i = 0; i < MAX_FINGERS && g_f[i].used; i++)
            ;
         if (i == MAX_FINGERS)
            break;
         f = &g_f[i];
         memset(f, 0, sizeof(*f));
         f->used = true;
         f->id = ev->tfinger.fingerID;
         f->x = x;
         f->y = y;
         f->anchor_x = x;
         f->role = classify(x, y, v.gameplay, &v);
         if (f->role == TB_JUMP)
            g_jump_until = SDL_GetTicksNS() + JUMP_MIN_NS;
         else if (f->role == TB_PAUSE)
            g_pause_pending = true;
         else if ((f->key = remote_key(f->role)) != 0)
            input_virtual_key(f->key, remote_ascii(f->key), true);
         g_dirty = true;
         break;
      case SDL_EVENT_FINGER_MOTION:
         if (!(f = find(ev->tfinger.fingerID)))
            break;
         f->x = ev->tfinger.x * (float)g_out_w;
         f->y = ev->tfinger.y * (float)g_out_h;
         if (f->role == TB_LEFT || f->role == TB_RIGHT) {
            /* rocking the thumb across the zone boundary switches direction */
            float W = (float)g_out_w, mx = g_left_handed ? W - f->x : f->x;
            touch_button r = mx < W * 0.1325f ? TB_LEFT : TB_RIGHT;
            if (r != f->role) { f->role = r; g_dirty = true; }
         } else if (f->role == TB_SLIDE) {
            float follow = (float)g_out_h * 0.08f;
            float dx = f->x - f->anchor_x;
            if (dx > follow) f->anchor_x = f->x - follow;       /* anchor trails the thumb */
            if (dx < -follow) f->anchor_x = f->x + follow;
            g_dirty = true;
         }
         break;
      case SDL_EVENT_FINGER_UP:
      case SDL_EVENT_FINGER_CANCELED:
         if ((f = find(ev->tfinger.fingerID)) != NULL)
            release(f);
         break;
      case SDL_EVENT_SENSOR_UPDATE:
         if (g_accel && ev->sensor.which == SDL_GetSensorID(g_accel)) {
            /* gravity along the screen's horizontal axis (sensor axes are the
               device's natural portrait orientation) */
            float ax = ev->sensor.data[0], ay = ev->sensor.data[1], s;
            SDL_DisplayOrientation o = SDL_GetCurrentDisplayOrientation(SDL_GetPrimaryDisplay());
            if (o == SDL_ORIENTATION_LANDSCAPE) s = -ay;
            else if (o == SDL_ORIENTATION_LANDSCAPE_FLIPPED) s = ay;
            else if (o == SDL_ORIENTATION_PORTRAIT_FLIPPED) s = ax;
            else s = -ax;
            /* s > 0: the left edge is lower.  ~9 degrees to start running,
               hysteresis so the direction does not flicker */
            {
               int d = g_tilt_dir;
               if (s > 1.6f) d = -1;
               else if (s < -1.6f) d = 1;
               else if (s > -0.9f && s < 0.9f) d = 0;
               if (d != g_tilt_dir) { g_tilt_dir = d; g_dirty = true; }
            }
         }
         break;
      default:
         break;
   }
}

void touch_init(void)
{
   static bool inited;
   if (inited)
      return;
   inited = true;
   /* touches are touches: the game must not see them as mouse clicks */
   SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
#ifdef SDL_PLATFORM_ANDROID
   g_seen = true;
#endif
   plat_add_event_handler(on_event);
   tilt_open(g_scheme == TOUCH_TILT && g_seen);   /* the config may have chosen tilt already */
}

int touch_control_flags(void)
{
   int f = 0, i;
   bool left = false, right = false, jump = false;
   uint64_t now = SDL_GetTicksNS();
   if (g_pause_pending) {
      g_pause_pending = false;
      f |= 0x40;
   }
   if (!in_gameplay() || g_scheme == TOUCH_OFF)
      return f;
   for (i = 0; i < MAX_FINGERS; i++) {
      const finger *p = &g_f[i];
      if (!p->used)
         continue;
      if (p->role == TB_LEFT) left = true;
      else if (p->role == TB_RIGHT) right = true;
      else if (p->role == TB_JUMP) jump = true;
      else if (p->role == TB_SLIDE) {
         float dx = p->x - p->anchor_x, dead = (float)g_out_h * 0.03f;
         if (g_left_handed) dx = -dx;
         if (dx < -dead) left = true;
         else if (dx > dead) right = true;
      }
   }
   if (g_scheme == TOUCH_TILT) {
      left = g_tilt_dir < 0;
      right = g_tilt_dir > 0;
   }
   if (now < g_jump_until)
      jump = true;
   if (left && !right) f |= 0x01;
   if (right && !left) f |= 0x02;
   if (jump) f |= 0x10;
   return f;
}

void touch_get_view(int w, int h, touch_view *v)
{
   int i, j;
   layout(w, h, in_gameplay(), v);
   for (i = 0; i < MAX_FINGERS; i++) {
      const finger *p = &g_f[i];
      if (!p->used)
         continue;
      for (j = 0; j < v->count; j++)
         if (v->w[j].id == p->role)
            v->w[j].pressed = true;
      if (p->role == TB_SLIDE) {
         v->slide_anchor_x = p->anchor_x;
         v->slide_thumb_x = p->x;
         v->slide_y = p->y;
      }
   }
   if (SDL_GetTicksNS() < g_jump_until)
      for (j = 0; j < v->count; j++)
         if (v->w[j].id == TB_JUMP)
            v->w[j].pressed = true;
}

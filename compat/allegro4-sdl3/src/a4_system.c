/*
 * System, graphics mode and screen, messages, display switching, and the
 * main-thread service loop that replaces Allegro's background threads.
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "a4_internal.h"
#include "port/platform/platform.h"
#include "port/render/present.h"
#include "port/input/input.h"

static int a4_errno_fallback;
int *allegro_errno = &a4_errno_fallback;
int gui_fg_color, gui_bg_color;
BITMAP *screen;
GFX_DRIVER *gfx_driver;
volatile int a4_close_requested;
int a4_gfx_mode_set;

static GFX_DRIVER the_driver;
static void (*close_proc)(void);
static void (*switch_in_proc)(void);
static void (*switch_out_proc)(void);
static char window_title[256] = "Icy Tower";

void a4_bitmap_init(void);
void a4_bitmap_mode_set(void);
void a4_mouse_refresh(void);

int allegro_init(void)
{
   allegro_errno = &errno;
   a4_bitmap_init();
   return 0;
}

void allegro_exit(void)
{
   if (screen) {
      BITMAP *s = screen;
      present_set_canvas(NULL);
      screen = NULL;
      destroy_bitmap(s);
   }
   present_close();
   gfx_driver = NULL;
   remove_sound();
}

void allegro_message(const char *msg, ...)
{
   char buf[4096];
   va_list ap;
   va_start(ap, msg);
   vsnprintf(buf, sizeof(buf), msg, ap);
   va_end(ap);
   plat_message_box(window_title, buf);
}

void set_window_title(const char *name)
{
   snprintf(window_title, sizeof(window_title), "%s", name ? name : "");
   present_set_title(window_title);
}

int set_close_button_callback(void (*proc)(void))
{
   close_proc = proc;
   return 0;
}

int alert(const char *s1, const char *s2, const char *s3,
          const char *b1, const char *b2, int c1, int c2)
{
   char buf[1024];
   (void)c1; (void)c2;
   snprintf(buf, sizeof(buf), "%s%s%s%s%s", s1 ? s1 : "", s2 ? "\n" : "", s2 ? s2 : "",
            s3 ? "\n" : "", s3 ? s3 : "");
   return plat_choice_box(window_title, buf, b1 ? b1 : "OK", b2) + 1;
}

/* ---------------------------------------------------------------- display */

static void on_focus_in(void) { if (switch_in_proc) switch_in_proc(); }
static void on_focus_out(void) { if (switch_out_proc) switch_out_proc(); }

int set_display_switch_mode(int mode) { (void)mode; return 0; }

int set_display_switch_callback(int dir, void (*cb)(void))
{
   if (dir == SWITCH_IN)
      switch_in_proc = cb;
   else
      switch_out_proc = cb;
   plat_set_focus_callbacks(on_focus_in, on_focus_out);
   return 0;
}

int desktop_color_depth(void) { return 32; }

int set_gfx_mode(int card, int w, int h, int v_w, int v_h)
{
   (void)v_w; (void)v_h;
   if (card == GFX_TEXT) {
      /* Allegro closes the graphics mode (used before message boxes and on
       * exit).  Keep the window so a later mode switch is cheap; hide the
       * canvas contents by leaving presentation to the next set_gfx_mode. */
      return 0;
   }
   if (plat_headless())
      return -1;
   if (!screen || screen->w != w || screen->h != h || screen->depth != a4_color_depth) {
      BITMAP *old = screen;
      screen = NULL;
      if (old)
         destroy_bitmap(old);
      screen = create_bitmap_ex(a4_color_depth, w, h);
      if (!screen)
         return -1;
   }
   the_driver.id = card;
   the_driver.name = "SDL3";
   the_driver.w = w;
   the_driver.h = h;
   the_driver.windowed = card != GFX_AUTODETECT_FULLSCREEN;
   gfx_driver = &the_driver;
   if (!present_is_open()) {
      if (!present_open(w, h, card == GFX_AUTODETECT_FULLSCREEN))
         return -1;
      present_set_title(window_title);
   } else {
      present_set_fullscreen(card == GFX_AUTODETECT_FULLSCREEN);
   }
   input_init();
   present_set_canvas(screen);
   a4_gfx_mode_set = 1;
   clear_bitmap(screen);
   a4_bitmap_mode_set();
   present_service(true);
   return 0;
}

void vsync(void)
{
   a4_service();
   present_service(false);
}

void acquire_screen(void) { }
void release_screen(void) { }

/* ---------------------------------------------------------------- service */

void a4_service(void)
{
   static int depth;
   if (depth)
      return;
   depth++;
   plat_pump_events();
   a4_mouse_refresh();
   if (plat_quit_requested()) {
      plat_clear_quit_request();
      a4_close_requested = 1;
      if (close_proc)
         close_proc();
   }
   a4_timer_service();
   present_service(false);
   depth--;
}

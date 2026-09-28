/*
 * allegro_init/exit, graphics mode, screen, messages, display switching.
 * OWNER: integration (supervisor).  Temporary minimal version.
 */
#include <stdarg.h>
#include <stdio.h>
#include "a4_internal.h"
#include "port/platform/platform.h"

static int a4_errno_fallback;
int *allegro_errno = &a4_errno_fallback;
int gui_fg_color, gui_bg_color;
BITMAP *screen;
GFX_DRIVER *gfx_driver;
volatile int a4_close_requested;
int a4_gfx_mode_set;

void a4_bitmap_init(void);

int allegro_init(void)
{
   allegro_errno = &errno;
   a4_bitmap_init();
   return 0;
}

void allegro_exit(void) { }

void allegro_message(const char *msg, ...)
{
   char buf[4096];
   va_list ap;
   va_start(ap, msg);
   vsnprintf(buf, sizeof(buf), msg, ap);
   va_end(ap);
   plat_message_box("Icy Tower", buf);
}

void a4_service(void)
{
   plat_pump_events();
   a4_timer_service();
}

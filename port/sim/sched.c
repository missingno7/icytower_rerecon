/*
 * Fixed-step simulation scheduler (see sched.h).
 */
#include "port/sim/sched.h"
#include "port/platform/platform.h"
#include "port/render/render.h"
#include "port/render/present.h"

void a4_service(void);   /* compat: pump events + run due timers */

static bool g_inited, g_virtual;
static uint64_t g_last_ns, g_acc_ns, g_ticks, g_dropped;

void sched_init(void)
{
   g_last_ns = plat_ticks_ns();
   g_acc_ns = 0;
   g_inited = true;
}

void sched_set_virtual(bool on) { g_virtual = on; }
bool sched_is_virtual(void) { return g_virtual || plat_headless(); }

static void advance(void)
{
   const uint64_t max_backlog = (uint64_t)SCHED_MAX_CATCHUP_TICKS * SIMULATION_DT_NS;
   uint64_t now = plat_ticks_ns();
   g_acc_ns += now - g_last_ns;
   g_last_ns = now;
   if (g_acc_ns > max_backlog + SIMULATION_DT_NS) {
      uint64_t excess = g_acc_ns - max_backlog;
      g_dropped += excess / SIMULATION_DT_NS;
      g_acc_ns = max_backlog;
   }
}

void sched_resync(void)
{
   g_last_ns = plat_ticks_ns();
   g_acc_ns = 0;
}

void sched_wait_tick(void)
{
   if (!g_inited)
      sched_init();
   if (sched_is_virtual()) {
      g_ticks++;
      return;
   }
   for (;;) {
      a4_service();
      advance();
      if (g_acc_ns >= SIMULATION_DT_NS) {
         g_acc_ns -= SIMULATION_DT_NS;
         g_ticks++;
         return;
      }
      if (present_service(false))
         continue;                /* may block on vsync; time keeps accumulating */
      {
         /* nothing to draw yet: sleep until the step is due, but wake at
          * least every 2 ms to keep input and window events responsive */
         uint64_t left = SIMULATION_DT_NS - g_acc_ns;
         if (left > 2000000u)
            left = 2000000u;
         plat_sleep_ns(left);
      }
   }
}

float sched_alpha(void)
{
   float a;
   if (!g_inited || sched_is_virtual())
      return 1.0f;
   advance();
   a = (float)((double)g_acc_ns / (double)SIMULATION_DT_NS);
   return a < 0.0f ? 0.0f : a > 1.0f ? 1.0f : a;
}

uint64_t sched_ticks(void) { return g_ticks; }
uint64_t sched_dropped_ticks(void) { return g_dropped; }

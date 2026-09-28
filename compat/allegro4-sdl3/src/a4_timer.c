/*
 * Timers.  Allegro ran install_int() callbacks on a background thread; here
 * they run on the main thread from the service loop (a4_service), driven by
 * the platform's monotonic clock.  A callback due N times since the last
 * service runs N times (bounded), so counters such as the game's
 * cycle_count advance exactly with real time.
 *
 * The game's simulation pacing does not use these timers directly any more
 * once the fixed-step scheduler (port/sim/) is active; they remain for the
 * legacy wait loops and for fps bookkeeping.
 */
#include "a4_internal.h"
#include "port/platform/platform.h"

#define MAX_TIMERS 8
#define MAX_CATCH_UP 25

typedef struct {
   void (*proc)(void);
   uint64_t period_ns;
   uint64_t due_ns;
} a4_timer;

static a4_timer timers[MAX_TIMERS];
static int in_service;

int install_timer(void) { return 0; }

int install_int(void (*proc)(void), long speed_ms)
{
   int i, slot = -1;
   uint64_t now = plat_ticks_ns();
   if (!proc || speed_ms <= 0)
      return -1;
   for (i = 0; i < MAX_TIMERS; i++) {
      if (timers[i].proc == proc) { slot = i; break; }
      if (!timers[i].proc && slot < 0) slot = i;
   }
   if (slot < 0)
      return -1;
   timers[slot].proc = proc;
   timers[slot].period_ns = (uint64_t)speed_ms * 1000000u;
   timers[slot].due_ns = now + timers[slot].period_ns;
   return 0;
}

void remove_int(void (*proc)(void))
{
   int i;
   for (i = 0; i < MAX_TIMERS; i++)
      if (timers[i].proc == proc)
         timers[i].proc = NULL;
}

void a4_timer_service(void)
{
   int i, n;
   uint64_t now;
   if (in_service)
      return;
   in_service = 1;
   now = plat_ticks_ns();
   for (i = 0; i < MAX_TIMERS; i++) {
      a4_timer *t = &timers[i];
      if (!t->proc)
         continue;
      n = 0;
      while (now >= t->due_ns) {
         t->proc();
         t->due_ns += t->period_ns;
         if (++n >= MAX_CATCH_UP) {
            /* after a long stall, drop the backlog instead of spiralling */
            if (now >= t->due_ns)
               t->due_ns = now + t->period_ns;
            break;
         }
      }
   }
   in_service = 0;
}

/* Time until the earliest timer is due (for sleeping precisely). */
uint64_t a4_timer_next_due_in(void)
{
   int i;
   uint64_t now = plat_ticks_ns(), best = UINT64_MAX;
   for (i = 0; i < MAX_TIMERS; i++) {
      if (!timers[i].proc)
         continue;
      if (timers[i].due_ns <= now)
         return 0;
      if (timers[i].due_ns - now < best)
         best = timers[i].due_ns - now;
   }
   return best;
}

void rest(unsigned int ms)
{
   uint64_t end = plat_ticks_ns() + (uint64_t)ms * 1000000u;
   a4_service();
   while (!plat_headless()) {
      uint64_t now = plat_ticks_ns();
      uint64_t left, next;
      if (now >= end)
         break;
      left = end - now;
      next = a4_timer_next_due_in();
      if (next < left)
         left = next;
      if (left > 0)
         plat_sleep_ns(left);
      a4_service();
      if (plat_quit_requested())
         break;
   }
   if (plat_headless())
      a4_timer_service();
}

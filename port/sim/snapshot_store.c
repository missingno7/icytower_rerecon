/* Two-deep store of gameplay render snapshots (see snapshot.h). */
#include <string.h>
#include "port/sim/snapshot.h"

static game_snapshot g_snap[2];
static int g_cur;
static uint64_t g_gen;
static int g_discontinuity = 1;

void snapshot_push(const game_snapshot *s)
{
   int next = g_cur ^ 1;
   g_snap[next] = *s;
   g_snap[next].gen = ++g_gen;
   g_snap[next].valid = 1;
   if (g_discontinuity) {
      g_snap[next].teleported = 1;
      g_discontinuity = 0;
   }
   g_cur = next;
}

const game_snapshot *snapshot_current(void) { return g_gen ? &g_snap[g_cur] : NULL; }
const game_snapshot *snapshot_previous(void) { return g_gen > 1 ? &g_snap[g_cur ^ 1] : NULL; }
uint64_t snapshot_generation(void) { return g_gen; }
void snapshot_discontinuity(void) { g_discontinuity = 1; }

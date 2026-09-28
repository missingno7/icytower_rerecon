/*
 * Simulation fingerprint for regression tests (--sim-trace FILE).
 *
 * After every simulation step of play() the complete gameplay state that
 * feeds later steps (player kinematics as exact bit patterns, collision
 * state, level/score/combo bookkeeping, scroll offset, map rows, replay
 * position) is folded into a 64-bit FNV-1a hash.  When a game ends one line
 * is appended to FILE:
 *
 *   play ticks=N hash=H level=L score=S
 *
 * Running the same replay under different renderers, frame caps, vsync or
 * interpolation settings must produce identical lines.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "recovered_types.h"
#include "map.h"
#include "port/game/port_game.h"

extern Tplayer *ply[1000];
extern int player_id;
extern Tmap map;
extern int rec_pos;

static char g_path[1024];
static uint64_t g_hash, g_ticks;

void port_sim_trace_configure(const char *path)
{
   if (path)
      snprintf(g_path, sizeof(g_path), "%s", path);
}

static void mix(const void *p, size_t n)
{
   const unsigned char *b = (const unsigned char *)p;
   while (n--) {
      g_hash ^= *b++;
      g_hash *= 0x100000001B3ull;
   }
}

void port_sim_trace_begin(void)
{
   g_hash = 0xCBF29CE484222325ull;
   g_ticks = 0;
}

void port_sim_trace_tick(void)
{
   const Tplayer *p;
   if (!g_path[0])
      return;
   p = ply[player_id];
   g_ticks++;
   mix(&p->x, sizeof(p->x));
   mix(&p->y, sizeof(p->y));
   mix(&p->sx, sizeof(p->sx));
   mix(&p->sy, sizeof(p->sy));
   /* frame (animation) is presentation state, normalised by draw_frame */
   mix(&p->level, sizeof(p->level));
   mix(&p->score, sizeof(p->score));
   mix(&p->best_combo, sizeof(p->best_combo));
   mix(&p->status, sizeof(p->status));
   mix(&p->jump_key, sizeof(p->jump_key));
   mix(&p->in_combo, sizeof(p->in_combo));
   mix(&p->acc_level, sizeof(p->acc_level));
   mix(&p->acc_jumps, sizeof(p->acc_jumps));
   mix(&p->dead, sizeof(p->dead));
   mix(&p->rotate, sizeof(p->rotate));
   mix(&p->angle, sizeof(p->angle));
   mix(&p->edge, sizeof(p->edge));
   mix(&p->ccc, sizeof(p->ccc));
   mix(&p->jcTop, sizeof(p->jcTop));
   mix(&p->jc, sizeof(p->jc));
   mix(&map, sizeof(map));
   mix(&rec_pos, sizeof(rec_pos));
   if (getenv("ITOWER_TRACE_VERBOSE")) {
      FILE *f = fopen(g_path, "a");
      if (f) {
         uint64_t bx, by, bsx, bsy;
         memcpy(&bx, &p->x, 8); memcpy(&by, &p->y, 8); memcpy(&bsx, &p->sx, 8); memcpy(&bsy, &p->sy, 8);
         fprintf(f, "t%llu x=%016llx y=%016llx sx=%016llx sy=%016llx lvl=%d sc=%d bc=%d st=%d jk=%d ic=%d al=%d aj=%d dead=%d rot=%d ang=%d edge=%d off=%d rp=%d\n",
                 (unsigned long long)g_ticks, (unsigned long long)bx, (unsigned long long)by,
                 (unsigned long long)bsx, (unsigned long long)bsy, p->level, p->score, p->best_combo,
                 p->status, p->jump_key, p->in_combo, p->acc_level, p->acc_jumps, p->dead, p->rotate,
                 (int)p->angle, p->edge, map.offset, rec_pos);
         fclose(f);
      }
   }
}

void port_sim_trace_end(void)
{
   FILE *f;
   const Tplayer *p = ply[player_id];
   if (!g_path[0])
      return;
   f = fopen(g_path, "a");
   if (!f)
      return;
   fprintf(f, "play ticks=%llu hash=%016llx level=%d score=%d\n", (unsigned long long)g_ticks,
           (unsigned long long)g_hash, p->level, p->level * 10 + p->score);
   fclose(f);
}

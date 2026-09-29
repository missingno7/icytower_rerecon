/*
 * Game side of the render snapshots (port/sim/snapshot.h).
 *
 * play() brackets every draw_frame(swap_screen) with port_snapshot_pre()
 * and port_snapshot_post().  The capture reads the game state exactly as
 * draw_frame() used it and repeats draw_frame()'s presentation decisions
 * (player pose, sprite offsets, HUD shake) without changing any state, so
 * the historical software frame and the snapshot always describe the same
 * picture.  Then swap_screen's display list is reset to an "underlay"
 * marker: everything drawn after this point in the frame (results panel,
 * pause text, fades) is an overlay on top of the modern world rendering.
 */
#include <string.h>
#include "port/input/touch.h"
#include <allegro.h>
#include "recovered_types.h"
#include "map.h"
#include "particle.h"
#include "custom.h"
#include "control.h"
#include "timer.h"
#include "recovered/Treplay.h"
#include "recovered/Tprofile.h"
#include "recovered/Toptions.h"
#include "recovered/Tmenu_selection.h"
#include "port/sim/snapshot.h"
#include "port/sim/sched.h"
#include "port/platform/platform.h"

/* compat extension (a4_dl.h) */
void a4_dl_set_underlay(BITMAP *dst, uint64_t snapshot_generation);

extern Tplayer *ply[1000];
extern int player_id;
extern Tmap map;
extern Tparticle stars[512];
extern int bg_stripe_ids[5];
extern int hurry_y;
extern DATAFILE *data;
extern Tprofile *profile;
extern Toptions options;
extern Tcustom custom;
extern int reward_time;
extern fixed reward_scale;
extern BITMAP *reward_bmp;
extern int clock_angle;
extern int recording;
extern int rec_pos;
extern Treplay *demo;
extern int scroll_count;
extern int is_playing_custom_game;
extern Tcontrol ctrl;
extern Tmenu_selection floor_size_selection, scroll_speed_selection, gravity_selection;
extern BITMAP *swap_screen;
extern int debug;

static int g_pre_scroll_count;
static int g_pre_frame_count;

void port_snapshot_pre(void)
{
   /* draw_frame() uses these values before it updates them */
   g_pre_scroll_count = scroll_count;
   g_pre_frame_count = frame_count + 1;   /* draw_frame increments first */
}

static void capture_pose(game_snapshot *s, const Tplayer *p)
{
   /* mirrors draw_frame() (src/main.c, player section) */
   int p_im;
   BITMAP *cf;
   int oy, ox = 0;

   p_im = 6;
   if (!p->status) p_im = 1;
   if (p->status == 3 && p->sy > 3.0) p_im = 7;
   if (p->status == 2 && p->sy > 3.0) p_im = 7;
   if (p->status == 1 && p->sy < -3.0) p_im = 5;
   if (!p->status) { if (ABS(p->sx) < 0.02) p_im = 0; }
   if (p_im >= 5 && p_im <= 7 && ABS(p->sx) < 0.01) p_im = 8;

   s->rotating = 0;
   s->pose_flip = 0;
   s->pose_bmp = 0;
   cf = custom.frame[0];
   if (!cf) {
      s->pose_bmp = -1;
      return;
   }
   oy = 1 - cf->h;
   if (!p_im) {
      if (p->edge) {
         p_im = (logic_count & 8) ? 13 : 14;
         cf = custom.frame[p_im];
         s->pose_bmp = p_im;
         if (p->edge == 2) {
            s->pose_flip = 1;
            s->pose_ox = -cf->w + 11;
         } else {
            s->pose_ox = -11;
         }
         s->pose_oy = oy;
      } else {
         int idx = 0;
         /* historical control flow, including its dangling else: frame 10
          * is never selected */
         if (map.offset > 200 && p->y > 400.0) idx = 11;
         else if (logic_count <= 11) idx = 9;
         cf = custom.frame[idx];
         s->pose_bmp = idx;
         ox = -(cf->w / 2);
         s->pose_ox = ox;
         s->pose_oy = oy;
         s->pose_flip = !(p->sx > 0.0);
      }
   } else if (p->rotate) {
      cf = custom.frame[12];
      s->pose_bmp = 12;
      s->rotating = 1;
      s->pose_ox = -(cf->w / 2);
      s->pose_oy = -8 - custom.frame[0]->h;
   } else {
      int idx = p_im + p->frame;
      cf = custom.frame[idx];
      s->pose_bmp = idx;
      s->pose_ox = -(cf->w / 2);
      s->pose_oy = oy;
      s->pose_flip = !(p->sx > 0.0);
   }
}

void port_snapshot_post(void)
{
   static game_snapshot s;
   const Tplayer *p = ply[player_id];
   int i;

   memset(&s, 0, sizeof(s));
   s.tick = sched_ticks();
   s.time_ns = plat_ticks_ns();
   s.data = data;
   for (i = 0; i < 15; i++)
      s.frame[i] = custom.frame[i];

   s.map_offset = map.offset;
   for (i = 0; i < SNAP_ROOMS; i++) {
      s.room[i].empty = map.room[i].empty;
      s.room[i].start_tile = map.room[i].start_tile;
      s.room[i].end_tile = map.room[i].end_tile;
      s.room[i].level = map.room[i].level;
      s.room[i].sign = map.room[i].sign;
      s.room[i].tiles = map.room[i].tiles;
   }
   memcpy(s.stripe_ids, bg_stripe_ids, sizeof(s.stripe_ids));
   s.hurry_y = hurry_y;
   s.hurry_visible = hurry_y > -100 && hurry_y < 480 && options.flash != 2;
   s.fo = profile ? profile->start_floor * 3 + 17 : 17;
   s.so = profile ? profile->start_floor + 101 : 101;

   s.px = p->x;
   s.py = p->y;
   s.psx = p->sx;
   s.psy = p->sy;
   s.angle = p->angle;
   s.dead = p->dead;
   capture_pose(&s, p);

   for (i = 0; i < SNAP_PARTICLES; i++) {
      s.stars[i].x = stars[i].x;
      s.stars[i].y = stars[i].y;
      s.stars[i].color = stars[i].color;
      s.stars[i].intensity = stars[i].intensity;
   }

   s.score = p->level * 10 + p->score;
   s.in_combo = p->in_combo;
   s.combo_value = p->in_combo ? p->acc_level : (reward_time ? p->latest_combo : -1);
   s.reward_time = reward_time;
   s.reward_scale = reward_scale;
   s.reward_bmp = reward_bmp;
   s.flash = options.flash;
   s.clock_angle = clock_angle;
   if (hurry_y < 251 || hurry_y > 479) {
      s.clock_cx = 0;
      s.clock_cy = 0;
   } else {
      s.clock_cx = logic_count % 3 - 1;
      s.clock_cy = (logic_count + 1) % 3 - 1;
   }
   s.hand_cx = s.clock_cx;
   s.hand_cy = s.clock_cy;
   if (hurry_y >= 201 && hurry_y <= 479) {
      s.hand_cx = (logic_count + 2) % 3 - 1;
      s.hand_cy = (logic_count + 3) % 3 - 1;
   }
   s.frame_count = g_pre_frame_count;
   s.logic_count = logic_count;

   s.replay = !recording;
   if (s.replay && demo) {
      s.vcr_left = !p->dead && is_left(&ctrl);
      s.vcr_fire = !p->dead && is_fire(&ctrl);
      s.vcr_right = !p->dead && is_right(&ctrl);
      s.rec_pos = rec_pos;
      s.rec_size = demo->size;
      s.scroll_count = g_pre_scroll_count;
      memcpy(s.name, demo->name, 32);
      memcpy(s.comment, demo->comment, 42);
      s.custom_game = is_playing_custom_game;
      if (is_playing_custom_game) {
         sprintf(s.cap_floors, "%s Floors", floor_size_selection.caption[demo->floor_size]);
         sprintf(s.cap_speed, "%s Speed", scroll_speed_selection.caption[demo->start_speed]);
         strncpy(s.cap_gravity, gravity_selection.caption[demo->gravity], sizeof(s.cap_gravity) - 1);
      }
   }
   s.debug_rows = debug;

   snapshot_push(&s);
   touch_note_gameplay(s.replay != 0);   /* touch: gameplay controls while frames flow */
   a4_dl_set_underlay(swap_screen, snapshot_generation());
}

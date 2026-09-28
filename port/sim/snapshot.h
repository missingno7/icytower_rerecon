/*
 * Render snapshots of the gameplay scene.
 *
 * One snapshot is captured every time the game draws a gameplay frame
 * (draw_frame() in play(), i.e. once per simulation tick while playing and
 * once per drawn tick during fast-forward).  It holds every value the
 * historical renderer used, in game units (the 640x480 historical frame),
 * plus the decisions draw_frame() made (which sprite, which pose, flips).
 *
 * The modern renderer draws the world and HUD from the two most recent
 * snapshots, interpolating continuous quantities.  Snapshots are strictly
 * presentation data: nothing in them is ever read back by the simulation.
 */
#ifndef PORT_SNAPSHOT_H
#define PORT_SNAPSHOT_H

#include <stdint.h>
#include "allegro.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SNAP_ROOMS      32
#define SNAP_PARTICLES  512

typedef struct snap_room {
   int empty, start_tile, end_tile, level, sign, tiles;
} snap_room;

typedef struct snap_particle {
   fixed x, y;
   int color;
   int intensity;
} snap_particle;

typedef struct game_snapshot {
   uint64_t gen;               /* capture counter (matches the DL underlay marker) */
   uint64_t tick;              /* simulation tick at capture */
   uint64_t time_ns;           /* wall clock at capture */
   int valid;

   /* assets */
   DATAFILE *data;
   BITMAP *frame[15];          /* custom character frames */

   /* camera / world */
   int map_offset;             /* vertical scroll, monotonic */
   snap_room room[SNAP_ROOMS];
   int stripe_ids[5];          /* background stripe bitmaps (data index - 1) */
   int hurry_y;
   int hurry_visible;          /* hurry sign drawn this frame (flash option) */
   int fo, so;                 /* floor / sign sprite base indices */
   int debug_rows;

   /* player (game units; x,y are the feet as in the simulation) */
   double px, py;
   double psx, psy;
   int pose_bmp;               /* index into frame[], -1 = none */
   int pose_flip;              /* 1: horizontally flipped */
   int pose_ox, pose_oy;       /* sprite top-left relative to ((int)x, (int)y) */
   int rotating;               /* rotate_sprite path */
   fixed angle;
   int dead;
   int teleported;             /* new game / discontinuity: do not interpolate */

   /* particles */
   snap_particle stars[SNAP_PARTICLES];

   /* HUD */
   int score;                  /* level*10 + score */
   int in_combo;               /* combo meter height (0..100) */
   int combo_value;            /* number on the combo meter, -1 = not shown */
   int reward_time;
   fixed reward_scale;
   BITMAP *reward_bmp;
   int flash;                  /* eye-candy option */
   int clock_angle;
   int clock_cx, clock_cy;     /* shake of the clock body */
   int hand_cx, hand_cy;       /* shake of the clock hand */
   int frame_count;
   int logic_count;

   /* replay HUD */
   int replay;                 /* watching a replay (not recording) */
   int vcr_left, vcr_fire, vcr_right;
   int rec_pos, rec_size;
   int scroll_count;           /* replay title scroller position as drawn */
   char name[33];
   char comment[43];
   int custom_game;
   char cap_floors[64], cap_speed[64], cap_gravity[64];
} game_snapshot;

/* Store a new capture (game side: port/game/snapshot_capture.c). */
void snapshot_push(const game_snapshot *s);
/* Latest two snapshots (the renderer interpolates between them). */
const game_snapshot *snapshot_current(void);
const game_snapshot *snapshot_previous(void);
/* generation of the latest capture; 0 before the first */
uint64_t snapshot_generation(void);
/* Mark the next capture as discontinuous (new game, replay start). */
void snapshot_discontinuity(void);

#ifdef __cplusplus
}
#endif

#endif

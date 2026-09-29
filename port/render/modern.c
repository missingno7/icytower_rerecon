/*
 * Modern renderer.
 *
 * Coordinate domains (docs/port/ARCHITECTURE.md):
 *   game space    the historical 640x480 frame; the simulation's x range and
 *                 the 480-unit view height are game rules and never change;
 *   camera        world units -> output pixels.  The vertical field of view
 *                 stays 480 units; on wide outputs the horizontal view grows
 *                 (853 units at 16:9, ~1120 at 21:9) around the tower axis
 *                 x = 320 instead of pillar-boxing;
 *   presentation  the real drawable size; HUD elements are anchored to the
 *                 output edges (safe area), overlays use a centred 4:3
 *                 design canvas.
 *
 * Everything is drawn from original bitmaps at output resolution; there is
 * no 640x480 intermediate.  Interpolation between the last two simulation
 * snapshots is presentation-only (snapshots are never read by the game).
 */
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "port/render/modern.h"
#include "port/render/render.h"
#include "port/render/present.h"
#include "port/render/texcache.h"
#include "port/render/dlrender.h"
#include "port/sim/snapshot.h"
#include "port/sim/sched.h"
#include "port/platform/platform.h"

#define GAME_WIDTH     640   /* historical frame (game units) */
#define GAME_HEIGHT    480
#define GAME_CENTER_X  320   /* tower axis */

BITMAP *present_get_canvas(void);

static SDL_Renderer *g_r;

void modern_on_open(SDL_Renderer *r)
{
   g_r = r;
   texcache_init(r);
}

void modern_on_close(void)
{
   texcache_shutdown();
   g_r = NULL;
}

static int list_has_underlay(const A4_DL *dl)
{
   int i;
   if (!dl || !dl->valid)
      return 0;
   for (i = 0; i < dl->n; i++)
      if (dl->ops[i].kind == DLOP_UNDERLAY)
         return 1;
   return 0;
}

bool modern_scene_active(void)
{
   BITMAP *cv = present_get_canvas();
   const game_snapshot *s = snapshot_current();
   if (!cv || !s || !list_has_underlay(a4_dl_get(cv)))
      return false;
   /* animate only while the simulation is producing snapshots */
   return plat_ticks_ns() - s->time_ns < 100000000ull;
}

/* ------------------------------------------------------------ camera */

typedef struct view {
   int W, H;            /* output pixels */
   float s;             /* pixels per game unit */
   xform world;         /* game -> output */
   SDL_Rect world_clip; /* visible world area on the output */
   float left, right;   /* visible world x range (game units) */
   /* HUD anchoring */
   float u;             /* HUD pixels per design unit */
   float ax0, ax1;      /* output x of the left/right HUD anchors */
   float ay0, ay1;      /* output y of the top/bottom HUD anchors */
} view;

static void make_view(view *v, const xform *canvas, float shift_x, float shift_y)
{
   const render_config *rc = render_get_config();
   SDL_Rect safe;
   present_output_size(&v->W, &v->H);
   v->s = canvas->s;
   v->world.s = v->s;
   /* the tower axis sits at the centre of the canvas (and of the output) */
   v->world.ox = canvas->ox + shift_x * v->s;   /* x = 320 lands on the output centre */
   v->world.oy = canvas->oy + shift_y * v->s;
   if (rc->widescreen) {
      v->world_clip.x = 0;
      v->world_clip.w = v->W;
   } else {
      v->world_clip.x = (int)floorf(canvas->ox + 0.5f);
      v->world_clip.w = (int)floorf(canvas->ox + GAME_WIDTH * v->s + 0.5f) - v->world_clip.x;
   }
   v->world_clip.y = (int)floorf(canvas->oy + 0.5f);
   v->world_clip.h = (int)floorf(canvas->oy + GAME_HEIGHT * v->s + 0.5f) - v->world_clip.y;
   v->left = ((float)v->world_clip.x - v->world.ox) / v->s;
   v->right = ((float)(v->world_clip.x + v->world_clip.w) - v->world.ox) / v->s;

   v->u = rc->ui_scale > 0.0f ? v->s * rc->ui_scale : v->s;
   /* anchors: the visible world edges, pulled in by the safe area */
   v->ax0 = (float)v->world_clip.x;
   v->ax1 = (float)(v->world_clip.x + v->world_clip.w);
   v->ay0 = (float)v->world_clip.y + shift_y * v->s;
   v->ay1 = (float)(v->world_clip.y + v->world_clip.h) + shift_y * v->s;
   if (present_window() && SDL_GetWindowSafeArea(present_window(), &safe)) {
      float scale_x = 1.0f, scale_y = 1.0f;
      int ww, wh;
      SDL_GetWindowSize(present_window(), &ww, &wh);
      if (ww > 0 && wh > 0) {
         scale_x = (float)v->W / (float)ww;
         scale_y = (float)v->H / (float)wh;
      }
      if (safe.x * scale_x > v->ax0) v->ax0 = safe.x * scale_x;
      if ((safe.x + safe.w) * scale_x < v->ax1) v->ax1 = (safe.x + safe.w) * scale_x;
      if (safe.y * scale_y > v->ay0) v->ay0 = safe.y * scale_y;
      if ((safe.y + safe.h) * scale_y < v->ay1) v->ay1 = (safe.y + safe.h) * scale_y;
   }
}

/* HUD transforms: design coordinates of the historical frame, anchored */
static xform hud_left(const view *v) { xform t = { v->ax0, v->ay0, v->u }; return t; }
static xform hud_right(const view *v) { xform t = { v->ax1 - GAME_WIDTH * v->u, v->ay0, v->u }; return t; }
static xform hud_center(const view *v) { xform t = { (v->ax0 + v->ax1) * 0.5f - GAME_CENTER_X * v->u, v->ay0, v->u }; return t; }
static xform hud_bottom_left(const view *v) { xform t = { v->ax0, v->ay1 - GAME_HEIGHT * v->u, v->u }; return t; }
static xform hud_bottom_right(const view *v) { xform t = { v->ax1 - GAME_WIDTH * v->u, v->ay1 - GAME_HEIGHT * v->u, v->u }; return t; }

/* ------------------------------------------------------------ drawing */

static BITMAP *dat_bmp(const game_snapshot *s, int i)
{
   return s->data ? (BITMAP *)s->data[i].dat : NULL;
}

static const FONT *dat_font(const game_snapshot *s, int i)
{
   return s->data ? (const FONT *)s->data[i].dat : NULL;
}

static void sprite(dl_ctx *c, const xform *t, BITMAP *b, float x, float y, int flip, tex_variant v, float alpha)
{
   SDL_FRect src, dst;
   SDL_Texture *tex;
   if (!b)
      return;
   tex = texcache_get(b, v);
   if (!tex)
      return;
   src.x = 0; src.y = 0; src.w = (float)b->w; src.h = (float)b->h;
   dst = xf_rect(t, x, y, (float)b->w, (float)b->h);
   texcache_filter(tex);
   SDL_SetTextureColorMod(tex, 255, 255, 255);
   SDL_SetTextureAlphaModFloat(tex, alpha);
   if (flip)
      SDL_RenderTextureRotated(c->r, tex, &src, &dst, 0.0, NULL, SDL_FLIP_HORIZONTAL);
   else
      SDL_RenderTexture(c->r, tex, &src, &dst);
}

static void sprite_part(dl_ctx *c, const xform *t, BITMAP *b, int sx, int sy, int sw, int sh,
                        float x, float y, tex_variant v)
{
   SDL_FRect src, dst;
   SDL_Texture *tex;
   if (!b || sw <= 0 || sh <= 0)
      return;
   tex = texcache_get(b, v);
   if (!tex)
      return;
   src.x = (float)sx; src.y = (float)sy; src.w = (float)sw; src.h = (float)sh;
   dst = xf_rect(t, x, y, (float)sw, (float)sh);
   texcache_filter(tex);
   SDL_SetTextureColorMod(tex, 255, 255, 255);
   SDL_SetTextureAlphaModFloat(tex, 1.0f);
   SDL_RenderTexture(c->r, tex, &src, &dst);
}

/* rotate_sprite / rotate_scaled_sprite: (x, y) is the top-left of the
 * unrotated scaled sprite; rotation about its centre; 256 units per turn */
static void sprite_rot(dl_ctx *c, const xform *t, BITMAP *b, float x, float y, double angle_units, float scale)
{
   SDL_FRect src, dst;
   SDL_FPoint pv;
   SDL_Texture *tex;
   if (!b || scale <= 0.0f)
      return;
   tex = texcache_get(b, TEXV_MASKED);
   if (!tex)
      return;
   src.x = 0; src.y = 0; src.w = (float)b->w; src.h = (float)b->h;
   dst.x = t->ox + x * t->s;
   dst.y = t->oy + y * t->s;
   dst.w = (float)b->w * scale * t->s;
   dst.h = (float)b->h * scale * t->s;
   pv.x = dst.w * 0.5f;
   pv.y = dst.h * 0.5f;
   texcache_filter(tex);
   SDL_SetTextureColorMod(tex, 255, 255, 255);
   SDL_SetTextureAlphaModFloat(tex, 1.0f);
   SDL_RenderTextureRotated(c->r, tex, &src, &dst, angle_units * 360.0 / 256.0, &pv, SDL_FLIP_NONE);
}

static void text(dl_ctx *c, const xform *t, const FONT *f, const char *s, float x, float y, int color_mode, uint32_t rgb)
{
   dl_draw_text(c, t, f, s, x, y, color_mode, rgb, -1);
}

static float lerpf(float a, float b, float t) { return a + (b - a) * t; }

/* ------------------------------------------------------------ world */

/* Area outside the historical tower walls on wide screens.  The tower
 * interior (background stripes) and walls are the only world graphics the
 * game has; outside x = -57..697 the original frame never showed anything.
 * Draw a darker continuation of the brick walls there so wide screens show
 * a tower standing in its own masonry rather than black bars. */
static void draw_surroundings(dl_ctx *c, const view *v, const game_snapshot *s, float wall_y)
{
   BITMAP *wall = dat_bmp(s, 100);
   float x;
   int ls;
   if (!wall || v->left >= -57.0f)
      return;
   for (ls = -1; ls < 5; ls++) {
      float y = (float)(ls * 124) + wall_y;
      /* right side: repeat the wall outward */
      for (x = 565.0f + (float)wall->w; x < v->right; x += (float)wall->w)
         sprite(c, &v->world, wall, x, y, 0, TEXV_MASKED, 1.0f);
      /* left side: mirrored */
      for (x = -57.0f - (float)wall->w; x + (float)wall->w > v->left; x -= (float)wall->w)
         sprite(c, &v->world, wall, x, y, 1, TEXV_MASKED, 1.0f);
   }
   /* shade the outer masonry so the playfield stays the focus */
   {
      SDL_FRect r;
      SDL_SetRenderDrawBlendMode(c->r, SDL_BLENDMODE_BLEND);
      SDL_SetRenderDrawColor(c->r, 0, 0, 0, 110);
      r = xf_rect(&v->world, v->left, 0, -57.0f - v->left, GAME_HEIGHT);
      if (r.w > 0) SDL_RenderFillRect(c->r, &r);
      r = xf_rect(&v->world, 565.0f + (float)wall->w, 0, v->right - (565.0f + (float)wall->w), GAME_HEIGHT);
      if (r.w > 0) SDL_RenderFillRect(c->r, &r);
   }
}

typedef struct interp {
   const game_snapshot *a, *b;   /* previous, current */
   float t;                      /* 0..1 */
   int smooth;                   /* interpolation enabled for this frame */
   double off;                   /* interpolated map offset */
} interp;

static void draw_world(dl_ctx *c, const view *v, const interp *ip)
{
   const game_snapshot *s = ip->b, *p = ip->a;
   double off = ip->off;
   int ls, y, i;
   int ioff = s->map_offset;
   int base256 = ioff - (ioff % 256);
   int base16 = ioff - (ioff % 16);
   float wall_y;

   SDL_SetRenderClipRect(c->r, &v->world_clip);

   /* walls scroll faster than the floors (x1.476 per 84 units) */
   if (ip->smooth)
      wall_y = (float)(fmod(off, 84.0) * 1.476);
   else
      wall_y = (float)(int)((ioff % 84) * 1.476);
   draw_surroundings(c, v, s, wall_y);

   /* background stripes: 128-unit slots moving at half the scroll speed */
   for (ls = -1; ls < 5; ls++) {
      int id;
      float yy;
      if (ls < 4)
         id = s->stripe_ids[ls + 1];
      else if (ip->smooth && p && off < base256)
         id = p->stripe_ids[4];
      else
         continue;
      yy = (float)(ls * 128) + (ip->smooth ? (float)((off - base256) / 2.0) : (float)((ioff % 256) / 2));
      sprite(c, &v->world, dat_bmp(s, id + 1), 37.0f, yy, 0, TEXV_OPAQUE, 1.0f);
   }

   /* hurry-up sign (drawn behind the floors, as in draw_frame) */
   if (s->hurry_visible) {
      BITMAP *h = dat_bmp(s, 67);
      float hy = (float)s->hurry_y;
      if (ip->smooth && p && p->hurry_visible && p->hurry_y >= s->hurry_y && p->hurry_y - s->hurry_y < 10)
         hy = lerpf((float)p->hurry_y, (float)s->hurry_y, ip->t);
      if (h)
         sprite(c, &v->world, h, (float)(GAME_CENTER_X - h->w / 2), hy, 0, TEXV_MASKED, 1.0f);
   }

   /* floors and signs; rows move rigidly with the scroll offset */
   {
      float dy = ip->smooth ? (float)(off - ioff) : 0.0f;
      for (y = 31; y >= -1; y--) {
         const snap_room *rm;
         float cx;
         if (y >= 0) {
            rm = &s->room[y];
            cx = (float)(464 - y * 16 + (ioff % 16)) + dy;
         } else {
            /* the row that scrolled out at the bottom during this tick */
            if (!ip->smooth || !p || p->map_offset - (p->map_offset % 16) == base16 || off >= base16)
               continue;
            rm = &p->room[0];
            cx = (float)(464 + (p->map_offset % 16)) + (float)(off - p->map_offset);
         }
         if (!rm->empty) {
            int f = s->fo + rm->tiles * 3, x;
            if (f > 44) f = 44;
            if (rm->level > 4999) f += 3;
            x = rm->start_tile;
            sprite(c, &v->world, dat_bmp(s, f), (float)(x * 16 - 5), cx - 6.0f, 0, TEXV_MASKED, 1.0f);
            x++;
            while (x < rm->end_tile) {
               sprite(c, &v->world, dat_bmp(s, f + 1), (float)(x * 16), cx - 6.0f, 0, TEXV_MASKED, 1.0f);
               x++;
            }
            sprite(c, &v->world, dat_bmp(s, f + 2), (float)(x * 16), cx - 6.0f, 0, TEXV_MASKED, 1.0f);
         }
         if (rm->sign) {
            int sn = s->so + rm->tiles;
            BITMAP *sb;
            float sy = cx + 10.0f;
            float scx;
            char num[16];
            const FONT *f54 = dat_font(s, 54);
            if (sn > 110) sn = 110;
            if (rm->level > 4999) sn++;
            sb = dat_bmp(s, sn);
            scx = (float)((rm->start_tile + (rm->end_tile - rm->start_tile) / 2) * 16);
            sprite(c, &v->world, sb, scx, sy, 0, TEXV_MASKED, 1.0f);
            if (sb && f54) {
               float tx;
               snprintf(num, sizeof(num), "%d", rm->sign);
               scx += (float)(sb->w / 2);
               tx = scx - (float)(text_length(f54, num) / 2);
               text(c, &v->world, f54, num, tx + 1, sy + 7, 0, 0x373737u);
               text(c, &v->world, f54, num, tx + 2, sy + 6, 0, 0x373737u);
               text(c, &v->world, f54, num, tx, sy + 6, 0, 0x373737u);
               text(c, &v->world, f54, num, tx + 1, sy + 5, 0, 0x373737u);
               text(c, &v->world, f54, num, tx + 1, sy + 6, 0, 0xFFFFFFu);
            }
         }
      }
   }

   /* particles (screen-space in the historical frame) */
   for (i = 0; i < SNAP_PARTICLES; i++) {
      const snap_particle *q = &s->stars[i];
      float px, py;
      if (!q->intensity)
         continue;
      if (ip->smooth && p && p->stars[i].intensity == q->intensity + 1) {
         px = lerpf((float)p->stars[i].x, (float)q->x, ip->t) / 65536.0f;
         py = lerpf((float)p->stars[i].y, (float)q->y, ip->t) / 65536.0f;
      } else {
         px = (float)fixtoi(q->x);
         py = (float)fixtoi(q->y);
      }
      sprite(c, &v->world, dat_bmp(s, q->color + 117), px, py, 0, TEXV_MASKED, 1.0f);
   }

   /* player */
   if (s->pose_bmp >= 0 && s->frame[s->pose_bmp]) {
      BITMAP *pb = s->frame[s->pose_bmp];
      double px = s->px, py = s->py;
      if (ip->smooth && p && !s->teleported && fabs(p->px - s->px) < 40.0 && fabs(p->py - s->py) < 64.0) {
         px = p->px + (s->px - p->px) * ip->t;
         py = p->py + (s->py - p->py) * ip->t;
      }
      if (!ip->smooth) {
         px = (double)(int)px;
         py = (double)(int)py;
      }
      if (s->rotating) {
         double ang = (double)s->angle / 65536.0;
         if (ip->smooth && p && p->rotating && s->angle - p->angle > 0 && s->angle - p->angle <= itofix(16))
            ang = ((double)p->angle + (double)(s->angle - p->angle) * ip->t) / 65536.0;
         sprite_rot(c, &v->world, pb, (float)px + s->pose_ox, (float)py + s->pose_oy, ang, 1.0f);
      } else {
         sprite(c, &v->world, pb, (float)px + s->pose_ox, (float)py + s->pose_oy, s->pose_flip, TEXV_MASKED, 1.0f);
      }
   }

   /* walls in front */
   for (ls = -1; ls < 4; ls++) {
      BITMAP *wall = dat_bmp(s, 100);
      float yy = (float)(ls * 124) + wall_y;
      sprite(c, &v->world, wall, 565.0f, yy, 0, TEXV_MASKED, 1.0f);
      sprite(c, &v->world, wall, -57.0f, yy, 1, TEXV_MASKED, 1.0f);
   }
   SDL_SetRenderClipRect(c->r, NULL);
}

/* ------------------------------------------------------------ HUD */

static void draw_hud(dl_ctx *c, const view *v, const interp *ip)
{
   const game_snapshot *s = ip->b, *p = ip->a;
   xform L = hud_left(v), BL = hud_bottom_left(v), R = hud_right(v), BR = hud_bottom_right(v), C = hud_center(v);
   const FONT *f50 = dat_font(s, 50), *f52 = dat_font(s, 52), *f53 = dat_font(s, 53);
   char buf[256];

   /* combo meter */
   sprite(c, &L, dat_bmp(s, 16), 22, 100, 0, TEXV_MASKED, 1.0f);
   if (s->in_combo) {
      int h = s->in_combo;
      float hf = (float)h;
      if (ip->smooth && p && p->in_combo > s->in_combo && p->in_combo - s->in_combo <= 2)
         hf = lerpf((float)p->in_combo, (float)s->in_combo, ip->t);
      h = (int)(hf + 0.5f);
      sprite_part(c, &L, dat_bmp(s, 15), 0, 100 - h, 16, h, 33, (float)(219 - h), TEXV_OPAQUE);
   }
   if (s->combo_value >= 0) {
      sprite(c, &L, dat_bmp(s, 14), -8, 210, 0, TEXV_MASKED, 1.0f);
      if (f50) {
         snprintf(buf, sizeof(buf), "%d", s->combo_value);
         text(c, &L, f50, buf, (float)(42 - text_length(f50, buf) / 2), 210, -1, 0);
      }
   }

   /* clock */
   {
      BITMAP *hand = dat_bmp(s, 13);
      double ca = s->clock_angle;
      double units;
      sprite(c, &L, dat_bmp(s, 12), (float)(s->clock_cx + 6), (float)(s->clock_cy + 10), 0, TEXV_MASKED, 1.0f);
      if (ip->smooth && p && s->clock_angle > p->clock_angle && s->clock_angle - p->clock_angle <= 4)
         ca = p->clock_angle + (s->clock_angle - p->clock_angle) * (double)ip->t;
      units = s->clock_angle ? fmod(ca, 1500.0) * 0.1706666 : 0.0;
      if (!ip->smooth)
         units = s->clock_angle ? fixtof(ftofix((s->clock_angle % 1500) * 0.1706666)) : 0.0;
      if (hand)
         sprite_rot(c, &L, hand, (float)(s->hand_cx + 34), (float)(s->hand_cy + 28), units, 1.0f);
   }

   /* combo reward ("Good!", "Sweet!", ...) */
   if (s->reward_time && s->reward_bmp) {
      BITMAP *rb = s->reward_bmp;
      double sc = fixtof(s->reward_scale);
      if (ip->smooth && p && p->reward_time == s->reward_time + 1 && p->reward_bmp == rb)
         sc = fixtof(p->reward_scale) + (fixtof(s->reward_scale) - fixtof(p->reward_scale)) * ip->t;
      if (s->flash == 1) {
         float w = (float)(sc * rb->w), h = (float)(sc * rb->h);
         SDL_FRect src = { 0, 0, (float)rb->w, (float)rb->h };
         SDL_FRect dst = xf_rect(&C, (float)(320 - sc * 0.5 * rb->w), (float)(360 - sc * 0.5 * rb->h - sc * rb->h * 0.5), w, h);
         SDL_Texture *t = texcache_get(rb, TEXV_MASKED);
         if (t) {
            texcache_filter(t);
            SDL_SetTextureAlphaModFloat(t, 1.0f);
            SDL_RenderTexture(c->r, t, &src, &dst);
         }
      } else if (s->flash == 0) {
         /* rotate_scaled_sprite(bmp, reward, x, y, scale << 8, scale) */
         float x = (float)(320 - sc * 0.5 * rb->w);
         float y = (float)(360 - (sc * 120 - rb->h * sc * 0.5));
         sprite_rot(c, &C, rb, x, y, sc * 256.0, (float)sc);
      }
   }

   /* score */
   if (f52) {
      snprintf(buf, sizeof(buf), "score: %d", s->score);
      text(c, &BL, f52, buf, 8, 440, -1, 0);
   }

   /* replay HUD */
   if (s->replay && f53) {
      BITMAP *vcr = dat_bmp(s, 127);
      if (s->frame_count & 8) {
         int w = text_length(f53, "REPLAY");
         text(c, &R, f53, "REPLAY", (float)(630 - w + 1), 5, 0, 0x000000u);
         text(c, &R, f53, "REPLAY", (float)(630 - w), 4, 0, 0xFFFFFFu);
      }
      if (s->custom_game) {
         const char *caps[3] = { s->cap_floors, s->cap_speed, s->cap_gravity };
         int k;
         for (k = 0; k < 3; k++) {
            int w = text_length(f53, caps[k]);
            text(c, &R, f53, caps[k], (float)(630 - w + 1), (float)(16 + 10 * k), 0, 0x000000u);
            text(c, &R, f53, caps[k], (float)(630 - w), (float)(15 + 10 * k), 0, 0xFFFFFFu);
         }
      }
      if (vcr) {
         int x = 635 - vcr->w, y = 475 - vcr->h;
         int cx = y + 10, cy = x + 10;
         int len = s->rec_size > 0 ? s->rec_size : 1;
         int bar = s->rec_pos * 117 / len > 116 ? 116 : s->rec_pos * 117 / len;
         sprite(c, &BR, vcr, (float)x, (float)y, 0, TEXV_MASKED, 1.0f);
         if (s->vcr_left) sprite(c, &BR, dat_bmp(s, 128), (float)(x + 97), (float)(y + 5), 0, TEXV_MASKED, 1.0f);
         if (s->vcr_fire) sprite(c, &BR, dat_bmp(s, 130), (float)(x + 107), (float)(y + 5), 0, TEXV_MASKED, 1.0f);
         if (s->vcr_right) sprite(c, &BR, dat_bmp(s, 129), (float)(x + 117), (float)(y + 5), 0, TEXV_MASKED, 1.0f);
         /* scrolling title, clipped to the display window of the VCR */
         dl_set_clip(c, &BR, cy, 0, 624, 480);
         text(c, &BR, f53, s->name, (float)(x + 12 - s->scroll_count / 2), (float)(cx + 4), 0, 0x9696A0u);
         text(c, &BR, f53, s->name, (float)(x + 13 - s->scroll_count / 2), (float)(cx + 4), 0, 0xC8C8D2u);
         if (s->comment[0]) {
            int nl = text_length(f53, s->name);
            text(c, &BR, f53, " - ", (float)(x + 12 - s->scroll_count / 2 + nl), (float)(cx + 4), 0, 0xC8C8D2u);
            text(c, &BR, f53, s->comment, (float)(x + 30 - s->scroll_count / 2 + nl), (float)(cx + 4), 0, 0xC8C8D2u);
         }
         SDL_SetRenderClipRect(c->r, NULL);
         /* progress bar: rect(bmp, cy, cx + 20, cy + bar, cx + 19) */
         {
            SDL_FRect r1 = xf_rect(&BR, (float)cy, (float)(cx + 19), (float)(bar + 1), 1.0f);
            SDL_FRect r2 = xf_rect(&BR, (float)cy, (float)(cx + 20), (float)(bar + 1), 1.0f);
            SDL_SetRenderDrawBlendMode(c->r, SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(c->r, 50, 200, 50, 255);
            SDL_RenderFillRect(c->r, &r1);
            SDL_RenderFillRect(c->r, &r2);
         }
      }
   }
}

/* ------------------------------------------------------------ underlay */

static void underlay(dl_ctx *c, const xform *t, uint64_t gen)
{
   const render_config *rc = render_get_config();
   const game_snapshot *cur = snapshot_current(), *prev = snapshot_previous();
   interp ip;
   view v;
   float shift_x = (t->ox - c->t.ox) / t->s, shift_y = (t->oy - c->t.oy) / t->s;
   if (!cur)
      return;
   make_view(&v, &c->t, shift_x, shift_y);
   ip.b = cur;
   ip.a = prev;
   ip.smooth = rc->interpolation && prev && !cur->teleported && cur->gen == gen && !sched_is_virtual();
   ip.t = ip.smooth ? sched_alpha() : 1.0f;
   if (ip.smooth)
      ip.off = (double)prev->map_offset + (double)(cur->map_offset - prev->map_offset) * ip.t;
   else
      ip.off = cur->map_offset;
   if (ip.smooth && (cur->map_offset < prev->map_offset || cur->map_offset - prev->map_offset > 400)) {
      ip.smooth = 0;
      ip.t = 1.0f;
      ip.off = cur->map_offset;
   }
   draw_world(c, &v, &ip);
   draw_hud(c, &v, &ip);
}

/* ------------------------------------------------------------ frame */

/* The full-canvas opaque background of a menu screen (first operation). */
static BITMAP *find_background(const A4_DL *dl, int depth)
{
   const a4_dl_op *op;
   if (!dl || !dl->valid || dl->n == 0 || depth > 4)
      return NULL;
   op = &dl->ops[0];
   while (op->kind == DLOP_NONE && op < &dl->ops[dl->n - 1])
      op++;
   if (op->kind == DLOP_NESTED)
      return find_background(op->child, depth + 1);
   if (op->kind == DLOP_BITMAP && op->blend == DLB_SOLID && op->dx <= 0 && op->dy <= 0 &&
       op->dw >= (float)dl->w && op->dh >= (float)dl->h && op->src && (op->src->a4_flags & A4_BMP_STATIC))
      return op->src;   /* a loaded background image, never a canvas shown as pixels */
   return NULL;
}

/* Wide outputs showing a 4:3 menu: continue the menu's own background into
 * the side areas (mirrored, dimmed) instead of black bars. */
static void draw_menu_sides(SDL_Renderer *r, const A4_DL *dl, const xform *t, int W, int H)
{
   BITMAP *bg = find_background(dl, 0);
   SDL_Texture *tex;
   if (getenv("ITOWER_DEBUG_DL") && dl && dl->n) {
      static int cnt;
      if (++cnt % 120 == 0) {
         int i;
         for (i = 0; i < dl->n && i < 6; i++)
            SDL_Log("op%d kind=%d blend=%d dx=%g dy=%g dw=%g dh=%g src=%p flags=%x w=%d h=%d", i, dl->ops[i].kind, dl->ops[i].blend,
                    dl->ops[i].dx, dl->ops[i].dy, dl->ops[i].dw, dl->ops[i].dh, (void *)dl->ops[i].src,
                    dl->ops[i].src ? dl->ops[i].src->a4_flags : 0u, dl->w, dl->h);
      }
   }
   SDL_FRect src, dst;
   float cw, x;
   int k;
   if (!bg || !render_get_config()->widescreen)
      return;
   tex = texcache_get(bg, TEXV_OPAQUE);
   if (!tex)
      return;
   texcache_filter(tex);
   SDL_SetTextureColorMod(tex, 110, 110, 120);
   SDL_SetTextureAlphaModFloat(tex, 1.0f);
   src.x = 0; src.y = 0; src.w = (float)bg->w; src.h = (float)bg->h;
   cw = (float)bg->w * t->s;
   for (k = 1; ; k++) {
      int drawn = 0;
      dst.y = t->oy;
      dst.w = cw;
      dst.h = (float)bg->h * t->s;
      x = t->ox - cw * (float)k;
      if (x + cw > 0) {
         dst.x = x;
         SDL_RenderTextureRotated(r, tex, &src, &dst, 0, NULL, (k & 1) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
         drawn = 1;
      }
      x = t->ox + cw * (float)k;
      if (x < (float)W) {
         dst.x = x;
         SDL_RenderTextureRotated(r, tex, &src, &dst, 0, NULL, (k & 1) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
         drawn = 1;
      }
      if (!drawn)
         break;
   }
   SDL_SetTextureColorMod(tex, 255, 255, 255);
   (void)H;
}

bool modern_draw_frame(SDL_Renderer *r)
{
   BITMAP *cv = present_get_canvas();
   A4_DL *dl = cv ? a4_dl_get(cv) : NULL;
   dl_ctx c;
   int W, H;
   float s;
   if (getenv("ITOWER_DEBUG_DL")) {
      static int cnt;
      if (++cnt % 60 == 0)
         SDL_Log("frame: dl=%p valid=%d n=%d underlay=%d", (void *)dl, dl ? dl->valid : -1, dl ? dl->n : -1,
                 list_has_underlay(dl));
   }
   if (!dl || !dl->valid)
      return false;
   present_output_size(&W, &H);
   s = (float)W / (float)cv->w;
   if ((float)H / (float)cv->h < s)
      s = (float)H / (float)cv->h;
   memset(&c, 0, sizeof(c));
   c.r = r;
   c.t.s = s;
   c.t.ox = floorf(((float)W - (float)cv->w * s) * 0.5f + 0.5f);
   c.t.oy = floorf(((float)H - (float)cv->h * s) * 0.5f + 0.5f);
   c.bounds.x = 0;
   c.bounds.y = 0;
   c.bounds.w = W;
   c.bounds.h = H;
   c.alpha = 1.0f;
   c.underlay = underlay;
   c.extend_x = list_has_underlay(dl) && render_get_config()->widescreen;
   if (!list_has_underlay(dl)) {
      /* menus and dialogs: the historical 4:3 design canvas, drawn natively */
      draw_menu_sides(r, dl, &c.t, W, H);
      c.bounds.x = (int)c.t.ox;
      c.bounds.y = (int)c.t.oy;
      c.bounds.w = (int)floorf(cv->w * s + 0.5f);
      c.bounds.h = (int)floorf(cv->h * s + 0.5f);
   }
   dl_render(&c, dl);
   texcache_frame_end();
   return true;
}

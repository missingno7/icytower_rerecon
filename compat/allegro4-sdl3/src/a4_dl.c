/*
 * Display-list recording for canvas bitmaps.  See a4_dl.h for the model.
 */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "a4_internal.h"
#include "a4_dl.h"

#define DL_MAX_OPS 8192

static int g_enabled = 1;
static int g_static_depth;
static int g_canvas_w, g_canvas_h;
static BITMAP *g_hooked;          /* target of the op currently being recorded */
void (*a4_dl_texture_evict)(BITMAP *b);

void a4_dl_enable(int on) { g_enabled = on; }
int a4_dl_enabled(void) { return g_enabled; }

/* ---------------------------------------------------------------- lists */

static A4_DL *dl_new(int w, int h)
{
   A4_DL *dl = (A4_DL *)calloc(1, sizeof(A4_DL));
   if (!dl)
      return NULL;
   dl->refs = 1;
   dl->valid = 1;
   dl->w = w;
   dl->h = h;
   return dl;
}

static void src_ref(BITMAP *b)
{
   if (b)
      b->a4_refs++;
}

void a4_real_destroy_bitmap(BITMAP *b);

static void src_unref(BITMAP *b)
{
   if (!b)
      return;
   if (--b->a4_refs <= 0) {
      b->a4_refs = 0;
      if (b->a4_flags & 0x8000u)   /* destroyed while referenced */
         a4_real_destroy_bitmap(b);
   }
}

static void op_release(a4_dl_op *op)
{
   if (op->kind == DLOP_BITMAP)
      src_unref(op->src);
   else if (op->kind == DLOP_NESTED)
      a4_dl_unref(op->child);
   else if (op->kind == DLOP_TEXT)
      free(op->text);
}

static void dl_clear_ops(A4_DL *dl)
{
   int i;
   for (i = 0; i < dl->n; i++)
      op_release(&dl->ops[i]);
   dl->n = 0;
   dl->dead = 0;
   dl->shift_x = dl->shift_y = 0;
   dl->valid = 1;
   dl->version++;
}

void a4_dl_ref(A4_DL *dl)
{
   if (dl)
      dl->refs++;
}

void a4_dl_unref(A4_DL *dl)
{
   if (!dl || --dl->refs > 0)
      return;
   dl_clear_ops(dl);
   free(dl->ops);
   free(dl);
}

static A4_DL *dl_clone(const A4_DL *src)
{
   int i;
   A4_DL *dl = dl_new(src->w, src->h);
   if (!dl)
      return NULL;
   if (src->n) {
      dl->ops = (a4_dl_op *)malloc(sizeof(a4_dl_op) * (size_t)src->n);
      if (!dl->ops) {
         free(dl);
         return NULL;
      }
      dl->cap = src->n;
      memcpy(dl->ops, src->ops, sizeof(a4_dl_op) * (size_t)src->n);
      dl->n = src->n;
      for (i = 0; i < dl->n; i++) {
         a4_dl_op *op = &dl->ops[i];
         if (op->kind == DLOP_BITMAP) src_ref(op->src);
         else if (op->kind == DLOP_NESTED) a4_dl_ref(op->child);
         else if (op->kind == DLOP_TEXT && op->text) op->text = strdup(op->text);
      }
   }
   dl->valid = src->valid;
   dl->shift_x = src->shift_x;
   dl->shift_y = src->shift_y;
   dl->dead = src->dead;
   dl->version = src->version + 1;
   return dl;
}

/* the list of `b`, made private so it can be modified */
static A4_DL *writable(BITMAP *b)
{
   A4_DL *dl = b->a4_dl;
   if (dl && dl->refs > 1) {
      A4_DL *c = dl_clone(dl);
      if (!c)
         return NULL;
      dl->refs--;
      b->a4_dl = c;
      dl = c;
   }
   return dl;
}

A4_DL *a4_dl_get(BITMAP *b) { return b ? b->a4_dl : NULL; }

/* ---------------------------------------------------------------- lifetime */

void a4_dl_begin_static(void) { g_static_depth++; }
void a4_dl_end_static(void) { if (g_static_depth > 0) g_static_depth--; }

void a4_dl_mark_canvas(BITMAP *b)
{
   if (!b || b->parent)
      return;
   if (!g_canvas_w) {
      g_canvas_w = b->w;
      g_canvas_h = b->h;
   }
   if (!b->a4_dl)
      b->a4_dl = dl_new(b->w, b->h);
   b->a4_flags |= A4_BMP_CANVAS;
}

void a4_dl_bitmap_created(BITMAP *b)
{
   if (g_static_depth) {
      b->a4_flags |= A4_BMP_STATIC;
      return;
   }
   if (g_canvas_w && b->w == g_canvas_w && b->h == g_canvas_h && !b->parent)
      a4_dl_mark_canvas(b);
}

/* Returns nonzero when the caller may free the bitmap now. */
int a4_dl_bitmap_release(BITMAP *b)
{
   if (b->a4_dl) {
      a4_dl_unref(b->a4_dl);
      b->a4_dl = NULL;
   }
   if (b->a4_refs > 0) {
      b->a4_flags |= 0x8000u;   /* free when the last list lets go */
      return 0;
   }
   if (a4_dl_texture_evict)
      a4_dl_texture_evict(b);
   return 1;
}

void a4_dl_bitmap_dying(BITMAP *b) { (void)b; }

/* ---------------------------------------------------------------- helpers */

uint32_t a4_dl_rgb(int depth, unsigned long c)
{
   if (depth == 32 || depth == 24)
      return (uint32_t)c & 0xFFFFFFu;
   return ((uint32_t)getr_depth(depth, (int)c) << 16) | ((uint32_t)getg_depth(depth, (int)c) << 8) |
          (uint32_t)getb_depth(depth, (int)c);
}

void a4_dl_invalidate(BITMAP *b)
{
   A4_DL *dl;
   if (!b)
      return;
   if (b->parent)
      b = b->parent;
   dl = b->a4_dl;
   if (!dl || !dl->valid)
      return;
   if (getenv("ITOWER_DEBUG_DL"))
      fprintf(stderr, "dl invalidate %p (%dx%d) from %p\n", (void *)b, b->w, b->h, __builtin_return_address(0));
   dl = writable(b);
   if (!dl)
      return;
   dl_clear_ops(dl);
   dl->valid = 0;
}

/* Is `dst` a recording target?  Sub-bitmaps of canvases invalidate their
 * parent (their drawing is not recorded). */
static A4_DL *target(BITMAP *dst)
{
   A4_DL *dl;
   g_hooked = dst;
   if (!dst)
      return NULL;
   if (dst->parent) {
      if (dst->parent->a4_dl)
         a4_dl_invalidate(dst->parent);
      return NULL;
   }
   if (!dst->a4_dl)
      return NULL;
   if (!g_enabled) {
      if (dst->a4_dl->valid)
         a4_dl_invalidate(dst);
      return NULL;
   }
   if (!dst->a4_dl->valid)
      return NULL;
   dl = writable(dst);
   return dl;
}

/* ---------------------------------------------------------------- pruning */

/* destination footprint of a completed op, clipped (inclusive; empty when
 * x2 < x1) */
static void op_bounds(a4_dl_op *op)
{
   int x1, y1, x2, y2;
   switch (op->kind) {
      case DLOP_FILL:
         x1 = op->x1; y1 = op->y1; x2 = op->x2; y2 = op->y2;
         break;
      case DLOP_LINE:
         x1 = op->x1 < op->x2 ? op->x1 : op->x2; x2 = op->x1 < op->x2 ? op->x2 : op->x1;
         y1 = op->y1 < op->y2 ? op->y1 : op->y2; y2 = op->y1 < op->y2 ? op->y2 : op->y1;
         break;
      case DLOP_TEXT:
         x1 = op->x1; y1 = op->y1;
         x2 = x1 + text_length(op->font, op->text) - 1;
         y2 = y1 + text_height(op->font) - 1;
         break;
      case DLOP_BITMAP:
      case DLOP_NESTED:
         if (op->angle != 0.0f) {
            /* rotated about (px, py): any point lies within the rectangle's
               farthest-corner distance of the pivot */
            float ex = op->dx - op->px, ey = op->dy - op->py, fx = ex + op->dw, fy = ey + op->dh;
            float r2 = 0, v;
            v = ex * ex + ey * ey; if (v > r2) r2 = v;
            v = fx * fx + ey * ey; if (v > r2) r2 = v;
            v = ex * ex + fy * fy; if (v > r2) r2 = v;
            v = fx * fx + fy * fy; if (v > r2) r2 = v;
            v = (float)sqrt((double)r2) + 1.0f;
            x1 = (int)floor(op->px - v); y1 = (int)floor(op->py - v);
            x2 = (int)ceil(op->px + v); y2 = (int)ceil(op->py + v);
         } else {
            x1 = (int)floor(op->dx); y1 = (int)floor(op->dy);
            x2 = (int)ceil(op->dx + op->dw) - 1; y2 = (int)ceil(op->dy + op->dh) - 1;
         }
         break;
      default:   /* UNDERLAY: the whole canvas; never pruned */
         x1 = -32768; y1 = -32768; x2 = 32767; y2 = 32767;
         break;
   }
   if (x1 < op->cl) x1 = op->cl;
   if (y1 < op->ct) y1 = op->ct;
   if (x2 > op->cr - 1) x2 = op->cr - 1;
   if (y2 > op->cb - 1) y2 = op->cb - 1;
   op->bx1 = (int16_t)(x1 < -32768 ? -32768 : x1 > 32767 ? 32767 : x1);
   op->by1 = (int16_t)(y1 < -32768 ? -32768 : y1 > 32767 ? 32767 : y1);
   op->bx2 = (int16_t)(x2 < -32768 ? -32768 : x2 > 32767 ? 32767 : x2);
   op->by2 = (int16_t)(y2 < -32768 ? -32768 : y2 > 32767 ? 32767 : y2);
   op->has_bounds = 1;
}

/* rectangle every pixel of which `k` paints opaquely; 0 if none */
static int op_cover(const a4_dl_op *k, int *x1, int *y1, int *x2, int *y2)
{
   int ok = 0;
   switch (k->kind) {
      case DLOP_FILL:
         ok = k->blend == DLB_SOLID;
         break;
      case DLOP_LINE:
         ok = k->blend == DLB_SOLID && (k->x1 == k->x2 || k->y1 == k->y2);
         break;
      case DLOP_TEXT:
         ok = k->text_bg >= 0;   /* glyph cells filled with the background */
         break;
      case DLOP_BITMAP:
         if (k->blend == DLB_SOLID && k->angle == 0.0f && k->src && !(k->src->a4_flags & A4_BMP_CANVAS)) {
            /* only whole destination pixels count */
            *x1 = (int)ceil(k->dx); *y1 = (int)ceil(k->dy);
            *x2 = (int)floor(k->dx + k->dw) - 1; *y2 = (int)floor(k->dy + k->dh) - 1;
            if (*x1 < k->cl) *x1 = k->cl;
            if (*y1 < k->ct) *y1 = k->ct;
            if (*x2 > k->cr - 1) *x2 = k->cr - 1;
            if (*y2 > k->cb - 1) *y2 = k->cb - 1;
            return *x2 >= *x1 && *y2 >= *y1;
         }
         return 0;
      default:
         return 0;
   }
   if (!ok)
      return 0;
   *x1 = k->bx1; *y1 = k->by1; *x2 = k->bx2; *y2 = k->by2;
   return *x2 >= *x1 && *y2 >= *y1;
}

/* `k` redraws exactly the pixels `j` drew, with the same values */
static int op_same(const a4_dl_op *j, const a4_dl_op *k)
{
   if (j->kind != k->kind || j->cl != k->cl || j->ct != k->ct || j->cr != k->cr || j->cb != k->cb)
      return 0;
   if (k->kind == DLOP_BITMAP)
      return (k->blend == DLB_SOLID || k->blend == DLB_MASKED) && j->blend == k->blend && j->src == k->src &&
             j->src_gen == k->src_gen && j->flip == k->flip && j->angle == k->angle &&
             j->sx == k->sx && j->sy == k->sy && j->sw == k->sw && j->sh == k->sh &&
             j->dx == k->dx && j->dy == k->dy && j->dw == k->dw && j->dh == k->dh &&
             j->px == k->px && j->py == k->py;
   if (k->kind == DLOP_TEXT)
      return j->font == k->font && j->x1 == k->x1 && j->y1 == k->y1 && j->rgb == k->rgb &&
             j->text_color_mode == k->text_color_mode && j->text_bg == k->text_bg &&
             j->text && k->text && strcmp(j->text, k->text) == 0;
   if (k->kind == DLOP_LINE || k->kind == DLOP_FILL)
      return k->blend == DLB_SOLID && j->blend == DLB_SOLID && j->x1 == k->x1 && j->y1 == k->y1 &&
             j->x2 == k->x2 && j->y2 == k->y2 && j->rgb == k->rgb;
   return 0;
}

static void op_kill(A4_DL *dl, a4_dl_op *j)
{
   op_release(j);
   memset(j, 0, sizeof(*j));
   j->kind = DLOP_NONE;
   dl->dead++;
}

/* The last op of `dl` is complete: drop every earlier op whose pixels it
 * overwrites entirely.  Ops between the two cannot depend on the dropped
 * one outside its footprint, and inside it their result is overwritten too,
 * so the image is unchanged.  A translucent fill covering an op fades it;
 * once its remaining contribution is below half a colour step it goes. */
static void prune_last(A4_DL *dl)
{
   a4_dl_op *k = &dl->ops[dl->n - 1];
   int i, cx1 = 0, cy1 = 0, cx2 = -1, cy2 = -1, cover, fade;
   float keep = 1.0f;
   if (k->kind == DLOP_NONE || k->has_bounds)
      return;
   op_bounds(k);
   if (dl->shift_x || dl->shift_y)
      return;   /* shaken list: earlier ops are drawn shifted, keep them all */
   cover = op_cover(k, &cx1, &cy1, &cx2, &cy2);
   fade = k->kind == DLOP_FILL && k->blend == DLB_TRANS && k->bx2 >= k->bx1 && k->by2 >= k->by1;
   if (fade)
      keep = 1.0f - (float)k->alpha / 255.0f;
   if (!cover && !fade && k->kind != DLOP_BITMAP && k->kind != DLOP_TEXT && k->kind != DLOP_LINE)
      return;
   for (i = 0; i < dl->n - 1; i++) {
      a4_dl_op *j = &dl->ops[i];
      if (j->kind == DLOP_NONE || j->kind == DLOP_UNDERLAY || !j->has_bounds)
         continue;
      if (j->bx2 < j->bx1 || j->by2 < j->by1) {
         op_kill(dl, j);                       /* clipped away entirely */
      } else if (cover && j->bx1 >= cx1 && j->by1 >= cy1 && j->bx2 <= cx2 && j->by2 <= cy2 &&
                 !(j->kind == DLOP_LINE && j->x1 == j->x2 && j->by1 <= 0 && j->by2 >= dl->h - 1 &&
                   (cx1 > 0 || cx2 < dl->w - 1))) {
         /* (full-height vertical lines are also repeated past the 4:3 edges on
            wide outputs; only a full-width cover hides those repeats) */
         op_kill(dl, j);
      } else if (op_same(j, k)) {
         op_kill(dl, j);
      } else if (fade && j->bx1 >= k->bx1 && j->by1 >= k->by1 && j->bx2 <= k->bx2 && j->by2 <= k->by2) {
         j->vis *= keep;
         if (j->vis < 0.5f / 255.0f)
            op_kill(dl, j);
      }
   }
}

static void compact(A4_DL *dl)
{
   int i, n = 0;
   for (i = 0; i < dl->n; i++)
      if (dl->ops[i].kind != DLOP_NONE)
         dl->ops[n++] = dl->ops[i];
   dl->n = n;
   dl->dead = 0;
}

static a4_dl_op *push(BITMAP *dst, A4_DL *dl, int kind)
{
   a4_dl_op *op;
   if (dl->n > 0)
      prune_last(dl);
   if (dl->dead > 64 && dl->dead * 4 > dl->n)
      compact(dl);
   if (dl->n >= DL_MAX_OPS) {
      a4_dl_invalidate(dst);
      return NULL;
   }
   if (dl->n == dl->cap) {
      int ncap = dl->cap ? dl->cap * 2 : 64;
      a4_dl_op *n = (a4_dl_op *)realloc(dl->ops, sizeof(a4_dl_op) * (size_t)ncap);
      if (!n) {
         a4_dl_invalidate(dst);
         return NULL;
      }
      dl->ops = n;
      dl->cap = ncap;
   }
   op = &dl->ops[dl->n++];
   memset(op, 0, sizeof(*op));
   op->kind = (uint8_t)kind;
   op->vis = 1.0f;
   if (dst->clip) {
      op->cl = (int16_t)dst->cl; op->ct = (int16_t)dst->ct;
      op->cr = (int16_t)dst->cr; op->cb = (int16_t)dst->cb;
   } else {
      op->cl = 0; op->ct = 0; op->cr = (int16_t)dst->w; op->cb = (int16_t)dst->h;
   }
   dl->version++;
   return op;
}

static int full_clip(BITMAP *b)
{
   return !b->clip || (b->cl == 0 && b->ct == 0 && b->cr == b->w && b->cb == b->h);
}

/* 8-bit sources drawn onto truecolour canvases are converted by Allegro with
 * the palette selected at draw time (the loading screen selects the logo's
 * palette only around its draw_sprite).  The list must not depend on the
 * palette at render time, so such sources are recorded as a copy converted
 * now.  Masked draws keep index 0 transparent (the canvas mask colour). */
#define PAL_CACHE 16
static struct {
   uint32_t serial, gen, pal;
   int depth, masked;
   BITMAP *conv;
} g_pal_cache[PAL_CACHE];
static int g_pal_next;

static uint32_t palette_hash(void)
{
   const unsigned char *p = (const unsigned char *)a4_current_palette;
   uint32_t h = 2166136261u;
   size_t i;
   for (i = 0; i < sizeof(PALETTE); i++)
      h = (h ^ p[i]) * 16777619u;
   return h;
}

static BITMAP *record_source(BITMAP *src, BITMAP *dst, int masked)
{
   uint32_t pal;
   int i, x, y, depth;
   BITMAP *c;
   if (!src || src->depth != 8 || dst->depth == 8)
      return src;
   depth = dst->depth;
   pal = palette_hash();
   for (i = 0; i < PAL_CACHE; i++)
      if (g_pal_cache[i].conv && g_pal_cache[i].serial == src->serial && g_pal_cache[i].gen == src->generation &&
          g_pal_cache[i].pal == pal && g_pal_cache[i].depth == depth && g_pal_cache[i].masked == masked)
         return g_pal_cache[i].conv;
   a4_dl_begin_static();
   c = create_bitmap_ex(depth, src->w, src->h);
   a4_dl_end_static();
   if (!c)
      return src;
   for (y = 0; y < src->h; y++)
      for (x = 0; x < src->w; x++) {
         unsigned long v = a4_get_raw(src, x, y);
         a4_put_raw(c, x, y, masked && v == 0 ? a4_mask_color(depth) : a4_convert_color(v, 8, depth));
      }
   i = g_pal_next;
   g_pal_next = (g_pal_next + 1) % PAL_CACHE;
   if (g_pal_cache[i].conv) {
      BITMAP *old = g_pal_cache[i].conv;
      g_pal_cache[i].conv = NULL;
      src_unref(old);            /* freed now, or with the last list op using it */
   }
   /* the cache holds one reference; destroy_bitmap then defers the free until
      the cache and every list that recorded it have let go */
   src_ref(c);
   destroy_bitmap(c);
   g_pal_cache[i].serial = src->serial;
   g_pal_cache[i].gen = src->generation;
   g_pal_cache[i].pal = pal;
   g_pal_cache[i].depth = depth;
   g_pal_cache[i].masked = masked;
   g_pal_cache[i].conv = c;
   return c;
}

/* ---------------------------------------------------------------- recording */

void a4_dl_blit(BITMAP *src, BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h, int blend)
{
   A4_DL *dl = target(dst);
   a4_dl_op *op;
   if (!dl || !src)
      return;
   if (a4_root(src) == dst) {
      /* self copy: the game's screen shake blits the frame onto itself
       * shifted vertically; anything else is not representable */
      if (!src->parent && blend == DLB_SOLID && sx == 0 && dx == 0 && w >= dst->w && h >= dst->h - (sy > dy ? sy - dy : dy - sy)) {
         dl->shift_x += dx - sx;
         dl->shift_y += dy - sy;
         dl->version++;
      } else {
         a4_dl_invalidate(dst);
      }
      return;
   }
   if (src->a4_dl && !src->parent) {
      if (!src->a4_dl->valid) {
         /* the source canvas has no usable list: draw its pixels */
      } else if (blend == DLB_SOLID && sx == 0 && dx == 0 && w >= src->w && h >= src->h - (sy > 0 ? sy : 0) &&
                 src->w == dst->w && src->h == dst->h && full_clip(dst)) {
         /* whole-canvas copy (possibly shifted): share the list */
         A4_DL *s = src->a4_dl;
         int shx = s->shift_x + dx - sx, shy = s->shift_y + dy - sy;
         a4_dl_ref(s);
         a4_dl_unref(dst->a4_dl);
         dst->a4_dl = s;
         if (shx != s->shift_x || shy != s->shift_y) {
            dl = writable(dst);
            if (dl) {
               dl->shift_x = shx;
               dl->shift_y = shy;
               dl->version++;
            }
         }
         return;
      } else {
         op = push(dst, dl, DLOP_NESTED);
         if (!op)
            return;
         a4_dl_ref(src->a4_dl);
         op->child = src->a4_dl;
         op->blend = (uint8_t)blend;
         op->alpha = a4_blend_a;
         op->sx = sx; op->sy = sy; op->sw = w; op->sh = h;
         op->dx = (float)dx; op->dy = (float)dy; op->dw = (float)w; op->dh = (float)h;
         return;
      }
   }
   src = record_source(src, dst, blend != DLB_SOLID);
   /* an opaque copy that covers the whole target hides everything before it */
   if (blend == DLB_SOLID && full_clip(dst) && dx <= 0 && dy <= 0 && sx >= 0 && sy >= 0 &&
       dx + w >= dst->w && dy + h >= dst->h && sx - dx + dst->w <= src->w && sy - dy + dst->h <= src->h)
      dl_clear_ops(dl);
   op = push(dst, dl, DLOP_BITMAP);
   if (!op)
      return;
   src_ref(src);
   op->src = src;
   op->src_gen = src->generation;
   op->blend = (uint8_t)blend;
   op->alpha = a4_blend_a;
   op->sx = sx; op->sy = sy; op->sw = w; op->sh = h;
   op->dx = (float)dx; op->dy = (float)dy; op->dw = (float)w; op->dh = (float)h;
}

void a4_dl_sprite(BITMAP *dst, BITMAP *spr, int x, int y, int flip, int blend)
{
   A4_DL *dl;
   a4_dl_op *op;
   if (spr && spr->a4_dl && !spr->parent && spr->a4_dl->valid && flip == 0) {
      /* a canvas drawn as a sprite (fades: draw_sprite(tmp, swap_screen)) */
      a4_dl_blit(spr, dst, 0, 0, x, y, spr->w, spr->h, blend == DLB_MASKED && x == 0 && y == 0 ? DLB_SOLID : blend);
      return;
   }
   dl = target(dst);
   if (!dl || !spr)
      return;
   spr = record_source(spr, dst, blend != DLB_SOLID);
   op = push(dst, dl, DLOP_BITMAP);
   if (!op)
      return;
   src_ref(spr);
   op->src = spr;
   op->src_gen = spr->generation;
   op->blend = (uint8_t)blend;
   op->alpha = a4_blend_a;
   op->flip = (uint8_t)flip;
   op->sx = 0; op->sy = 0; op->sw = spr->w; op->sh = spr->h;
   op->dx = (float)x; op->dy = (float)y; op->dw = (float)spr->w; op->dh = (float)spr->h;
}

void a4_dl_stretch(BITMAP *src, BITMAP *dst, int sx, int sy, int sw, int sh,
                   int dx, int dy, int dw, int dh, int masked)
{
   A4_DL *dl = target(dst);
   a4_dl_op *op;
   if (!dl || !src)
      return;
   if (a4_root(src) == dst || (src->a4_dl && src->a4_dl->valid && !src->parent)) {
      /* zooming a canvas (debug views) is not represented */
      a4_dl_invalidate(dst);
      return;
   }
   src = record_source(src, dst, masked);
   op = push(dst, dl, DLOP_BITMAP);
   if (!op)
      return;
   src_ref(src);
   op->src = src;
   op->src_gen = src->generation;
   op->blend = masked ? DLB_MASKED : DLB_SOLID;
   op->sx = sx; op->sy = sy; op->sw = sw; op->sh = sh;
   op->dx = (float)dx; op->dy = (float)dy; op->dw = (float)dw; op->dh = (float)dh;
}

void a4_dl_rotate(BITMAP *dst, BITMAP *spr, int x, int y, fixed angle, fixed scale)
{
   A4_DL *dl = target(dst);
   a4_dl_op *op;
   float s = (float)scale / 65536.0f;
   if (!dl || !spr)
      return;
   if (spr->a4_dl && spr->a4_dl->valid && !spr->parent) {
      a4_dl_invalidate(dst);
      return;
   }
   spr = record_source(spr, dst, 1);
   op = push(dst, dl, DLOP_BITMAP);
   if (!op)
      return;
   src_ref(spr);
   op->src = spr;
   op->src_gen = spr->generation;
   op->blend = DLB_MASKED;
   op->sx = 0; op->sy = 0; op->sw = spr->w; op->sh = spr->h;
   /* Allegro rotates about the sprite centre; (x,y) is where the unrotated,
    * scaled sprite's top-left corner would be */
   op->dw = spr->w * s;
   op->dh = spr->h * s;
   op->dx = (float)x;
   op->dy = (float)y;
   op->px = x + op->dw * 0.5f;
   op->py = y + op->dh * 0.5f;
   /* Allegro angles: 256 units per turn, clockwise on screen */
   op->angle = (float)((double)angle * (360.0 / 256.0) / 65536.0);
}

void a4_dl_fill(BITMAP *dst, int x1, int y1, int x2, int y2, int color)
{
   A4_DL *dl;
   a4_dl_op *op;
   int t;
   if (a4_draw_mode == DRAW_MODE_XOR) {
      g_hooked = dst;
      a4_dl_invalidate(dst);
      return;
   }
   dl = target(dst);
   if (!dl)
      return;
   if (x2 < x1) { t = x1; x1 = x2; x2 = t; }
   if (y2 < y1) { t = y1; y1 = y2; y2 = t; }
   if (a4_draw_mode != DRAW_MODE_TRANS && full_clip(dst) && x1 <= 0 && y1 <= 0 &&
       x2 >= dst->w - 1 && y2 >= dst->h - 1) {
      /* opaque full cover: everything before it is invisible */
      dl_clear_ops(dl);
   }
   op = push(dst, dl, DLOP_FILL);
   if (!op)
      return;
   op->x1 = x1; op->y1 = y1; op->x2 = x2; op->y2 = y2;
   op->rgb = a4_dl_rgb(dst->depth, (unsigned long)(uint32_t)color);
   op->blend = (a4_draw_mode == DRAW_MODE_TRANS && dst->depth != 8) ? DLB_TRANS : DLB_SOLID;
   op->alpha = a4_blend_a;
}

void a4_dl_line(BITMAP *dst, int x1, int y1, int x2, int y2, int color)
{
   A4_DL *dl;
   a4_dl_op *op;
   if (a4_draw_mode == DRAW_MODE_XOR) {
      g_hooked = dst;
      a4_dl_invalidate(dst);
      return;
   }
   dl = target(dst);
   if (!dl)
      return;
   op = push(dst, dl, DLOP_LINE);
   if (!op)
      return;
   op->x1 = x1; op->y1 = y1; op->x2 = x2; op->y2 = y2;
   op->rgb = a4_dl_rgb(dst->depth, (unsigned long)(uint32_t)color);
   op->blend = (a4_draw_mode == DRAW_MODE_TRANS && dst->depth != 8) ? DLB_TRANS : DLB_SOLID;
   op->alpha = a4_blend_a;
}

void a4_dl_text(BITMAP *dst, const FONT *f, const char *s, int x, int y, int color, int bg)
{
   A4_DL *dl = target(dst);
   a4_dl_op *op;
   if (!dl || !f || !s)
      return;
   op = push(dst, dl, DLOP_TEXT);
   if (!op)
      return;
   op->font = f;
   op->text = strdup(s);
   op->x1 = x;
   op->y1 = y;
   op->text_color_mode = color < 0 ? -1 : 0;
   op->rgb = color < 0 ? 0 : a4_dl_rgb(dst->depth, (unsigned long)(uint32_t)color);
   op->text_bg = bg < 0 ? -1 : (int32_t)a4_dl_rgb(dst->depth, (unsigned long)(uint32_t)bg);
}

void a4_dl_clear(BITMAP *dst, int color)
{
   A4_DL *dl = target(dst);
   a4_dl_op *op;
   if (!dl)
      return;
   if (full_clip(dst))
      dl_clear_ops(dl);
   op = push(dst, dl, DLOP_FILL);
   if (!op)
      return;
   op->x1 = dst->cl; op->y1 = dst->ct; op->x2 = dst->cr - 1; op->y2 = dst->cb - 1;
   op->rgb = a4_dl_rgb(dst->depth, (unsigned long)(uint32_t)color);
   op->blend = DLB_SOLID;
}

void a4_dl_set_underlay(BITMAP *dst, uint64_t gen)
{
   A4_DL *dl;
   a4_dl_op *op;
   if (!dst || !dst->a4_dl || !g_enabled)
      return;
   dl = writable(dst);
   if (!dl)
      return;
   dl_clear_ops(dl);
   op = push(dst, dl, DLOP_UNDERLAY);
   if (op)
      op->underlay = gen;
}

/* a4_touch() consults this: a canvas touched by an op that was not
 * recorded loses its list (safety net for unhooked drawing paths). */
void a4_dl_check_touch(BITMAP *b)
{
   if (b->parent) {
      if (b->parent->a4_dl && b->parent->a4_dl->valid)
         a4_dl_invalidate(b->parent);
   } else if (b->a4_dl && b->a4_dl->valid && g_hooked != b) {
      a4_dl_invalidate(b);
   }
   g_hooked = NULL;
}

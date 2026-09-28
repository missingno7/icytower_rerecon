/* Historical CU: F:\projects\icytower\trunk\source\player.c
 * Ownership: GAME. Other entities remain unrecovered.
 * jump_player @ 0x00418678, 198 bytes: code equal, literal pool placement pending
 * update_player @ 0x00418740, 651 bytes: reconstructed from disassembly; 647 bytes, only the status==2 epilogue copy differs
 */

#include <allegro.h>
#include "recovered_types.h"
#include "port/sim/x87.h"

/* Historical .data objects of this CU (DWARF lines 14 and 16, verifier bytes at
 * 0x4bdb80 and 0x4bdba8). Indexed by collision_type and the gravity mode. */
double max_speed[5] = { 12.0, 12.0, 12.2, 12.2, 12.0 };
double gravity_modifier[3] = { -0.2, 0.0, 0.2 };

/* DWARF names the second parameter cheat.  The normal jump preserves the
 * original two-path x87 expression instead of reducing it to fabs(sx). */
extern int collision_type;
#include "recovered/Treplay.h"
extern Treplay *get_demo(void);

/* Forward declarations; definitions follow in their original source order. */
void reset_player(Tplayer *p);
void update_player(Tplayer *p);
int jump_player(Tplayer *p, int cheat);

/* DWARF names parameter p at original line 18. This body preserves the
 * original reset set; x, y, and angle deliberately remain untouched. */
void reset_player(Tplayer *p)
{
    int i;

    p->sx = 0.0;
    p->sy = 0.0;
    p->status = 0;
    p->jump_key = 1;
    p->frame = 0;
    p->level = 0;
    p->in_combo = 0;
    p->acc_level = 0;
    p->acc_jumps = 0;
    p->score = 0;
    p->dead = 0;
    p->max_s = 0.0;
    p->rotate = 0;
    p->edge = 0;
    p->edge_drawn = 0;
    p->bounce = 0;
    p->shake = 0;
    p->latest_combo = 0;
    p->show_combo = 0;
    p->best_combo = 0;
    p->no_combo_top_floor = 0;
    p->biggest_lost_combo = 0;
    for (i = 4; i >= 0; i--)
        p->ccc[i] = 0;
    for (i = 4; i >= 0; i--)
        p->jc[i] = 0;
    for (i = 4; i >= 0; i--)
        p->jcTop[i] = 0;
}

/* Partial recovery of player.c, 0x418740..0x4189cb.  The core integration
 * and state transition order are oracle-backed; mode-table tuning is pending. */
void update_player(Tplayer *p)
{
    /* port: the floating-point steps below are evaluated exactly as the
     * original i386 build did them (x87, 64-bit precision; port/sim/x87.h).
     * Sums and products are rounded to double only when stored, and the
     * comparisons use the unrounded 80-bit register values. */
    x87 X, Y;

    p->sy = MID(-100, p->sy, max_speed[collision_type]);
    p->sx = MID(-max_speed[collision_type], p->sx, max_speed[collision_type]);
    X = x87_add(x87_from_f64(p->sx), x87_from_f64(p->x));
    p->x = x87_to_f64(X);
    Y = x87_add(x87_from_f64(p->sy), x87_from_f64(p->y));
    p->y = x87_to_f64(Y);
    if (x87_gt(Y, x87_from_i32(1000)))
        p->y = 1000;
    if (x87_gt(X, x87_from_i32(555))) {
        x87 SX;
        p->x = 555;
        SX = x87_mul(x87_from_f64(-0.9), x87_from_f64(p->sx));
        p->sx = x87_to_f64(SX);
        if (x87_gt(x87_abs(SX), x87_from_i32(4)))
            p->bounce = -20;
    }
    else if (x87_lt(X, x87_from_i32(85))) {
        x87 SX;
        p->x = 85;
        SX = x87_mul(x87_from_f64(-0.9), x87_from_f64(p->sx));
        p->sx = x87_to_f64(SX);
        if (x87_gt(x87_abs(SX), x87_from_i32(4)))
            p->bounce = 20;
    }
    if (p->status) {
        x87 SY = x87_add(x87_add(x87_from_f64(0.8),
                                 x87_from_f64(gravity_modifier[get_demo()->gravity])),
                         x87_from_f64(p->sy));
        p->sy = x87_to_f64(SY);
        if (p->status == 1 && x87_gt(SY, x87_from_i32(0)))
            p->status = 2;
    }
}

int jump_player(Tplayer *p, int cheat)
{
    if (cheat) {
        p->status = 1;
        p->sy = (double)(-(cheat * 12));
        return -1;
    }
    if (p->status)
        return 0;

    p->status = 1;
    p->sy = MIN(-ABS(p->sx * 2), -max_speed[collision_type]);
    p->max_s = p->sx;
    if (p->sy < -22)
        p->rotate = 1;
    p->angle = 0;
    return -1;
}

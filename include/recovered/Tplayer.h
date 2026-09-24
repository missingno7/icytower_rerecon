/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TPLAYER_H
#define RECOVERED_TPLAYER_H
#include <stddef.h>
#include <allegro.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    double x;
    double y;
    double sx;
    double sy;
    double max_s;
    int level;
    int score;
    int best_combo;
    int status;
    int jump_key;
    int frame;
    int in_combo;
    int acc_level;
    int acc_jumps;
    int dead;
    int rotate;
    fixed angle;
    int edge;
    int edge_drawn;
    int bounce;
    int shake;
    int latest_combo;
    int show_combo;
    int no_combo_top_floor;
    int biggest_lost_combo;
    int ccc[5];
    int jcTop[5];
    int jc[5];
} Tplayer;
RECOVERED_STATIC_ASSERT(sizeof(Tplayer) == 184, Tplayer_size);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, x) == 0, Tplayer_offset_x);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, y) == 8, Tplayer_offset_y);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, sx) == 16, Tplayer_offset_sx);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, sy) == 24, Tplayer_offset_sy);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, max_s) == 32, Tplayer_offset_max_s);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, level) == 40, Tplayer_offset_level);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, score) == 44, Tplayer_offset_score);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, best_combo) == 48, Tplayer_offset_best_combo);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, status) == 52, Tplayer_offset_status);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, jump_key) == 56, Tplayer_offset_jump_key);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, frame) == 60, Tplayer_offset_frame);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, in_combo) == 64, Tplayer_offset_in_combo);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, acc_level) == 68, Tplayer_offset_acc_level);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, acc_jumps) == 72, Tplayer_offset_acc_jumps);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, dead) == 76, Tplayer_offset_dead);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, rotate) == 80, Tplayer_offset_rotate);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, angle) == 84, Tplayer_offset_angle);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, edge) == 88, Tplayer_offset_edge);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, edge_drawn) == 92, Tplayer_offset_edge_drawn);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, bounce) == 96, Tplayer_offset_bounce);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, shake) == 100, Tplayer_offset_shake);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, latest_combo) == 104, Tplayer_offset_latest_combo);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, show_combo) == 108, Tplayer_offset_show_combo);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, no_combo_top_floor) == 112, Tplayer_offset_no_combo_top_floor);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, biggest_lost_combo) == 116, Tplayer_offset_biggest_lost_combo);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, ccc) == 120, Tplayer_offset_ccc);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, jcTop) == 140, Tplayer_offset_jcTop);
RECOVERED_STATIC_ASSERT(offsetof(Tplayer, jc) == 160, Tplayer_offset_jc);
#endif

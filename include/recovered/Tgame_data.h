/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TGAME_DATA_H
#define RECOVERED_TGAME_DATA_H
#include <stddef.h>
#include "Tgd_combo.h"
#include "Tgd_jump_sequence.h"
#include "Treplay.h"
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    Treplay *replay;
    int score;
    int floor;
    int combo;
    int no_combo_top_floor;
    int biggest_lost_combo;
    int ccc[5];
    int jc[5];
    int comboPosts;
    Tgd_combo combos[5000];
    int jumpPosts;
    Tgd_jump_sequence jumps[5000];
    int left;
    int right;
    int jump;
} Tgame_data;
RECOVERED_STATIC_ASSERT(sizeof(Tgame_data) == 120084, Tgame_data_size);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, replay) == 0, Tgame_data_offset_replay);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, score) == 4, Tgame_data_offset_score);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, floor) == 8, Tgame_data_offset_floor);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, combo) == 12, Tgame_data_offset_combo);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, no_combo_top_floor) == 16, Tgame_data_offset_no_combo_top_floor);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, biggest_lost_combo) == 20, Tgame_data_offset_biggest_lost_combo);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, ccc) == 24, Tgame_data_offset_ccc);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, jc) == 44, Tgame_data_offset_jc);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, comboPosts) == 64, Tgame_data_offset_comboPosts);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, combos) == 68, Tgame_data_offset_combos);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, jumpPosts) == 60068, Tgame_data_offset_jumpPosts);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, jumps) == 60072, Tgame_data_offset_jumps);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, left) == 120072, Tgame_data_offset_left);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, right) == 120076, Tgame_data_offset_right);
RECOVERED_STATIC_ASSERT(offsetof(Tgame_data, jump) == 120080, Tgame_data_offset_jump);
#endif

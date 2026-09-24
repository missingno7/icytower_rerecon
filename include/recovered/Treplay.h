/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TREPLAY_H
#define RECOVERED_TREPLAY_H
#include <stddef.h>
#include "Trecord.h"
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    char header[6];
    int size;
    char name[32];
    char date[32];
    int checksum;
    int score;
    int floor;
    int combo;
    int no_combo_top_floor;
    int biggest_lost_combo;
    int ccc[5];
    int jc[5];
    int floor_shrink;
    int floor_size;
    int start_speed;
    int speed_increase;
    int gravity;
    int rejump;
    int random_seed;
    char comment[42];
    int tc_posts;
    float tc_c_data[100];
    float tc_q_data[100];
    float tc_t_data[100];
    float tc_s_data[100];
    float tc_f_data[100];
    Trecord *data;
} Treplay;
RECOVERED_STATIC_ASSERT(sizeof(Treplay) == 2220, Treplay_size);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, header) == 0, Treplay_offset_header);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, size) == 8, Treplay_offset_size);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, name) == 12, Treplay_offset_name);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, date) == 44, Treplay_offset_date);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, checksum) == 76, Treplay_offset_checksum);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, score) == 80, Treplay_offset_score);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, floor) == 84, Treplay_offset_floor);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, combo) == 88, Treplay_offset_combo);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, no_combo_top_floor) == 92, Treplay_offset_no_combo_top_floor);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, biggest_lost_combo) == 96, Treplay_offset_biggest_lost_combo);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, ccc) == 100, Treplay_offset_ccc);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, jc) == 120, Treplay_offset_jc);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, floor_shrink) == 140, Treplay_offset_floor_shrink);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, floor_size) == 144, Treplay_offset_floor_size);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, start_speed) == 148, Treplay_offset_start_speed);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, speed_increase) == 152, Treplay_offset_speed_increase);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, gravity) == 156, Treplay_offset_gravity);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, rejump) == 160, Treplay_offset_rejump);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, random_seed) == 164, Treplay_offset_random_seed);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, comment) == 168, Treplay_offset_comment);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, tc_posts) == 212, Treplay_offset_tc_posts);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, tc_c_data) == 216, Treplay_offset_tc_c_data);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, tc_q_data) == 616, Treplay_offset_tc_q_data);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, tc_t_data) == 1016, Treplay_offset_tc_t_data);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, tc_s_data) == 1416, Treplay_offset_tc_s_data);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, tc_f_data) == 1816, Treplay_offset_tc_f_data);
RECOVERED_STATIC_ASSERT(offsetof(Treplay, data) == 2216, Treplay_offset_data);
#endif

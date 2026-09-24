/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TPROFILE_H
#define RECOVERED_TPROFILE_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    char header[6];
    char handle[32];
    int checksum;
    int games_played;
    int custom_games_played;
    int games_quit;
    int seconds_spent_playing;
    int total_floors;
    int total_score;
    int total_combos;
    int total_combo_floors;
    int best_floor;
    int best_combo;
    int best_score;
    int no_combo_top_floor;
    int biggest_lost_combo;
    int cccNum[5];
    int cccTotal[5];
    int ccc[5];
    int jc[5];
    int rewards[10];
    int total_jumps;
    char best_replay_names[32][32];
    int flash;
    int jump_hold;
    char last_avatar[64];
    int start_floor;
    int msc_volume;
    int snd_volume;
    char creationDate[16];
    char saveDate[16];
} Tprofile;
RECOVERED_STATIC_ASSERT(sizeof(Tprofile) == 1360, Tprofile_size);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, header) == 0, Tprofile_offset_header);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, handle) == 6, Tprofile_offset_handle);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, checksum) == 40, Tprofile_offset_checksum);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, games_played) == 44, Tprofile_offset_games_played);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, custom_games_played) == 48, Tprofile_offset_custom_games_played);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, games_quit) == 52, Tprofile_offset_games_quit);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, seconds_spent_playing) == 56, Tprofile_offset_seconds_spent_playing);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, total_floors) == 60, Tprofile_offset_total_floors);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, total_score) == 64, Tprofile_offset_total_score);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, total_combos) == 68, Tprofile_offset_total_combos);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, total_combo_floors) == 72, Tprofile_offset_total_combo_floors);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, best_floor) == 76, Tprofile_offset_best_floor);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, best_combo) == 80, Tprofile_offset_best_combo);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, best_score) == 84, Tprofile_offset_best_score);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, no_combo_top_floor) == 88, Tprofile_offset_no_combo_top_floor);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, biggest_lost_combo) == 92, Tprofile_offset_biggest_lost_combo);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, cccNum) == 96, Tprofile_offset_cccNum);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, cccTotal) == 116, Tprofile_offset_cccTotal);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, ccc) == 136, Tprofile_offset_ccc);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, jc) == 156, Tprofile_offset_jc);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, rewards) == 176, Tprofile_offset_rewards);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, total_jumps) == 216, Tprofile_offset_total_jumps);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, best_replay_names) == 220, Tprofile_offset_best_replay_names);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, flash) == 1244, Tprofile_offset_flash);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, jump_hold) == 1248, Tprofile_offset_jump_hold);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, last_avatar) == 1252, Tprofile_offset_last_avatar);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, start_floor) == 1316, Tprofile_offset_start_floor);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, msc_volume) == 1320, Tprofile_offset_msc_volume);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, snd_volume) == 1324, Tprofile_offset_snd_volume);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, creationDate) == 1328, Tprofile_offset_creationDate);
RECOVERED_STATIC_ASSERT(offsetof(Tprofile, saveDate) == 1344, Tprofile_offset_saveDate);
#endif

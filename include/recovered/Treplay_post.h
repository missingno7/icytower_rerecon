/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TREPLAY_POST_H
#define RECOVERED_TREPLAY_POST_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    char *full_path;
    char directory;
    char parent;
    int version;
    int score;
    int floor;
    int combo;
} Treplay_post;
RECOVERED_STATIC_ASSERT(sizeof(Treplay_post) == 24, Treplay_post_size);
RECOVERED_STATIC_ASSERT(offsetof(Treplay_post, full_path) == 0, Treplay_post_offset_full_path);
RECOVERED_STATIC_ASSERT(offsetof(Treplay_post, directory) == 4, Treplay_post_offset_directory);
RECOVERED_STATIC_ASSERT(offsetof(Treplay_post, parent) == 5, Treplay_post_offset_parent);
RECOVERED_STATIC_ASSERT(offsetof(Treplay_post, version) == 8, Treplay_post_offset_version);
RECOVERED_STATIC_ASSERT(offsetof(Treplay_post, score) == 12, Treplay_post_offset_score);
RECOVERED_STATIC_ASSERT(offsetof(Treplay_post, floor) == 16, Treplay_post_offset_floor);
RECOVERED_STATIC_ASSERT(offsetof(Treplay_post, combo) == 20, Treplay_post_offset_combo);
#endif

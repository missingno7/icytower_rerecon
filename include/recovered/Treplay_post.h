/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TREPLAY_POST_H
#define RECOVERED_TREPLAY_POST_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
#ifndef RECOVERED_ILP32_ASSERT
/* portable build: pointer-bearing layouts are only fixed on 32-bit targets */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ != 4
#define RECOVERED_ILP32_ASSERT(expr, name) typedef char recovered_ilp32_unchecked_##name
#else
#define RECOVERED_ILP32_ASSERT(expr, name) RECOVERED_STATIC_ASSERT(expr, name)
#endif
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
RECOVERED_ILP32_ASSERT(sizeof(Treplay_post) == 24, Treplay_post_size);
RECOVERED_ILP32_ASSERT(offsetof(Treplay_post, full_path) == 0, Treplay_post_offset_full_path);
RECOVERED_ILP32_ASSERT(offsetof(Treplay_post, directory) == 4, Treplay_post_offset_directory);
RECOVERED_ILP32_ASSERT(offsetof(Treplay_post, parent) == 5, Treplay_post_offset_parent);
RECOVERED_ILP32_ASSERT(offsetof(Treplay_post, version) == 8, Treplay_post_offset_version);
RECOVERED_ILP32_ASSERT(offsetof(Treplay_post, score) == 12, Treplay_post_offset_score);
RECOVERED_ILP32_ASSERT(offsetof(Treplay_post, floor) == 16, Treplay_post_offset_floor);
RECOVERED_ILP32_ASSERT(offsetof(Treplay_post, combo) == 20, Treplay_post_offset_combo);
#endif

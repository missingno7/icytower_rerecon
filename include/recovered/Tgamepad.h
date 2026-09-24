/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TGAMEPAD_H
#define RECOVERED_TGAMEPAD_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    int up;
    int down;
    int left;
    int right;
    int b[32];
} Tgamepad;
RECOVERED_STATIC_ASSERT(sizeof(Tgamepad) == 144, Tgamepad_size);
RECOVERED_STATIC_ASSERT(offsetof(Tgamepad, up) == 0, Tgamepad_offset_up);
RECOVERED_STATIC_ASSERT(offsetof(Tgamepad, down) == 4, Tgamepad_offset_down);
RECOVERED_STATIC_ASSERT(offsetof(Tgamepad, left) == 8, Tgamepad_offset_left);
RECOVERED_STATIC_ASSERT(offsetof(Tgamepad, right) == 12, Tgamepad_offset_right);
RECOVERED_STATIC_ASSERT(offsetof(Tgamepad, b) == 16, Tgamepad_offset_b);
#endif

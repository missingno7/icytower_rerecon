/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TGD_JUMP_SEQUENCE_H
#define RECOVERED_TGD_JUMP_SEQUENCE_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    int start;
    int dist;
    int num;
} Tgd_jump_sequence;
RECOVERED_STATIC_ASSERT(sizeof(Tgd_jump_sequence) == 12, Tgd_jump_sequence_size);
RECOVERED_STATIC_ASSERT(offsetof(Tgd_jump_sequence, start) == 0, Tgd_jump_sequence_offset_start);
RECOVERED_STATIC_ASSERT(offsetof(Tgd_jump_sequence, dist) == 4, Tgd_jump_sequence_offset_dist);
RECOVERED_STATIC_ASSERT(offsetof(Tgd_jump_sequence, num) == 8, Tgd_jump_sequence_offset_num);
#endif

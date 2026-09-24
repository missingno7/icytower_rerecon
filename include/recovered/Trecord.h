/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TRECORD_H
#define RECOVERED_TRECORD_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    unsigned char key_flags;
    int cycle_count;
} Trecord;
RECOVERED_STATIC_ASSERT(sizeof(Trecord) == 8, Trecord_size);
RECOVERED_STATIC_ASSERT(offsetof(Trecord, key_flags) == 0, Trecord_offset_key_flags);
RECOVERED_STATIC_ASSERT(offsetof(Trecord, cycle_count) == 4, Trecord_offset_cycle_count);
#endif

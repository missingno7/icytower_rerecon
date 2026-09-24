/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TGD_COMBO_H
#define RECOVERED_TGD_COMBO_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    int start;
    int end;
    int length;
} Tgd_combo;
RECOVERED_STATIC_ASSERT(sizeof(Tgd_combo) == 12, Tgd_combo_size);
RECOVERED_STATIC_ASSERT(offsetof(Tgd_combo, start) == 0, Tgd_combo_offset_start);
RECOVERED_STATIC_ASSERT(offsetof(Tgd_combo, end) == 4, Tgd_combo_offset_end);
RECOVERED_STATIC_ASSERT(offsetof(Tgd_combo, length) == 8, Tgd_combo_offset_length);
#endif

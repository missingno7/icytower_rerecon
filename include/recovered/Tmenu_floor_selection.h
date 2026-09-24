/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TMENU_FLOOR_SELECTION_H
#define RECOVERED_TMENU_FLOOR_SELECTION_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    int value;
    int max;
} Tmenu_floor_selection;
RECOVERED_STATIC_ASSERT(sizeof(Tmenu_floor_selection) == 8, Tmenu_floor_selection_size);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu_floor_selection, value) == 0, Tmenu_floor_selection_offset_value);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu_floor_selection, max) == 4, Tmenu_floor_selection_offset_max);
#endif

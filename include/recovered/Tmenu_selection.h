/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TMENU_SELECTION_H
#define RECOVERED_TMENU_SELECTION_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    int value;
    int size;
    char *caption[32];
} Tmenu_selection;
RECOVERED_STATIC_ASSERT(sizeof(Tmenu_selection) == 136, Tmenu_selection_size);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu_selection, value) == 0, Tmenu_selection_offset_value);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu_selection, size) == 4, Tmenu_selection_offset_size);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu_selection, caption) == 8, Tmenu_selection_offset_caption);
#endif

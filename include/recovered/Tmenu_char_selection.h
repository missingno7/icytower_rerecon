/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TMENU_CHAR_SELECTION_H
#define RECOVERED_TMENU_CHAR_SELECTION_H
#include <stddef.h>
#include <allegro.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    int value;
    int max;
    BITMAP *bmp;
    PALETTE pal;
} Tmenu_char_selection;
RECOVERED_STATIC_ASSERT(sizeof(Tmenu_char_selection) == 1036, Tmenu_char_selection_size);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu_char_selection, value) == 0, Tmenu_char_selection_offset_value);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu_char_selection, max) == 4, Tmenu_char_selection_offset_max);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu_char_selection, bmp) == 8, Tmenu_char_selection_offset_bmp);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu_char_selection, pal) == 12, Tmenu_char_selection_offset_pal);
#endif

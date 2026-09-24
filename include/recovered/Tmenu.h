/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TMENU_H
#define RECOVERED_TMENU_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    char caption[128];
    int return_select;
    int return_left;
    int return_right;
    int flags;
    void *data;
} Tmenu;
RECOVERED_STATIC_ASSERT(sizeof(Tmenu) == 148, Tmenu_size);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu, caption) == 0, Tmenu_offset_caption);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu, return_select) == 128, Tmenu_offset_return_select);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu, return_left) == 132, Tmenu_offset_return_left);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu, return_right) == 136, Tmenu_offset_return_right);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu, flags) == 140, Tmenu_offset_flags);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu, data) == 144, Tmenu_offset_data);
#endif

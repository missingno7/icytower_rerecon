/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TMENU_H
#define RECOVERED_TMENU_H
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
    char caption[128];
    int return_select;
    int return_left;
    int return_right;
    int flags;
    void *data;
} Tmenu;
RECOVERED_ILP32_ASSERT(sizeof(Tmenu) == 148, Tmenu_size);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu, caption) == 0, Tmenu_offset_caption);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu, return_select) == 128, Tmenu_offset_return_select);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu, return_left) == 132, Tmenu_offset_return_left);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu, return_right) == 136, Tmenu_offset_return_right);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu, flags) == 140, Tmenu_offset_flags);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu, data) == 144, Tmenu_offset_data);
#endif

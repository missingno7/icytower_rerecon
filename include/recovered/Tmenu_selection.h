/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TMENU_SELECTION_H
#define RECOVERED_TMENU_SELECTION_H
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
    int value;
    int size;
    char *caption[32];
} Tmenu_selection;
RECOVERED_ILP32_ASSERT(sizeof(Tmenu_selection) == 136, Tmenu_selection_size);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu_selection, value) == 0, Tmenu_selection_offset_value);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu_selection, size) == 4, Tmenu_selection_offset_size);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu_selection, caption) == 8, Tmenu_selection_offset_caption);
#endif

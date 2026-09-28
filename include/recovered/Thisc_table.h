/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_THISC_TABLE_H
#define RECOVERED_THISC_TABLE_H
#include <stddef.h>
#include "Thisc.h"
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
    char name[32];
    Thisc *posts;
} Thisc_table;
RECOVERED_ILP32_ASSERT(sizeof(Thisc_table) == 36, Thisc_table_size);
RECOVERED_ILP32_ASSERT(offsetof(Thisc_table, name) == 0, Thisc_table_offset_name);
RECOVERED_ILP32_ASSERT(offsetof(Thisc_table, posts) == 32, Thisc_table_offset_posts);
#endif

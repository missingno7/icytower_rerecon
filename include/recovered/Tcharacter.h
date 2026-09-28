/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TCHARACTER_H
#define RECOVERED_TCHARACTER_H
#include <stddef.h>
#include <allegro.h>
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
    char filename[1024];
    BITMAP *bmp;
    int ok;
    char name[128];
    int uses_datafile;
    PALETTE pal;
} Tcharacter;
RECOVERED_ILP32_ASSERT(sizeof(Tcharacter) == 2188, Tcharacter_size);
RECOVERED_ILP32_ASSERT(offsetof(Tcharacter, filename) == 0, Tcharacter_offset_filename);
RECOVERED_ILP32_ASSERT(offsetof(Tcharacter, bmp) == 1024, Tcharacter_offset_bmp);
RECOVERED_ILP32_ASSERT(offsetof(Tcharacter, ok) == 1028, Tcharacter_offset_ok);
RECOVERED_ILP32_ASSERT(offsetof(Tcharacter, name) == 1032, Tcharacter_offset_name);
RECOVERED_ILP32_ASSERT(offsetof(Tcharacter, uses_datafile) == 1160, Tcharacter_offset_uses_datafile);
RECOVERED_ILP32_ASSERT(offsetof(Tcharacter, pal) == 1164, Tcharacter_offset_pal);
#endif

/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TCHARACTER_H
#define RECOVERED_TCHARACTER_H
#include <stddef.h>
#include <allegro.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    char filename[1024];
    BITMAP *bmp;
    int ok;
    char name[128];
    int uses_datafile;
    PALETTE pal;
} Tcharacter;
RECOVERED_STATIC_ASSERT(sizeof(Tcharacter) == 2188, Tcharacter_size);
RECOVERED_STATIC_ASSERT(offsetof(Tcharacter, filename) == 0, Tcharacter_offset_filename);
RECOVERED_STATIC_ASSERT(offsetof(Tcharacter, bmp) == 1024, Tcharacter_offset_bmp);
RECOVERED_STATIC_ASSERT(offsetof(Tcharacter, ok) == 1028, Tcharacter_offset_ok);
RECOVERED_STATIC_ASSERT(offsetof(Tcharacter, name) == 1032, Tcharacter_offset_name);
RECOVERED_STATIC_ASSERT(offsetof(Tcharacter, uses_datafile) == 1160, Tcharacter_offset_uses_datafile);
RECOVERED_STATIC_ASSERT(offsetof(Tcharacter, pal) == 1164, Tcharacter_offset_pal);
#endif

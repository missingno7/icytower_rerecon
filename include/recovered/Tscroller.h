/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TSCROLLER_H
#define RECOVERED_TSCROLLER_H
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
    int horizontal;
    char *text;
    FONT *fnt;
    int font_height;
    int width;
    int height;
    int offset;
    int rows;
    int length;
    char *lines[512];
} Tscroller;
RECOVERED_ILP32_ASSERT(sizeof(Tscroller) == 2084, Tscroller_size);
RECOVERED_ILP32_ASSERT(offsetof(Tscroller, horizontal) == 0, Tscroller_offset_horizontal);
RECOVERED_ILP32_ASSERT(offsetof(Tscroller, text) == 4, Tscroller_offset_text);
RECOVERED_ILP32_ASSERT(offsetof(Tscroller, fnt) == 8, Tscroller_offset_fnt);
RECOVERED_ILP32_ASSERT(offsetof(Tscroller, font_height) == 12, Tscroller_offset_font_height);
RECOVERED_ILP32_ASSERT(offsetof(Tscroller, width) == 16, Tscroller_offset_width);
RECOVERED_ILP32_ASSERT(offsetof(Tscroller, height) == 20, Tscroller_offset_height);
RECOVERED_ILP32_ASSERT(offsetof(Tscroller, offset) == 24, Tscroller_offset_offset);
RECOVERED_ILP32_ASSERT(offsetof(Tscroller, rows) == 28, Tscroller_offset_rows);
RECOVERED_ILP32_ASSERT(offsetof(Tscroller, length) == 32, Tscroller_offset_length);
RECOVERED_ILP32_ASSERT(offsetof(Tscroller, lines) == 36, Tscroller_offset_lines);
#endif

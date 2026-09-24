/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TSTAR_FIELD_H
#define RECOVERED_TSTAR_FIELD_H
#include <stddef.h>
#include "Tstar.h"
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    int clear_color;
    int stars;
    int width;
    int height;
    int depth;
    int col1;
    int col_step;
    Tstar star[1024];
} Tstar_field;
RECOVERED_STATIC_ASSERT(sizeof(Tstar_field) == 24608, Tstar_field_size);
RECOVERED_STATIC_ASSERT(offsetof(Tstar_field, clear_color) == 0, Tstar_field_offset_clear_color);
RECOVERED_STATIC_ASSERT(offsetof(Tstar_field, stars) == 4, Tstar_field_offset_stars);
RECOVERED_STATIC_ASSERT(offsetof(Tstar_field, width) == 8, Tstar_field_offset_width);
RECOVERED_STATIC_ASSERT(offsetof(Tstar_field, height) == 12, Tstar_field_offset_height);
RECOVERED_STATIC_ASSERT(offsetof(Tstar_field, depth) == 16, Tstar_field_offset_depth);
RECOVERED_STATIC_ASSERT(offsetof(Tstar_field, col1) == 20, Tstar_field_offset_col1);
RECOVERED_STATIC_ASSERT(offsetof(Tstar_field, col_step) == 24, Tstar_field_offset_col_step);
RECOVERED_STATIC_ASSERT(offsetof(Tstar_field, star) == 32, Tstar_field_offset_star);
#endif

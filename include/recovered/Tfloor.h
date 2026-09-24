/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TFLOOR_H
#define RECOVERED_TFLOOR_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    int empty;
    int start_tile;
    int end_tile;
    int level;
    int sign;
    int tiles;
} Tfloor;
RECOVERED_STATIC_ASSERT(sizeof(Tfloor) == 24, Tfloor_size);
RECOVERED_STATIC_ASSERT(offsetof(Tfloor, empty) == 0, Tfloor_offset_empty);
RECOVERED_STATIC_ASSERT(offsetof(Tfloor, start_tile) == 4, Tfloor_offset_start_tile);
RECOVERED_STATIC_ASSERT(offsetof(Tfloor, end_tile) == 8, Tfloor_offset_end_tile);
RECOVERED_STATIC_ASSERT(offsetof(Tfloor, level) == 12, Tfloor_offset_level);
RECOVERED_STATIC_ASSERT(offsetof(Tfloor, sign) == 16, Tfloor_offset_sign);
RECOVERED_STATIC_ASSERT(offsetof(Tfloor, tiles) == 20, Tfloor_offset_tiles);
#endif

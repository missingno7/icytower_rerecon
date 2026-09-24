/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TMAP_H
#define RECOVERED_TMAP_H
#include <stddef.h>
#include "Tfloor.h"
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    Tfloor room[32];
    int offset;
} Tmap;
RECOVERED_STATIC_ASSERT(sizeof(Tmap) == 772, Tmap_size);
RECOVERED_STATIC_ASSERT(offsetof(Tmap, room) == 0, Tmap_offset_room);
RECOVERED_STATIC_ASSERT(offsetof(Tmap, offset) == 768, Tmap_offset_offset);
#endif

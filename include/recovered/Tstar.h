/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TSTAR_H
#define RECOVERED_TSTAR_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    double x;
    double y;
    int z;
} Tstar;
RECOVERED_STATIC_ASSERT(sizeof(Tstar) == 24, Tstar_size);
RECOVERED_STATIC_ASSERT(offsetof(Tstar, x) == 0, Tstar_offset_x);
RECOVERED_STATIC_ASSERT(offsetof(Tstar, y) == 8, Tstar_offset_y);
RECOVERED_STATIC_ASSERT(offsetof(Tstar, z) == 16, Tstar_offset_z);
#endif

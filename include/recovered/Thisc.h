/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_THISC_H
#define RECOVERED_THISC_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    char name[32];
    unsigned int value;
} Thisc;
RECOVERED_STATIC_ASSERT(sizeof(Thisc) == 36, Thisc_size);
RECOVERED_STATIC_ASSERT(offsetof(Thisc, name) == 0, Thisc_offset_name);
RECOVERED_STATIC_ASSERT(offsetof(Thisc, value) == 32, Thisc_offset_value);
#endif

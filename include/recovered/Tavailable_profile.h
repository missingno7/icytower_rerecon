/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TAVAILABLE_PROFILE_H
#define RECOVERED_TAVAILABLE_PROFILE_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    char handle[32];
} Tavailable_profile;
RECOVERED_STATIC_ASSERT(sizeof(Tavailable_profile) == 32, Tavailable_profile_size);
RECOVERED_STATIC_ASSERT(offsetof(Tavailable_profile, handle) == 0, Tavailable_profile_offset_handle);
#endif

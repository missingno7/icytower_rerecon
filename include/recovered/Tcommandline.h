/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TCOMMANDLINE_H
#define RECOVERED_TCOMMANDLINE_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    int jumps;
    int combos;
    int sd;
    int keys;
    int tiny;
} Tcommandline;
RECOVERED_STATIC_ASSERT(sizeof(Tcommandline) == 20, Tcommandline_size);
RECOVERED_STATIC_ASSERT(offsetof(Tcommandline, jumps) == 0, Tcommandline_offset_jumps);
RECOVERED_STATIC_ASSERT(offsetof(Tcommandline, combos) == 4, Tcommandline_offset_combos);
RECOVERED_STATIC_ASSERT(offsetof(Tcommandline, sd) == 8, Tcommandline_offset_sd);
RECOVERED_STATIC_ASSERT(offsetof(Tcommandline, keys) == 12, Tcommandline_offset_keys);
RECOVERED_STATIC_ASSERT(offsetof(Tcommandline, tiny) == 16, Tcommandline_offset_tiny);
#endif

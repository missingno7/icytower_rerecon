/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TMENU_SLIDER_H
#define RECOVERED_TMENU_SLIDER_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    int value;
    int min;
    int max;
    int step;
} Tmenu_slider;
RECOVERED_STATIC_ASSERT(sizeof(Tmenu_slider) == 16, Tmenu_slider_size);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu_slider, value) == 0, Tmenu_slider_offset_value);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu_slider, min) == 4, Tmenu_slider_offset_min);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu_slider, max) == 8, Tmenu_slider_offset_max);
RECOVERED_STATIC_ASSERT(offsetof(Tmenu_slider, step) == 12, Tmenu_slider_offset_step);
#endif

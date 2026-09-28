/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TMENU_PARAMS_H
#define RECOVERED_TMENU_PARAMS_H
#include <stddef.h>
#include <allegro.h>
#include "Tcontrol.h"
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
    FONT *font;
    int font_height;
    Tcontrol ctrl;
    BITMAP *bullet;
    int pos;
    DATAFILE *data;
    int fo;
} Tmenu_params;
RECOVERED_ILP32_ASSERT(sizeof(Tmenu_params) == 60, Tmenu_params_size);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu_params, font) == 0, Tmenu_params_offset_font);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu_params, font_height) == 4, Tmenu_params_offset_font_height);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu_params, ctrl) == 8, Tmenu_params_offset_ctrl);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu_params, bullet) == 44, Tmenu_params_offset_bullet);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu_params, pos) == 48, Tmenu_params_offset_pos);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu_params, data) == 52, Tmenu_params_offset_data);
RECOVERED_ILP32_ASSERT(offsetof(Tmenu_params, fo) == 56, Tmenu_params_offset_fo);
#endif

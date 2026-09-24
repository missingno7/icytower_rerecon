/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TCONTROL_H
#define RECOVERED_TCONTROL_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    int use_joy;
    int key_left;
    int key_right;
    int key_up;
    int key_down;
    int key_fire;
    int key_enter;
    int key_pause;
    unsigned char flags;
} Tcontrol;
RECOVERED_STATIC_ASSERT(sizeof(Tcontrol) == 36, Tcontrol_size);
RECOVERED_STATIC_ASSERT(offsetof(Tcontrol, use_joy) == 0, Tcontrol_offset_use_joy);
RECOVERED_STATIC_ASSERT(offsetof(Tcontrol, key_left) == 4, Tcontrol_offset_key_left);
RECOVERED_STATIC_ASSERT(offsetof(Tcontrol, key_right) == 8, Tcontrol_offset_key_right);
RECOVERED_STATIC_ASSERT(offsetof(Tcontrol, key_up) == 12, Tcontrol_offset_key_up);
RECOVERED_STATIC_ASSERT(offsetof(Tcontrol, key_down) == 16, Tcontrol_offset_key_down);
RECOVERED_STATIC_ASSERT(offsetof(Tcontrol, key_fire) == 20, Tcontrol_offset_key_fire);
RECOVERED_STATIC_ASSERT(offsetof(Tcontrol, key_enter) == 24, Tcontrol_offset_key_enter);
RECOVERED_STATIC_ASSERT(offsetof(Tcontrol, key_pause) == 28, Tcontrol_offset_key_pause);
RECOVERED_STATIC_ASSERT(offsetof(Tcontrol, flags) == 32, Tcontrol_offset_flags);
#endif

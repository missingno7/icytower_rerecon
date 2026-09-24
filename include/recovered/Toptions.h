/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TOPTIONS_H
#define RECOVERED_TOPTIONS_H
#include <stddef.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    int flash;
    int checksum;
    int jump_hold;
    int full_screen;
    int floor_shrink;
    int floor_size;
    int start_speed;
    int speed_increase;
    int gravity;
    int msc_volume;
    int snd_volume;
    int sort_method;
    char updateDate[16];
    char posterDate[16];
    char posterUrl[256];
    char posterSrc[256];
    int posterSize;
    char lastProfile[32];
    int timesStarted;
} Toptions;
RECOVERED_STATIC_ASSERT(sizeof(Toptions) == 632, Toptions_size);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, flash) == 0, Toptions_offset_flash);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, checksum) == 4, Toptions_offset_checksum);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, jump_hold) == 8, Toptions_offset_jump_hold);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, full_screen) == 12, Toptions_offset_full_screen);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, floor_shrink) == 16, Toptions_offset_floor_shrink);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, floor_size) == 20, Toptions_offset_floor_size);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, start_speed) == 24, Toptions_offset_start_speed);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, speed_increase) == 28, Toptions_offset_speed_increase);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, gravity) == 32, Toptions_offset_gravity);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, msc_volume) == 36, Toptions_offset_msc_volume);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, snd_volume) == 40, Toptions_offset_snd_volume);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, sort_method) == 44, Toptions_offset_sort_method);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, updateDate) == 48, Toptions_offset_updateDate);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, posterDate) == 64, Toptions_offset_posterDate);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, posterUrl) == 80, Toptions_offset_posterUrl);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, posterSrc) == 336, Toptions_offset_posterSrc);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, posterSize) == 592, Toptions_offset_posterSize);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, lastProfile) == 596, Toptions_offset_lastProfile);
RECOVERED_STATIC_ASSERT(offsetof(Toptions, timesStarted) == 628, Toptions_offset_timesStarted);
#endif

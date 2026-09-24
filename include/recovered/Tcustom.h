/* Generated from locked DWARF; do not hand-edit. */
#ifndef RECOVERED_TCUSTOM_H
#define RECOVERED_TCUSTOM_H
#include <stddef.h>
#include <allegro.h>
#ifndef RECOVERED_STATIC_ASSERT
#define RECOVERED_STATIC_ASSERT(expr, name) typedef char recovered_static_assert_##name[(expr) ? 1 : -1]
#endif
typedef struct {
    char name[128];
    BITMAP *frame[15];
    PALETTE pal;
    SAMPLE *jump_sound[3];
    SAMPLE *falling;
    SAMPLE *edge;
    SAMPLE *yo;
    SAMPLE *wazup;
    SAMPLE *bg_music;
    MIDI *bg_midi;
    int uses_datafile;
    DATAFILE *df;
    int ok;
} Tcustom;
RECOVERED_STATIC_ASSERT(sizeof(Tcustom) == 1260, Tcustom_size);
RECOVERED_STATIC_ASSERT(offsetof(Tcustom, name) == 0, Tcustom_offset_name);
RECOVERED_STATIC_ASSERT(offsetof(Tcustom, frame) == 128, Tcustom_offset_frame);
RECOVERED_STATIC_ASSERT(offsetof(Tcustom, pal) == 188, Tcustom_offset_pal);
RECOVERED_STATIC_ASSERT(offsetof(Tcustom, jump_sound) == 1212, Tcustom_offset_jump_sound);
RECOVERED_STATIC_ASSERT(offsetof(Tcustom, falling) == 1224, Tcustom_offset_falling);
RECOVERED_STATIC_ASSERT(offsetof(Tcustom, edge) == 1228, Tcustom_offset_edge);
RECOVERED_STATIC_ASSERT(offsetof(Tcustom, yo) == 1232, Tcustom_offset_yo);
RECOVERED_STATIC_ASSERT(offsetof(Tcustom, wazup) == 1236, Tcustom_offset_wazup);
RECOVERED_STATIC_ASSERT(offsetof(Tcustom, bg_music) == 1240, Tcustom_offset_bg_music);
RECOVERED_STATIC_ASSERT(offsetof(Tcustom, bg_midi) == 1244, Tcustom_offset_bg_midi);
RECOVERED_STATIC_ASSERT(offsetof(Tcustom, uses_datafile) == 1248, Tcustom_offset_uses_datafile);
RECOVERED_STATIC_ASSERT(offsetof(Tcustom, df) == 1252, Tcustom_offset_df);
RECOVERED_STATIC_ASSERT(offsetof(Tcustom, ok) == 1256, Tcustom_offset_ok);
#endif

/* Historical CU: F:\projects\icytower\trunk\source\timer.c
 * All three bodies and five global definitions are represented.
 * Provenance: DWARF CU 0x400a1; evidence/research/src/icytower/timer.c;
 * install_timers disassembly 0x41fee4..0x41ff3c. See docs/timer-experiment.md.
 * Ownership/proof state is maintained in src/recovery.json, not inferred
 * merely from compiling this file.
 */
#include <allegro.h>
#include "timer.h"

volatile int frame_count;
volatile int fps;
volatile int logic_count;
volatile int lps;
volatile int cycle_count;

void fps_counter(void)
{
    fps = frame_count;
    frame_count = 0;
    lps = logic_count;
    logic_count = 0;
}

void cycle_counter(void)
{
    cycle_count++;
}

int install_timers(void)
{
    install_timer();
    install_int(fps_counter, 1000);
    fps = 0;
    frame_count = 0;
    cycle_count = 0;
    install_int(cycle_counter, 20);
    return -1;
}

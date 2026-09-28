/*
 * Timers: install_int() callbacks run on the main thread from a4_service().
 * OWNER: integration (supervisor).  Temporary minimal version.
 */
#include "a4_internal.h"
#include "port/platform/platform.h"

void a4_timer_service(void) { }
int install_timer(void) { return 0; }
int install_int(void (*proc)(void), long speed_ms) { (void)proc; (void)speed_ms; return 0; }
void remove_int(void (*proc)(void)) { (void)proc; }
void rest(unsigned int ms) { a4_service(); plat_sleep_ns((uint64_t)ms * 1000000u); }

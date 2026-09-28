/*
 * Fixed-step simulation scheduler.
 *
 * The historical game advanced one logic step per 20 ms timer tick
 * (install_int(cycle_counter, 20)) and busy-waited on `cycle_count` between
 * steps.  Here the main thread owns time:
 *
 *   SDL event loop -> monotonic clock -> 20 ms accumulator
 *                  -> 0..N simulation ticks (exactly 50 Hz)
 *                  -> presentation frames at display rate in between
 *
 * Every historical "wait for the next tick" site calls sched_wait_tick().
 * While it waits it pumps events and renders frames (with interpolation
 * alpha = accumulated time / 20 ms); it returns when one 20 ms step of real
 * time is available and consumes it.  After a stall the accumulator lets the
 * simulation catch up (bounded, see SCHED_MAX_CATCHUP_TICKS) instead of
 * silently slowing the game down; beyond the bound, time is dropped.
 *
 * Nothing here changes what a tick computes: the number and order of
 * simulation steps for a replay are independent of frame rate, vsync or
 * frame caps.  In headless mode time is virtual and waits return at once.
 */
#ifndef PORT_SCHED_H
#define PORT_SCHED_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SIMULATION_HZ          50
#define SIMULATION_DT_NS       20000000ull    /* 1 s / 50 */
#define SCHED_MAX_CATCHUP_TICKS 5             /* at most 100 ms of backlog is replayed */

void     sched_init(void);
/* Block until the next 20 ms simulation step is due, presenting frames
 * meanwhile.  Consumes one step. */
void     sched_wait_tick(void);
/* Forget accumulated time (after loading, modal dialogs, focus changes). */
void     sched_resync(void);
/* Interpolation factor in [0,1]: fraction of the next step already elapsed. */
float    sched_alpha(void);
/* Number of steps consumed so far. */
uint64_t sched_ticks(void);
/* Number of steps dropped by the catch-up bound (diagnostics). */
uint64_t sched_dropped_ticks(void);
/* Headless/virtual time (tests, replay checking, bots). */
void     sched_set_virtual(bool on);
/* debug: run the simulation `speed` times faster than real time */
void     sched_set_speed(double speed);
bool     sched_is_virtual(void);

#ifdef __cplusplus
}
#endif

#endif

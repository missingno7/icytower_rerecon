/*
 * Portability helpers used by the adapted historical game sources.
 *
 * The historical code ran on Win32 with MSVCRT.  Anything platform-specific
 * it relied on is expressed here in portable terms:
 *   - file paths: the game's relative paths are resolved against the user
 *     (writable) and asset (read-only) roots, see port/platform/platform.h;
 *   - rand()/srand(): MSVCRT's LCG, because map generation depends on it
 *     (replays store only the seed);
 *   - stricmp, mkdir(path), QueryPerformanceCounter.
 */
#ifndef PORT_GAME_H
#define PORT_GAME_H

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* stdio fopen with game-path resolution (write modes go to the user root) */
FILE *port_fopen(const char *path, const char *mode);
/* create a directory given as a game path (user root); returns 0 on success */
int port_mkdir(const char *path);
int port_stricmp(const char *a, const char *b);
#ifndef stricmp
#define stricmp port_stricmp
#endif

/* MSVCRT-compatible rand()/srand() (LCG 214013/2531011, 15-bit output).
 * Map generation (map.c add_floor) consumes this stream after
 * srand(replay seed), so it must be identical on every platform. */
int  hist_rand(void);
void hist_srand(unsigned int seed);

/* 32-bit views of the performance counter (historical LARGE_INTEGER.LowPart
 * use in play()'s anti-cheat timing) */
int port_qpc_low(void);
int port_qpf_low(void);

void port_open_url(const char *url);

/* Wait for the next 50 Hz simulation step (port/sim/sched.h).  Replaces the
 * historical `while (!cycle_count) rest(2);` busy waits; frames are
 * rendered at display rate while waiting. */
void port_wait_tick(void);

/* write tower.cfg and the current profile now (mobile apps are killed
 * without an exit path; see port_game.c) */
void port_save_state(void);

/* touch scheme for the in-game selection (port/input/touch.h values) */
int port_touch_scheme(void);
/* soft keyboard on/off while the game reads a typed string */
void port_text_input(int on, int canvas_y);

#ifdef __cplusplus
}
#endif

#endif

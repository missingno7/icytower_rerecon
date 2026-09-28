/*
 * Portable platform layer (SDL3).  Owns the OS window, the event pump, the
 * monotonic clock, file-system roots and the audio device.  Game code and the
 * Allegro compatibility layer only reach SDL through this interface (and the
 * renderer in port/render/).
 */
#ifndef PORT_PLATFORM_H
#define PORT_PLATFORM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- lifetime ------------------------------------------------------ */
bool plat_init(int argc, char **argv);
void plat_shutdown(void);
/* true when running without a window/audio (replay checking, tests) */
bool plat_headless(void);

/* ---- clock --------------------------------------------------------- */
uint64_t plat_ticks_ns(void);          /* monotonic */
void     plat_sleep_ns(uint64_t ns);   /* precise sleep */
/* Performance counter for the game's historical QueryPerformanceCounter use
 * (anti-cheat timing only; never gameplay). */
uint64_t plat_perf_counter(void);
uint64_t plat_perf_frequency(void);

/* ---- events -------------------------------------------------------- */
/* Process pending OS events: input devices, focus, resize, quit. */
void plat_pump_events(void);
bool plat_quit_requested(void);
void plat_clear_quit_request(void);
bool plat_has_focus(void);
/* focus callbacks (Allegro display-switch callbacks) */
void plat_set_focus_callbacks(void (*on_in)(void), void (*on_out)(void));

/* ---- messages ------------------------------------------------------ */
void plat_message_box(const char *title, const char *text);
/* returns index of chosen button (0 or 1) */
int  plat_choice_box(const char *title, const char *text, const char *b1, const char *b2);
bool plat_open_url(const char *url);

/* ---- file system roots ---------------------------------------------
 * Game code uses the historical relative paths ("data/data.dat",
 * "profiles/NAME/", "tower.cfg", "screenshots/...").  They are resolved
 * against two roots:
 *   user root   writable per-user directory (SDL_GetPrefPath), searched
 *               first for reading and always used for writing;
 *   asset roots read-only game data (ITOWER_DATA_DIR env/--data, the
 *               directory of the executable, then the current directory).
 * Absolute paths pass through unchanged. */
const char *plat_user_dir(void);
int  plat_asset_dir_count(void);
const char *plat_asset_dir(int i);
/* Resolve for reading: first existing candidate; if none exists the user
 * root candidate is returned.  Returns false on overflow. */
bool plat_resolve_read(const char *game_path, char *out, size_t outsz);
/* Resolve for writing (user root); creates missing parent directories. */
bool plat_resolve_write(const char *game_path, char *out, size_t outsz);
bool plat_is_absolute(const char *path);
bool plat_mkdir_p(const char *native_path);

/* ---- audio device --------------------------------------------------- */
typedef void (*plat_mix_fn)(float *out, int frames, int freq);
/* Opens a stereo float device and calls mix() from the audio thread. */
bool plat_audio_open(int freq, plat_mix_fn mix);
void plat_audio_close(void);
void plat_audio_lock(void);
void plat_audio_unlock(void);
int  plat_audio_freq(void);

#ifdef __cplusplus
}
#endif

#endif

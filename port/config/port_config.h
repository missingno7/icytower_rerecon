/*
 * Port-specific settings (display, renderer, audio), kept in a human-readable
 * INI file in the user directory, separate from the historical tower.cfg /
 * profile / replay files, which are never reinterpreted.
 *
 * File: <user dir>/icytower-port.ini  (created with documented defaults on
 * first run).  Command-line options override the file for one run:
 *   --renderer faithful|modern   --window WxH   --fullscreen   --borderless
 *   --windowed   --filter nearest|linear|pixelart   --interpolation on|off
 *   --vsync on|off   --max-fps N   --widescreen on|off   --ui-scale X
 *   --config PATH (alternative INI file)
 */
#ifndef PORT_CONFIG_H
#define PORT_CONFIG_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct port_audio_config {
   int  frequency;            /* mixer/device rate, Hz */
   int  master_volume;        /* 0..100, applied on top of the game's volumes */
} port_audio_config;

void port_config_load(int argc, char **argv);
const port_audio_config *port_audio_cfg(void);
/* display mode helpers used by the game's own Fullscreen option */
bool port_config_fullscreen(void);
void port_config_set_fullscreen(bool fullscreen);
/* write the current settings back (e.g. after an in-game fullscreen toggle) */
bool port_config_save(void);
/* in-game touch scheme change (applies it and saves the INI) */
void port_config_set_touch_scheme(int scheme);
const char *port_config_path(void);

#ifdef __cplusplus
}
#endif

#endif

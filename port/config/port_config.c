/*
 * INI configuration for the portable build (see port_config.h).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "port/config/port_config.h"
#include "port/render/render.h"
#include "port/platform/platform.h"

void a4_sound_configure(int freq, float master_gain);   /* compat mixer */

static port_audio_config g_audio = { 44100, 100 };
static char g_path[2048];

#ifdef SDL_PLATFORM_ANDROID
#define DEFAULT_MODE_NAME "borderless"
#else
#define DEFAULT_MODE_NAME "windowed"
#endif

static const char *DEFAULT_INI =
"# Icy Tower portable build - port settings.\n"
"# This file only holds settings of the SDL3 port.  Game options, profiles,\n"
"# high scores and replays stay in their historical files (tower.cfg,\n"
"# profiles/) and are not affected by anything here.\n"
"# Delete this file to restore the defaults.\n"
"\n"
"[display]\n"
"# windowed | fullscreen | borderless\n"
"#   fullscreen = exclusive display mode change (uses width/height)\n"
"#   borderless = desktop-resolution fullscreen window\n"
"mode = " DEFAULT_MODE_NAME "\n"
"# initial window size in desktop units (high-DPI screens get more pixels)\n"
"width = 1280\n"
"height = 720\n"
"resizable = true\n"
"# wait for the display refresh when presenting\n"
"vsync = true\n"
"# frame cap in frames per second, 0 = no cap\n"
"max_fps = 0\n"
"# render at the full pixel density of high-DPI displays\n"
"high_dpi = true\n"
"\n"
"[render]\n"
"# modern   = resolution-independent renderer: sprites drawn at output\n"
"#            resolution, widescreen camera, anchored HUD, interpolation\n"
"# faithful = the historical 640x480 frame, scaled to the window\n"
"renderer = modern\n"
"# modern renderer: show extra tower surroundings on wide screens\n"
"# (false = keep the historical 4:3 view, pillar-boxed)\n"
"widescreen = true\n"
"# nearest | linear | pixelart (sharp pixels at any scale)\n"
"texture_filter = nearest\n"
"# smooth movement between the 50 Hz simulation ticks (presentation only;\n"
"# never affects gameplay, scores or replays)\n"
"interpolation = true\n"
"# HUD/menu scale: auto or a number such as 1.0, 1.5, 2\n"
"ui_scale = auto\n"
"# faithful renderer: scale the 640x480 frame by whole multiples only\n"
"integer_scaling = false\n"
"\n"
"[audio]\n"
"# mixer output rate in Hz\n"
"frequency = 44100\n"
"# 0..100, applied on top of the in-game sound and music volumes\n"
"master_volume = 100\n";

static bool parse_bool(const char *v, bool def)
{
   if (!v) return def;
   if (!SDL_strcasecmp(v, "true") || !SDL_strcasecmp(v, "on") || !SDL_strcasecmp(v, "yes") || !strcmp(v, "1"))
      return true;
   if (!SDL_strcasecmp(v, "false") || !SDL_strcasecmp(v, "off") || !SDL_strcasecmp(v, "no") || !strcmp(v, "0"))
      return false;
   return def;
}

static void trim(char *s)
{
   char *e;
   char *b = s;
   while (*b == ' ' || *b == '\t') b++;
   if (b != s) memmove(s, b, strlen(b) + 1);
   e = s + strlen(s);
   while (e > s && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\r' || e[-1] == '\n'))
      *--e = 0;
}

static void apply(const char *section, const char *k, const char *v)
{
   render_config *rc = render_config_mut();
   if (!SDL_strcasecmp(section, "display")) {
      if (!SDL_strcasecmp(k, "mode")) {
         if (!SDL_strcasecmp(v, "fullscreen")) { rc->window_mode = WINMODE_FULLSCREEN; rc->exclusive_fullscreen = true; }
         else if (!SDL_strcasecmp(v, "borderless")) { rc->window_mode = WINMODE_BORDERLESS; rc->exclusive_fullscreen = false; }
         else rc->window_mode = WINMODE_WINDOWED;
      }
      else if (!SDL_strcasecmp(k, "width")) rc->width = SDL_max(320, atoi(v));
      else if (!SDL_strcasecmp(k, "height")) rc->height = SDL_max(240, atoi(v));
      else if (!SDL_strcasecmp(k, "resizable")) rc->resizable = parse_bool(v, rc->resizable);
      else if (!SDL_strcasecmp(k, "vsync")) rc->vsync = parse_bool(v, rc->vsync);
      else if (!SDL_strcasecmp(k, "max_fps")) rc->max_fps = SDL_max(0, atoi(v));
      else if (!SDL_strcasecmp(k, "high_dpi")) rc->high_dpi = parse_bool(v, rc->high_dpi);
   } else if (!SDL_strcasecmp(section, "render")) {
      if (!SDL_strcasecmp(k, "renderer")) rc->modern = SDL_strcasecmp(v, "faithful") != 0;
      else if (!SDL_strcasecmp(k, "widescreen")) rc->widescreen = parse_bool(v, rc->widescreen);
      else if (!SDL_strcasecmp(k, "texture_filter")) {
         rc->filter = !SDL_strcasecmp(v, "linear") ? FILTER_LINEAR :
                      !SDL_strcasecmp(v, "pixelart") ? FILTER_PIXELART : FILTER_NEAREST;
      }
      else if (!SDL_strcasecmp(k, "interpolation")) rc->interpolation = parse_bool(v, rc->interpolation);
      else if (!SDL_strcasecmp(k, "ui_scale")) rc->ui_scale = SDL_strcasecmp(v, "auto") ? (float)atof(v) : 0.0f;
      else if (!SDL_strcasecmp(k, "integer_scaling")) rc->integer_scaling = parse_bool(v, rc->integer_scaling);
   } else if (!SDL_strcasecmp(section, "audio")) {
      if (!SDL_strcasecmp(k, "frequency")) g_audio.frequency = SDL_clamp(atoi(v), 8000, 192000);
      else if (!SDL_strcasecmp(k, "master_volume")) g_audio.master_volume = SDL_clamp(atoi(v), 0, 100);
   }
}

static void load_file(const char *path)
{
   char line[512], section[64] = "";
   FILE *f = fopen(path, "r");
   if (!f)
      return;
   while (fgets(line, sizeof(line), f)) {
      char *eq;
      trim(line);
      if (!line[0] || line[0] == '#' || line[0] == ';')
         continue;
      if (line[0] == '[') {
         char *e = strchr(line, ']');
         if (e) *e = 0;
         SDL_strlcpy(section, line + 1, sizeof(section));
         continue;
      }
      eq = strchr(line, '=');
      if (!eq)
         continue;
      *eq = 0;
      trim(line);
      trim(eq + 1);
      apply(section, line, eq + 1);
   }
   fclose(f);
}

static const char *arg_after(int argc, char **argv, int *i)
{
   if (*i + 1 < argc)
      return argv[++*i];
   return "";
}

void port_config_load(int argc, char **argv)
{
   int i;
   const char *custom = NULL;
   render_config *rc = render_config_mut();
   for (i = 1; i < argc; i++)
      if (!strcmp(argv[i], "--config") && i + 1 < argc)
         custom = argv[i + 1];
   if (custom)
      SDL_strlcpy(g_path, custom, sizeof(g_path));
   else
      SDL_snprintf(g_path, sizeof(g_path), "%sicytower-port.ini", plat_user_dir());
   if (!SDL_GetPathInfo(g_path, NULL) && !plat_headless()) {
      FILE *f = fopen(g_path, "w");
      if (f) {
         fputs(DEFAULT_INI, f);
         fclose(f);
      }
   }
   load_file(g_path);
   a4_sound_configure(g_audio.frequency, (float)g_audio.master_volume / 100.0f);
   for (i = 1; i < argc; i++) {
      const char *a = argv[i];
      if (!strcmp(a, "--renderer")) apply("render", "renderer", arg_after(argc, argv, &i));
      else if (!strcmp(a, "--filter")) apply("render", "texture_filter", arg_after(argc, argv, &i));
      else if (!strcmp(a, "--interpolation")) apply("render", "interpolation", arg_after(argc, argv, &i));
      else if (!strcmp(a, "--widescreen")) apply("render", "widescreen", arg_after(argc, argv, &i));
      else if (!strcmp(a, "--ui-scale")) apply("render", "ui_scale", arg_after(argc, argv, &i));
      else if (!strcmp(a, "--vsync")) apply("display", "vsync", arg_after(argc, argv, &i));
      else if (!strcmp(a, "--max-fps")) apply("display", "max_fps", arg_after(argc, argv, &i));
      else if (!strcmp(a, "--fullscreen")) apply("display", "mode", "fullscreen");
      else if (!strcmp(a, "--borderless")) apply("display", "mode", "borderless");
      else if (!strcmp(a, "--windowed")) apply("display", "mode", "windowed");
      else if (!strcmp(a, "--window")) {
         int w = 0, h = 0;
         if (sscanf(arg_after(argc, argv, &i), "%dx%d", &w, &h) == 2) {
            rc->width = SDL_max(320, w);
            rc->height = SDL_max(240, h);
         }
      }
   }
}

const port_audio_config *port_audio_cfg(void) { return &g_audio; }


bool port_config_fullscreen(void)
{
   return render_get_config()->window_mode != WINMODE_WINDOWED;
}

void port_config_set_fullscreen(bool fullscreen)
{
   render_config *rc = render_config_mut();
   bool now = rc->window_mode != WINMODE_WINDOWED;
   if (now == fullscreen)
      return;
   /* keep the configured fullscreen flavour when toggling back */
   rc->window_mode = fullscreen ? (rc->exclusive_fullscreen ? WINMODE_FULLSCREEN : WINMODE_BORDERLESS)
                                : WINMODE_WINDOWED;
   port_config_save();
}
const char *port_config_path(void) { return g_path; }

bool port_config_save(void)
{
   /* The file is user-edited documentation as much as data; only the
    * display mode is ever changed from inside the game, so rewrite just
    * that key and keep comments intact. */
   char *text;
   size_t n;
   FILE *f;
   const render_config *rc = render_get_config();
   const char *mode = rc->window_mode == WINMODE_FULLSCREEN ? "fullscreen" :
                      rc->window_mode == WINMODE_BORDERLESS ? "borderless" : "windowed";
   text = (char *)SDL_LoadFile(g_path, &n);
   if (!text)
      return false;
   f = fopen(g_path, "w");
   if (f) {
      char *line = text, *next;
      while (line && *line) {
         next = strchr(line, '\n');
         if (next) *next++ = 0;
         if (!strncmp(line, "mode", 4) && strchr(line, '='))
            fprintf(f, "mode = %s\n", mode);
         else
            fprintf(f, "%s\n", line);
         line = next;
      }
      fclose(f);
   }
   SDL_free(text);
   return f != NULL;
}

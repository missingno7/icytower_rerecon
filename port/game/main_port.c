/*
 * Program entry point of the portable build.  Initialises the platform layer
 * and runs the historical game main (_mangled_main in src/main.c).
 */
#include <stdio.h>
#include <string.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "port/platform/platform.h"
#include "port/config/port_config.h"
#include "port/render/capture.h"
#include "port/render/render.h"
#include "port/sim/sched.h"

#ifdef _WIN32
#include <windows.h>
#endif

int _mangled_main(int argc, char **argv);
void port_sim_trace_configure(const char *path);

/* Port options (platform, config, capture); everything else is passed to the
 * historical command-line parser unchanged ("-check FILE -all", or a replay/
 * profile file to open). */
static const struct { const char *name; int args; } port_opts[] = {
   { "--data", 1 }, { "--user-dir", 1 }, { "--config", 1 }, { "--headless", 0 },
   { "--renderer", 1 }, { "--filter", 1 }, { "--interpolation", 1 }, { "--widescreen", 1 },
   { "--ui-scale", 1 }, { "--vsync", 1 }, { "--max-fps", 1 }, { "--fullscreen", 0 },
   { "--borderless", 0 }, { "--windowed", 0 }, { "--window", 1 },
   { "--capture", 1 }, { "--capture-ms", 1 }, { "--capture-ticks", 1 },
   { "--exit-after-ms", 1 }, { "--exit-after-ticks", 1 },
   { "--sim-trace", 1 }, { "--sim-speed", 1 },
};

static int strip_port_args(int argc, char **argv, char **out)
{
   int i, n = 0;
   for (i = 0; i < argc; i++) {
      size_t k;
      int skip = -1;
      if (i > 0)
         for (k = 0; k < sizeof(port_opts) / sizeof(port_opts[0]); k++)
            if (!strcmp(argv[i], port_opts[k].name))
               skip = port_opts[k].args;
      if (skip < 0)
         out[n++] = argv[i];
      else
         i += skip;
   }
   out[n] = NULL;
   return n;
}

static void attach_console_if_checking(int argc, char **argv)
{
#ifdef _WIN32
   int i;
   for (i = 1; i < argc; i++) {
      if (!SDL_strcasecmp(argv[i], "-check") || !SDL_strcasecmp(argv[i], "--headless")) {
         /* GUI-subsystem executable: reuse the parent console for the
          * replay checker's XML unless stdout is already redirected. */
         if (GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_UNKNOWN &&
             AttachConsole(ATTACH_PARENT_PROCESS)) {
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
         }
         break;
      }
   }
#else
   (void)argc; (void)argv;
#endif
}

int main(int argc, char **argv)
{
   int ret;
   attach_console_if_checking(argc, argv);
   if (!plat_init(argc, argv))
      return 1;
   port_config_load(argc, argv);
   capture_configure(argc, argv);
   {
      int i;
      for (i = 1; i + 1 < argc; i++) {
         if (!strcmp(argv[i], "--sim-trace"))
            port_sim_trace_configure(argv[i + 1]);
         else if (!strcmp(argv[i], "--sim-speed"))
            sched_set_speed(SDL_atof(argv[i + 1]));
      }
   }
   {
      char **game_argv = (char **)SDL_calloc((size_t)argc + 1, sizeof(char *));
      int game_argc = strip_port_args(argc, argv, game_argv);
      ret = _mangled_main(game_argc, game_argv);
      SDL_free(game_argv);
   }
   if (!plat_headless())
      SDL_Log("frames presented %llu, simulation ticks %llu, dropped ticks %llu",
              (unsigned long long)render_frames_presented(), (unsigned long long)sched_ticks(),
              (unsigned long long)sched_dropped_ticks());
   plat_shutdown();
   return ret;
}

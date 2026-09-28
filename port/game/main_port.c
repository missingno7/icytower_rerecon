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

#ifdef _WIN32
#include <windows.h>
#endif

int _mangled_main(int argc, char **argv);

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
   ret = _mangled_main(argc, argv);
   plat_shutdown();
   return ret;
}

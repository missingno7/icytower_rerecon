/*
 * Small platform-facing leftovers: path canonicalisation and the file
 * selector used by the replay browser (F1 = choose another folder).
 */
#include <string.h>
#include <SDL3/SDL.h>
#include "a4_internal.h"
#include "port/platform/platform.h"

char *canonicalize_filename(char *dest, const char *filename, int size)
{
   char native[2048];
   if (!plat_resolve_read(filename, native, sizeof(native)))
      SDL_strlcpy(native, filename, sizeof(native));
   SDL_strlcpy(dest, native, (size_t)size);
   return dest;
}

void remove_mouse(void) { }

static volatile int dlg_done;
static char dlg_result[2048];

static void SDLCALL dlg_cb(void *userdata, const char * const *filelist, int filter)
{
   (void)userdata; (void)filter;
   dlg_result[0] = 0;
   if (filelist && filelist[0])
      SDL_snprintf(dlg_result, sizeof(dlg_result), "%s/", filelist[0]);
   dlg_done = 1;
}

int file_select_ex(const char *message, char *path, const char *ext, int size, int w, int h)
{
   char start[2048];
   (void)message; (void)ext; (void)w; (void)h;
   if (plat_headless())
      return 0;
   canonicalize_filename(start, path, sizeof(start));
   dlg_done = 0;
   SDL_ShowOpenFolderDialog(dlg_cb, NULL, NULL, start, false);
   while (!dlg_done) {
      a4_service();
      plat_sleep_ns(5000000u);
   }
   if (!dlg_result[0])
      return 0;
   SDL_strlcpy(path, dlg_result, (size_t)size);
   return 1;
}

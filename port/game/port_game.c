#include <string.h>
#include <SDL3/SDL.h>
#include "port/game/port_game.h"
#include "port/platform/platform.h"
#include "port/sim/sched.h"

FILE *port_fopen(const char *path, const char *mode)
{
   char native[2048];
   bool write = strchr(mode, 'w') || strchr(mode, 'a') || strchr(mode, '+');
   if (!path)
      return NULL;
   if (write ? !plat_resolve_write(path, native, sizeof(native))
             : !plat_resolve_read(path, native, sizeof(native)))
      return NULL;
   return fopen(native, mode);
}

int port_mkdir(const char *path)
{
   char native[2048];
   size_t n;
   if (!plat_resolve_write(path, native, sizeof(native)))
      return -1;
   n = strlen(native);
   while (n > 0 && (native[n - 1] == '/' || native[n - 1] == '\\'))
      native[--n] = 0;
   return plat_mkdir_p(native) ? 0 : -1;
}

int port_stricmp(const char *a, const char *b)
{
   return SDL_strcasecmp(a, b);
}

/* MSVCRT: holdrand = holdrand * 214013 + 2531011; return (holdrand >> 16) & 0x7fff */
static uint32_t hist_holdrand = 1;

int hist_rand(void)
{
   hist_holdrand = hist_holdrand * 214013u + 2531011u;
   return (int)((hist_holdrand >> 16) & 0x7FFF);
}

void hist_srand(unsigned int seed)
{
   hist_holdrand = seed;
}

int port_qpc_low(void) { return (int)(uint32_t)plat_perf_counter(); }
int port_qpf_low(void) { return (int)(uint32_t)plat_perf_frequency(); }

void port_open_url(const char *url) { plat_open_url(url); }

void port_wait_tick(void)
{
   sched_wait_tick();
}

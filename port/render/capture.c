#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "port/render/capture.h"
#include "port/sim/sched.h"
#include "port/platform/platform.h"

#define MAX_POINTS 64

static char g_dir[1024];
static uint64_t g_ms[MAX_POINTS], g_ticks[MAX_POINTS];
static int g_nms, g_nticks, g_ms_done, g_ticks_done;
static uint64_t g_exit_ms, g_exit_ticks, g_start_ns;
static int g_seq;

static int parse_list(const char *s, uint64_t *out)
{
   int n = 0;
   while (s && *s && n < MAX_POINTS) {
      out[n++] = strtoull(s, (char **)&s, 10);
      if (*s == ',') s++;
      else break;
   }
   return n;
}

void capture_configure(int argc, char **argv)
{
   int i;
   g_start_ns = plat_ticks_ns();
   for (i = 1; i + 1 < argc; i++) {
      if (!strcmp(argv[i], "--capture")) SDL_strlcpy(g_dir, argv[i + 1], sizeof(g_dir));
      else if (!strcmp(argv[i], "--capture-ms")) g_nms = parse_list(argv[i + 1], g_ms);
      else if (!strcmp(argv[i], "--capture-ticks")) g_nticks = parse_list(argv[i + 1], g_ticks);
      else if (!strcmp(argv[i], "--exit-after-ms")) g_exit_ms = strtoull(argv[i + 1], NULL, 10);
      else if (!strcmp(argv[i], "--exit-after-ticks")) g_exit_ticks = strtoull(argv[i + 1], NULL, 10);
   }
   if (g_dir[0])
      SDL_CreateDirectory(g_dir);
}

static void save(SDL_Renderer *r, const char *tag)
{
   char path[1200];
   SDL_Surface *s = SDL_RenderReadPixels(r, NULL);
   if (!s)
      return;
   SDL_snprintf(path, sizeof(path), "%s/frame_%02d_%s.png", g_dir, g_seq++, tag);
   SDL_SavePNG(s, path);
   SDL_Log("captured %s (%dx%d)", path, s->w, s->h);
   SDL_DestroySurface(s);
}

void capture_frame(SDL_Renderer *r)
{
   uint64_t ms = (plat_ticks_ns() - g_start_ns) / 1000000u;
   uint64_t t = sched_ticks();
   char tag[64];
   if (!g_dir[0])
      return;
   if (g_ms_done < g_nms && ms >= g_ms[g_ms_done]) {
      SDL_snprintf(tag, sizeof(tag), "%llums", (unsigned long long)g_ms[g_ms_done]);
      g_ms_done++;
      save(r, tag);
   }
   if (g_ticks_done < g_nticks && t >= g_ticks[g_ticks_done]) {
      SDL_snprintf(tag, sizeof(tag), "tick%llu", (unsigned long long)g_ticks[g_ticks_done]);
      g_ticks_done++;
      save(r, tag);
   }
}

bool capture_exit_requested(void)
{
   uint64_t ms = (plat_ticks_ns() - g_start_ns) / 1000000u;
   if (g_exit_ms && ms >= g_exit_ms)
      return true;
   if (g_exit_ticks && sched_ticks() >= g_exit_ticks)
      return true;
   return false;
}

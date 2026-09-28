/*
 * SDL3 implementation of port/platform/platform.h: lifetime, clock, events,
 * message boxes, file-system roots and the audio device.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "port/platform/plat_internal.h"
#include "port/render/capture.h"

#define MAX_ASSET_DIRS 4
#define MAX_HANDLERS 8

static bool g_inited, g_headless, g_quit, g_focus = true;
static char g_user_dir[1024];
static char g_asset_dirs[MAX_ASSET_DIRS][1024];
static int g_asset_count;
static plat_event_fn g_handlers[MAX_HANDLERS];
static int g_handler_count;
static void (*g_on_focus_in)(void), (*g_on_focus_out)(void);

static SDL_AudioStream *g_audio;
static plat_mix_fn g_mix;
static int g_audio_freq;

/* ------------------------------------------------------------ helpers */

static void add_trailing_sep(char *p, size_t n)
{
   size_t l = strlen(p);
   if (l && p[l - 1] != '/' && p[l - 1] != '\\' && l + 1 < n) {
      p[l] = '/';
      p[l + 1] = 0;
   }
}

static void add_asset_dir(const char *d)
{
   int i;
   char buf[1024];
   if (!d || !*d || g_asset_count >= MAX_ASSET_DIRS)
      return;
   SDL_strlcpy(buf, d, sizeof(buf));
   add_trailing_sep(buf, sizeof(buf));
   for (i = 0; i < g_asset_count; i++)
      if (!SDL_strcasecmp(g_asset_dirs[i], buf))
         return;
   SDL_strlcpy(g_asset_dirs[g_asset_count++], buf, sizeof(buf));
}

static const char *arg_value(int argc, char **argv, const char *name)
{
   int i;
   for (i = 1; i + 1 < argc; i++)
      if (!strcmp(argv[i], name))
         return argv[i + 1];
   return NULL;
}

static bool has_arg(int argc, char **argv, const char *name)
{
   int i;
   for (i = 1; i < argc; i++)
      if (!SDL_strcasecmp(argv[i], name))
         return true;
   return false;
}

/* ------------------------------------------------------------ lifetime */

bool plat_init(int argc, char **argv)
{
   const char *v;
   SDL_InitFlags flags = SDL_INIT_EVENTS;
   if (g_inited)
      return true;
   g_headless = has_arg(argc, argv, "--headless") || has_arg(argc, argv, "-check") ||
                getenv("ITOWER_HEADLESS") != NULL;
   if (!g_headless)
      flags |= SDL_INIT_VIDEO | SDL_INIT_GAMEPAD;
   SDL_SetAppMetadata("Icy Tower (portable)", "1.5.1-port", "io.github.icytower-rerecon.port");
   if (!SDL_Init(flags)) {
      fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
      return false;
   }
   g_inited = true;

   /* user root */
   v = arg_value(argc, argv, "--user-dir");
   if (!v) v = getenv("ITOWER_USER_DIR");
   if (v) {
      SDL_strlcpy(g_user_dir, v, sizeof(g_user_dir));
   } else {
      char *pref = SDL_GetPrefPath("IcyTowerPort", "IcyTower");
      if (pref) {
         SDL_strlcpy(g_user_dir, pref, sizeof(g_user_dir));
         SDL_free(pref);
      } else {
         SDL_strlcpy(g_user_dir, "./", sizeof(g_user_dir));
      }
   }
   add_trailing_sep(g_user_dir, sizeof(g_user_dir));
   plat_mkdir_p(g_user_dir);

   /* asset roots */
   add_asset_dir(arg_value(argc, argv, "--data"));
   add_asset_dir(getenv("ITOWER_DATA_DIR"));
   add_asset_dir(SDL_GetBasePath());
   {
      char *cwd = SDL_GetCurrentDirectory();
      if (cwd) {
         add_asset_dir(cwd);
         SDL_free(cwd);
      }
   }
   return true;
}

void plat_shutdown(void)
{
   if (!g_inited)
      return;
   plat_audio_close();
   SDL_Quit();
   g_inited = false;
}

bool plat_headless(void) { return g_headless; }

/* ------------------------------------------------------------ clock */

uint64_t plat_ticks_ns(void) { return SDL_GetTicksNS(); }
void plat_sleep_ns(uint64_t ns) { SDL_DelayPrecise(ns); }
uint64_t plat_perf_counter(void) { return SDL_GetPerformanceCounter(); }
uint64_t plat_perf_frequency(void) { return SDL_GetPerformanceFrequency(); }

/* ------------------------------------------------------------ events */

void plat_add_event_handler(plat_event_fn fn)
{
   int i;
   for (i = 0; i < g_handler_count; i++)
      if (g_handlers[i] == fn)
         return;
   if (g_handler_count < MAX_HANDLERS)
      g_handlers[g_handler_count++] = fn;
}

void plat_set_focus_callbacks(void (*on_in)(void), void (*on_out)(void))
{
   g_on_focus_in = on_in;
   g_on_focus_out = on_out;
}

void plat_pump_events(void)
{
   SDL_Event ev;
   int i;
   if (!g_inited)
      return;
   while (SDL_PollEvent(&ev)) {
      switch (ev.type) {
         case SDL_EVENT_QUIT:
         case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            g_quit = true;
            break;
         case SDL_EVENT_WINDOW_FOCUS_GAINED:
            if (!g_focus && g_on_focus_in) g_on_focus_in();
            g_focus = true;
            break;
         case SDL_EVENT_WINDOW_FOCUS_LOST:
            if (g_focus && g_on_focus_out) g_on_focus_out();
            g_focus = false;
            break;
         default:
            break;
      }
      for (i = 0; i < g_handler_count; i++)
         g_handlers[i](&ev);
   }
}

bool plat_quit_requested(void) { return g_quit || capture_exit_requested(); }
void plat_clear_quit_request(void) { g_quit = false; }
/* note: a capture exit request (--exit-after-*) stays active so every
 * later wait loop sees it too */
bool plat_has_focus(void) { return g_focus; }

/* ------------------------------------------------------------ messages */

void plat_message_box(const char *title, const char *text)
{
   if (g_headless || !g_inited || !SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, text, NULL))
      fprintf(stderr, "%s: %s\n", title ? title : "", text ? text : "");
}

int plat_choice_box(const char *title, const char *text, const char *b1, const char *b2)
{
   SDL_MessageBoxButtonData buttons[2];
   SDL_MessageBoxData data;
   int n = 0, hit = 0;
   if (g_headless || !g_inited) {
      fprintf(stderr, "%s: %s\n", title ? title : "", text ? text : "");
      return 0;
   }
   if (b1) {
      buttons[n].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
      buttons[n].buttonID = 0;
      buttons[n].text = b1;
      n++;
   }
   if (b2) {
      buttons[n].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
      buttons[n].buttonID = 1;
      buttons[n].text = b2;
      n++;
   }
   SDL_zero(data);
   data.flags = SDL_MESSAGEBOX_INFORMATION;
   data.title = title ? title : "";
   data.message = text ? text : "";
   data.numbuttons = n;
   data.buttons = buttons;
   if (!SDL_ShowMessageBox(&data, &hit))
      return 0;
   return hit < 0 ? 0 : hit;
}

bool plat_open_url(const char *url) { return g_inited && SDL_OpenURL(url); }

/* ------------------------------------------------------------ files */

const char *plat_user_dir(void) { return g_user_dir; }
int plat_asset_dir_count(void) { return g_asset_count; }
const char *plat_asset_dir(int i) { return (i >= 0 && i < g_asset_count) ? g_asset_dirs[i] : NULL; }

bool plat_is_absolute(const char *p)
{
   if (!p || !*p)
      return false;
   if (p[0] == '/' || p[0] == '\\')
      return true;
   if (((p[0] >= 'A' && p[0] <= 'Z') || (p[0] >= 'a' && p[0] <= 'z')) && p[1] == ':')
      return true;
   return false;
}

static bool join(char *out, size_t n, const char *root, const char *rel)
{
   while (rel[0] == '.' && (rel[1] == '/' || rel[1] == '\\'))
      rel += 2;
   return (size_t)SDL_snprintf(out, n, "%s%s", root, rel) < n;
}

bool plat_resolve_read(const char *game_path, char *out, size_t outsz)
{
   int i;
   if (!game_path)
      return false;
   if (plat_is_absolute(game_path) || !g_inited)
      return (size_t)SDL_snprintf(out, outsz, "%s", game_path) < outsz;
   if (!join(out, outsz, g_user_dir, game_path))
      return false;
   if (SDL_GetPathInfo(out, NULL))
      return true;
   for (i = 0; i < g_asset_count; i++) {
      char tmp[2048];
      if (join(tmp, sizeof(tmp), g_asset_dirs[i], game_path) && SDL_GetPathInfo(tmp, NULL))
         return (size_t)SDL_snprintf(out, outsz, "%s", tmp) < outsz;
   }
   return join(out, outsz, g_user_dir, game_path);
}

bool plat_mkdir_p(const char *native_path)
{
   return SDL_CreateDirectory(native_path);
}

bool plat_resolve_write(const char *game_path, char *out, size_t outsz)
{
   char dir[2048];
   char *slash;
   if (!game_path)
      return false;
   if (plat_is_absolute(game_path) || !g_inited)
      return (size_t)SDL_snprintf(out, outsz, "%s", game_path) < outsz;
   if (!join(out, outsz, g_user_dir, game_path))
      return false;
   SDL_strlcpy(dir, out, sizeof(dir));
   slash = SDL_strrchr(dir, '/');
   if (!slash || SDL_strrchr(dir, '\\') > slash)
      slash = SDL_strrchr(dir, '\\');
   if (slash) {
      *slash = 0;
      if (*dir)
         SDL_CreateDirectory(dir);
   }
   return true;
}

/* ------------------------------------------------------------ audio */

static void SDLCALL audio_cb(void *userdata, SDL_AudioStream *stream, int additional, int total)
{
   float buf[2048];
   (void)userdata; (void)total;
   while (additional > 0) {
      int frames = additional / (int)(2 * sizeof(float));
      if (frames <= 0)
         break;
      if (frames > 1024)
         frames = 1024;
      memset(buf, 0, sizeof(float) * 2 * (size_t)frames);
      if (g_mix)
         g_mix(buf, frames, g_audio_freq);
      SDL_PutAudioStreamData(stream, buf, frames * 2 * (int)sizeof(float));
      additional -= frames * 2 * (int)sizeof(float);
   }
}

bool plat_audio_open(int freq, plat_mix_fn mix)
{
   SDL_AudioSpec spec;
   if (g_audio)
      return true;
   if (g_headless || !g_inited)
      return false;
   if (!SDL_WasInit(SDL_INIT_AUDIO) && !SDL_InitSubSystem(SDL_INIT_AUDIO))
      return false;
   spec.format = SDL_AUDIO_F32;
   spec.channels = 2;
   spec.freq = freq;
   g_mix = mix;
   g_audio_freq = freq;
   g_audio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, audio_cb, NULL);
   if (!g_audio)
      return false;
   SDL_ResumeAudioStreamDevice(g_audio);
   return true;
}

void plat_audio_close(void)
{
   if (g_audio) {
      SDL_DestroyAudioStream(g_audio);
      g_audio = NULL;
   }
}

void plat_audio_lock(void) { if (g_audio) SDL_LockAudioStream(g_audio); }
void plat_audio_unlock(void) { if (g_audio) SDL_UnlockAudioStream(g_audio); }
int plat_audio_freq(void) { return g_audio_freq; }

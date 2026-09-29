/*
 * Unit test: file functions of the compat layer against the platform
 * file-system roots (user root first, then asset roots; writes to the
 * user root; absolute paths pass through), plus memory packfiles.
 * Runs headless with temporary --user-dir / --data directories.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "a4_internal.h"
#include "port/platform/platform.h"

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; \
   fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); } } while (0)

static char base[1024], udir[1024], ddir[1024];

static void put_file(const char *root, const char *rel, const char *text)
{
   char path[2048], dir[2048], *slash;
   SDL_IOStream *io;
   SDL_snprintf(path, sizeof(path), "%s%s", root, rel);
   SDL_strlcpy(dir, path, sizeof(dir));
   slash = SDL_strrchr(dir, '/');
   if (slash) {
      *slash = 0;
      SDL_CreateDirectory(dir);
   }
   io = SDL_IOFromFile(path, "wb");
   if (io) {
      SDL_WriteIO(io, text, SDL_strlen(text));
      SDL_CloseIO(io);
   }
}

static int on_disk(const char *root, const char *rel)
{
   char path[2048];
   SDL_snprintf(path, sizeof(path), "%s%s", root, rel);
   return SDL_GetPathInfo(path, NULL);
}

static SDL_EnumerationResult SDLCALL rm_cb(void *ud, const char *dirname, const char *fname)
{
   char path[2048];
   SDL_PathInfo info;
   (void)ud;
   SDL_snprintf(path, sizeof(path), "%s%s", dirname, fname);
   if (SDL_GetPathInfo(path, &info) && info.type == SDL_PATHTYPE_DIRECTORY)
      SDL_EnumerateDirectory(path, rm_cb, NULL);
   SDL_RemovePath(path);
   return SDL_ENUM_CONTINUE;
}

static void rm_tree(const char *dir)
{
   SDL_EnumerateDirectory(dir, rm_cb, NULL);
   SDL_RemovePath(dir);
}

static char seen[4096];

static int collect(const char *filename, int attrib, void *param)
{
   (void)param;
   SDL_strlcat(seen, filename, sizeof(seen));
   SDL_strlcat(seen, (attrib & FA_DIREC) ? "/|" : "|", sizeof(seen));
   return 0;
}

static long read_all(const char *name, const char *mode, char *buf, long max)
{
   PACKFILE *f = pack_fopen(name, mode);
   long n;
   if (!f)
      return -1;
   n = pack_fread(buf, max - 1, f);
   buf[n < 0 ? 0 : n] = 0;
   pack_fclose(f);
   return n;
}

static void test_resolution(void)
{
   char buf[256];
   PACKFILE *f;
   int a = 0;

   /* reading: user root first, then assets */
   CHECK(exists("data/a.dat"));
   CHECK(file_size_ex("data/a.dat") == 5);
   CHECK(read_all("shared.txt", F_READ, buf, sizeof(buf)) == 4 && !strcmp(buf, "user"));
   CHECK(read_all("data/a.dat", F_READ, buf, sizeof(buf)) == 5 && !strcmp(buf, "asset"));
   CHECK(!exists("missing.txt"));
   CHECK(file_size_ex("missing.txt") == 0);
   CHECK(file_exists("characters", FA_DIREC, &a) && (a & FA_DIREC));
   CHECK(!exists("characters"));
   CHECK(!file_exists("characters/", FA_DIREC, NULL));   /* trailing separator, like Windows */
   CHECK(exists("./data/a.dat"));
   CHECK(file_exists("data/*.dat", FA_ARCH | FA_RDONLY, NULL));

   /* enumeration: union of the roots, de-duplicated, user root first */
   seen[0] = 0;
   CHECK(for_each_file_ex("characters/*", FA_DIREC, 0, collect, NULL) == 5);
   CHECK(!strcmp(seen, "characters/./|characters/../|characters/c1/|characters/c2/|characters/c3/|"));
   seen[0] = 0;
   for_each_file_ex("characters/c1/*.txt", 0, FA_DIREC, collect, NULL);
   CHECK(!strcmp(seen, "characters/c1/c1.txt|characters/c1/extra.txt|"));
   CHECK(exists("characters/c3/c3.txt"));
   CHECK(exists("characters/c2/c2.txt"));
   seen[0] = 0;
   CHECK(for_each_file_ex("nothing/*", 0, 0, collect, NULL) == 0);

   /* writing goes to the user root; parents are created */
   f = pack_fopen("profiles/bob/bob.itp", F_WRITE);
   CHECK(f != NULL);
   if (f) {
      pack_fwrite("profile", 7, f);
      CHECK(pack_fclose(f) == 0);
   }
   CHECK(on_disk(udir, "profiles/bob/bob.itp"));
   CHECK(!on_disk(ddir, "profiles/bob/bob.itp"));
   CHECK(exists("profiles/bob/bob.itp"));
   CHECK(file_exists("profiles/bob", FA_DIREC, NULL));

   /* packed round trip through the roots */
   packfile_password(NULL);
   f = pack_fopen("tower.cfg", F_WRITE_PACKED);
   CHECK(f != NULL);
   if (f) {
      int i;
      for (i = 0; i < 3000; i++)
         pack_putc("icy tower "[i % 10], f);
      pack_fclose(f);
   }
   f = pack_fopen("tower.cfg", F_READ_PACKED);
   CHECK(f != NULL);
   if (f) {
      int i, ok = 1;
      for (i = 0; i < 3000; i++)
         ok &= pack_getc(f) == "icy tower "[i % 10];
      CHECK(ok);
      CHECK(pack_getc(f) == EOF);
      pack_fclose(f);
   }
   CHECK(on_disk(udir, "tower.cfg"));

   /* deleting only touches the user root */
   CHECK(delete_file("shared.txt") == 0);
   CHECK(read_all("shared.txt", F_READ, buf, sizeof(buf)) == 5 && !strcmp(buf, "asset"));
   CHECK(delete_file("data/a.dat") != 0);
   CHECK(on_disk(ddir, "data/a.dat"));
   CHECK(delete_file("profiles/bob") != 0);   /* not a file */

   /* absolute paths pass through */
   SDL_snprintf(buf, sizeof(buf), "%sdata/a.dat", ddir);
   CHECK(exists(buf));
   CHECK(file_size_ex(buf) == 5);

   /* config and datafiles through the roots */
   put_file(udir, "gamepad.txt", "up = jump\n[x]\nup = no\n");
   set_config_file("gamepad.txt");
   CHECK(!strcmp(get_config_string(NULL, "up", "nothing"), "jump"));
   CHECK(!strcmp(get_config_string("x", "up", "nothing"), "no"));
   CHECK(!strcmp(get_config_string(NULL, "down", "nothing"), "nothing"));

   SDL_snprintf(buf, sizeof(buf), "%sdata/t.dat", ddir);
   f = pack_fopen(buf, F_WRITE_NOPACK);
   CHECK(f != NULL);
   if (f) {
      pack_mputl(DAT_MAGIC, f);
      pack_mputl(1, f);
      pack_mputl(DAT_DATA, f);
      pack_mputl(3, f);
      pack_mputl(3, f);
      pack_fwrite("xyz", 3, f);
      pack_fclose(f);
   }
   {
      DATAFILE *d = load_datafile("data/t.dat");
      CHECK(d != NULL);
      if (d) {
         CHECK(d[0].type == DAT_DATA && d[0].size == 3 && !memcmp(d[0].dat, "xyz", 3));
         CHECK(d[1].type == DAT_END);
         unload_datafile(d);
      }
   }
}

static void test_memory_packfile(void)
{
   static const unsigned char blob[] = { 0x12, 0x34, 0x56, 0x78, 'a', 'b', 'c' };
   PACKFILE *f;
   char buf[8];

   packfile_password("CHEESE");   /* must not apply to memory packfiles */
   f = a4_pack_fopen_memory(blob, sizeof(blob));
   CHECK(f != NULL);
   if (f) {
      CHECK(pack_mgetl(f) == 0x12345678L);
      CHECK(pack_fread(buf, 8, f) == 3 && !memcmp(buf, "abc", 3));
      CHECK(pack_feof(f));
      CHECK(pack_getc(f) == EOF);
      pack_fclose(f);
   }
   packfile_password(NULL);
   f = a4_pack_fopen_memory(blob, 0);
   CHECK(f != NULL);
   if (f) {
      CHECK(pack_getc(f) == EOF);
      pack_fclose(f);
   }
}

/* Windows-authored names must be found with any case on case-sensitive
 * systems (plat_resolve_read's fallback); on Windows the OS already does. */
static void test_case_insensitive(void)
{
   char out[2048];
   CHECK(plat_resolve_read("casetest/mixed/frames.png", out, sizeof(out)));
   CHECK(SDL_GetPathInfo(out, NULL));
   CHECK(plat_resolve_read("CASETEST/MIXED/FRAMES.png", out, sizeof(out)));
   CHECK(SDL_GetPathInfo(out, NULL));
   CHECK(read_all("casetest/mixed/frames.png", "rb", out, sizeof(out)) == 1);
}

static void test_path_helpers(void)
{
   char buf[64];
   CHECK(!strcmp(get_filename("a/b\\c.txt"), "c.txt"));
   CHECK(!strcmp(get_extension("dir.d/file.PNG"), "PNG"));
   CHECK(!strcmp(get_extension("dir.d/file"), ""));
   CHECK(!strcmp(replace_extension(buf, "x/y.pcx", "png", sizeof(buf)), "x/y.png"));
   CHECK(!strcmp(replace_filename(buf, "C:\\g\\icy.exe", "", sizeof(buf)), "C:\\g\\"));
}

int main(int argc, char **argv)
{
   char *pargv[7];
   const char *tmp = getenv("TEMP");
   (void)argc; (void)argv;
   if (!tmp) tmp = getenv("TMPDIR");
   if (!tmp) tmp = "/tmp";

   SDL_snprintf(base, sizeof(base), "%s/a4_test_files_%u/", tmp, (unsigned)SDL_GetPerformanceCounter());
   SDL_snprintf(udir, sizeof(udir), "%suser/", base);
   SDL_snprintf(ddir, sizeof(ddir), "%sdata/", base);
   SDL_CreateDirectory(udir);
   SDL_CreateDirectory(ddir);

   put_file(ddir, "data/a.dat", "asset");
   put_file(ddir, "shared.txt", "asset");
   put_file(udir, "shared.txt", "user");
   put_file(ddir, "characters/c1/c1.txt", "1");
   put_file(ddir, "characters/c2/c2.txt", "2");
   put_file(udir, "characters/c3/c3.txt", "3");
   put_file(udir, "characters/c1/extra.txt", "x");
   put_file(ddir, "casetest/Mixed/Frames.PNG", "m");   /* case test */

   pargv[0] = "test_files";
   pargv[1] = "--headless";
   pargv[2] = "--user-dir";
   pargv[3] = udir;
   pargv[4] = "--data";
   pargv[5] = ddir;
   pargv[6] = NULL;
   if (!plat_init(6, pargv)) {
      fprintf(stderr, "plat_init failed\n");
      return 2;
   }

   test_path_helpers();
   test_memory_packfile();
   test_resolution();
   test_case_insensitive();

   plat_shutdown();
   rm_tree(base);
   if (failures)
      fprintf(stderr, "test_files: %d failure(s)\n", failures);
   else
      printf("test_files: ok\n");
   return failures ? 1 : 0;
}

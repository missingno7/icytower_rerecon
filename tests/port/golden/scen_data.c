/*
 * Golden scenario: packfiles (plain, LZSS, passwords), datafiles as the game
 * loads them, file utilities and config files.  C89 (built by GCC 4.4 too).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "golden.h"

static char *rec_name(const char *fmt, const char *a, int i, const char *b)
{
   static char buf[4][512];
   static int n;
   char *p = buf[n++ & 3];
   sprintf(p, fmt, a, i, b);
   return p;
}

static const char *tmp_path(const char *name)
{
   static char buf[4][1024];
   static int n;
   char *p = buf[n++ & 3];
   const char *t = getenv("TEMP");
   if (!t) t = getenv("TMP");
   if (!t) t = ".";
   sprintf(p, "%s/a4golden_%s", t, name);
   return p;
}

static void put_errno(golden_ctx *g, const char *name)
{
   golden_int(g, name, *allegro_errno);
}

/* ------------------------------------------------------------------ */
/* datafiles                                                          */
/* ------------------------------------------------------------------ */

static int utf8_encode(char *s, int c)
{
   if (c < 0x80) { s[0] = (char)c; return 1; }
   if (c < 0x800) { s[0] = (char)(0xC0 | (c >> 6)); s[1] = (char)(0x80 | (c & 0x3F)); return 2; }
   s[0] = (char)(0xE0 | (c >> 12));
   s[1] = (char)(0x80 | ((c >> 6) & 0x3F));
   s[2] = (char)(0x80 | (c & 0x3F));
   return 3;
}

static void dump_font_metrics(golden_ctx *g, const char *prefix, FONT *f)
{
   static unsigned char buf[4 * 0x200];
   char s[8];
   int c, n = 0, len;
   golden_int(g, rec_name("%s/%d%s", prefix, 0, "/height"), text_height(f));
   for (c = 1; c < 0x180; c++) {
      len = utf8_encode(s, c);
      s[len] = 0;
      len = text_length(f, s);
      buf[n++] = (unsigned char)len; buf[n++] = (unsigned char)(len >> 8);
      buf[n++] = (unsigned char)(len >> 16); buf[n++] = (unsigned char)(len >> 24);
   }
   golden_bytes(g, rec_name("%s/%d%s", prefix, 0, "/lengths"), buf, n);
   golden_int(g, rec_name("%s/%d%s", prefix, 0, "/euro"), text_length(f, "\xE2\x82\xAC"));
   golden_int(g, rec_name("%s/%d%s", prefix, 0, "/a4"), text_length(f, "\xA4"));
   golden_int(g, rec_name("%s/%d%s", prefix, 0, "/bad"), text_length(f, "\xC3(\xFF\xFE"));
}

static void dump_datafile(golden_ctx *g, const char *prefix, DATAFILE *d)
{
   int i, p;
   char sub[256];
   if (!d) {
      golden_int(g, rec_name("%s/%d%s", prefix, 0, "/null"), 1);
      return;
   }
   for (i = 0; d[i].type != DAT_END; i++) {
      DATAFILE *o = &d[i];
      golden_int(g, rec_name("%s/%d%s", prefix, i, "/type"), o->type);
      golden_int(g, rec_name("%s/%d%s", prefix, i, "/size"), o->size);
      if (o->prop) {
         for (p = 0; o->prop[p].type != DAT_END; p++) {
            char pn[32];
            sprintf(pn, "/prop%d", p);
            golden_int(g, rec_name("%s/%d%s", prefix, i, pn), o->prop[p].type);
            golden_bytes(g, rec_name("%s/%d%s.dat", prefix, i, pn), o->prop[p].dat,
                         (long)strlen(o->prop[p].dat));
         }
      }
      switch (o->type) {
         case DAT_BITMAP:
            golden_bitmap(g, rec_name("%s/%d%s", prefix, i, "/bmp"), (BITMAP *)o->dat);
            break;
         case DAT_FONT:
            sprintf(sub, "%s/%d/font", prefix, i);
            dump_font_metrics(g, sub, (FONT *)o->dat);
            break;
         case DAT_SAMPLE: {
            SAMPLE *s = (SAMPLE *)o->dat;
            golden_int(g, rec_name("%s/%d%s", prefix, i, "/bits"), s->bits);
            golden_int(g, rec_name("%s/%d%s", prefix, i, "/stereo"), s->stereo);
            golden_int(g, rec_name("%s/%d%s", prefix, i, "/freq"), s->freq);
            golden_int(g, rec_name("%s/%d%s", prefix, i, "/len"), (long)s->len);
            golden_int(g, rec_name("%s/%d%s", prefix, i, "/prio"), s->priority);
            golden_int(g, rec_name("%s/%d%s", prefix, i, "/loop"), (long)s->loop_end);
            golden_bytes(g, rec_name("%s/%d%s", prefix, i, "/data"), s->data,
                         (long)s->len * (s->bits == 8 ? 1 : 2) * (s->stereo ? 2 : 1));
            break;
         }
         case DAT_FILE:
            sprintf(sub, "%s/%d", prefix, i);
            dump_datafile(g, sub, (DATAFILE *)o->dat);
            break;
         case DAT_MIDI:
            break;
         default:
            golden_bytes(g, rec_name("%s/%d%s", prefix, i, "/raw"), o->dat, o->size);
            break;
      }
   }
   golden_int(g, rec_name("%s/%d%s", prefix, i, "/count"), i);
}

static long cb_log[4096];
static int cb_n;

static void dat_callback(DATAFILE *d)
{
   if (cb_n < 4094) {
      cb_log[cb_n++] = d->type;
      cb_log[cb_n++] = d->size;
   }
}

static void dump_cb_log(golden_ctx *g, const char *name)
{
   static unsigned char buf[4096 * 4];
   int i, n = 0;
   for (i = 0; i < cb_n; i++) {
      long v = cb_log[i];
      buf[n++] = (unsigned char)v; buf[n++] = (unsigned char)(v >> 8);
      buf[n++] = (unsigned char)(v >> 16); buf[n++] = (unsigned char)(v >> 24);
   }
   golden_bytes(g, name, buf, n);
   cb_n = 0;
}

static void load_and_dump(golden_ctx *g, const char *prefix, const char *rel, const char *pw,
                          int depth, int conv, int callback)
{
   DATAFILE *d;
   set_color_depth(depth);
   set_color_conversion(conv);
   packfile_password(pw);
   cb_n = 0;
   if (callback)
      d = load_datafile_callback(golden_path(g, rel), dat_callback);
   else
      d = load_datafile(golden_path(g, rel));
   packfile_password(NULL);
   if (callback)
      dump_cb_log(g, rec_name("%s/%d%s", prefix, 0, "/callbacks"));
   dump_datafile(g, prefix, d);
   unload_datafile(d);
}

static void scen_datafiles(golden_ctx *g)
{
   DATAFILE *pal;
   static const char *chars[] = {
      "characters/disco_dave/dave.dat", "characters/harold_the_homeboy/harold.dat",
      "characters/jungle_jane/jane.dat", "characters/wild_wendy/wendy.dat"
   };
   int i;

   /* exactly as the game: data.dat and sfx15.dat at 32 bpp with 0x00ffffff */
   load_and_dump(g, "dat/data", "data/data.dat", "CHEESE", 32, 0x00ffffff, 1);
   load_and_dump(g, "dat/loading", "data/loading.dat", "(c) Free Lunch Design", 32, COLORCONV_NONE, 1);
   load_and_dump(g, "dat/sfx", "data/sfx15.dat", "CHEESE", 32, 0x00ffffff, 1);
   for (i = 0; i < 4; i++) {
      char p[64];
      sprintf(p, "dat/char%d", i);
      load_and_dump(g, p, chars[i], NULL, 32, 0x00ffffff, 0);
   }

   /* wrong password / no password: must fail the same way */
   set_color_depth(32);
   packfile_password("WRONG");
   pal = load_datafile(golden_path(g, "data/loading.dat"));
   golden_int(g, "dat/wrongpw", pal != NULL);
   unload_datafile(pal);
   packfile_password(NULL);
   pal = load_datafile(golden_path(g, "data/data.dat"));
   golden_int(g, "dat/nopw", pal != NULL);
   unload_datafile(pal);
   golden_int(g, "dat/missing", load_datafile(golden_path(g, "data/nothere.dat")) != NULL);

   /* colour conversions: 8 bpp through the palette, other depths */
   packfile_password("(c) Free Lunch Design");
   set_color_depth(32);
   set_color_conversion(COLORCONV_NONE);
   pal = load_datafile(golden_path(g, "data/loading.dat"));
   packfile_password(NULL);
   if (pal) {
      set_palette((RGB *)pal[0].dat);
      load_and_dump(g, "conv/loading32", "data/loading.dat", "(c) Free Lunch Design", 32, COLORCONV_TOTAL, 0);
      load_and_dump(g, "conv/loading16", "data/loading.dat", "(c) Free Lunch Design", 16, COLORCONV_TOTAL, 0);
      load_and_dump(g, "conv/loading24k", "data/loading.dat", "(c) Free Lunch Design", 24,
                    COLORCONV_TOTAL | COLORCONV_KEEP_TRANS, 0);
      load_and_dump(g, "conv/harold8", chars[1], NULL, 8, COLORCONV_TOTAL, 0);
      load_and_dump(g, "conv/jane8k", chars[2], NULL, 8, COLORCONV_TOTAL | COLORCONV_KEEP_TRANS, 0);
      unload_datafile(pal);
   }
   load_and_dump(g, "conv/harold15", chars[1], NULL, 15, COLORCONV_TOTAL, 0);
   load_and_dump(g, "conv/harold24k", chars[1], NULL, 24, COLORCONV_TOTAL | COLORCONV_KEEP_TRANS, 0);
   load_and_dump(g, "conv/jane16", chars[2], NULL, 16, COLORCONV_TOTAL, 0);
   load_and_dump(g, "conv/jane16none", chars[2], NULL, 16, COLORCONV_NONE, 0);
   load_and_dump(g, "conv/wendy15k", chars[3], NULL, 15, COLORCONV_TOTAL | COLORCONV_KEEP_TRANS, 0);
   load_and_dump(g, "conv/wendy32", chars[3], NULL, 32, 0x00ffffff, 0);
   set_color_depth(32);
   set_color_conversion(0x00ffffff);

   /* datafile objects as special files */
   packfile_password("(c) Free Lunch Design");
   {
      int a = -12345;
      PACKFILE *f;
      unsigned char buf[64];
      const char *obj = golden_path(g, "data/loading.dat#FLD_LOGO");
      golden_int(g, "special/exists_all", file_exists(obj, FA_ALL, &a));
      golden_int(g, "special/aret", a);
      golden_int(g, "special/exists_arch", file_exists(obj, FA_ARCH, NULL));
      golden_int(g, "special/size", (long)file_size_ex(obj));
      f = pack_fopen(obj, F_READ);
      golden_int(g, "special/open", f != NULL);
      if (f) {
         memset(buf, 0, sizeof(buf));
         golden_int(g, "special/read", pack_fread(buf, sizeof(buf), f));
         golden_bytes(g, "special/bytes", buf, sizeof(buf));
         pack_fclose(f);
      }
      golden_int(g, "special/missing", pack_fopen(golden_path(g, "data/loading.dat#NOPE"), F_READ) != NULL);
      golden_int(g, "special/write", pack_fopen(obj, F_WRITE) != NULL);
   }
   packfile_password(NULL);
}

/* ------------------------------------------------------------------ */
/* packfiles                                                          */
/* ------------------------------------------------------------------ */

static unsigned char payload[20000];

static void make_payload(void)
{
   unsigned int s = 12345u;
   int i;
   for (i = 0; i < (int)sizeof(payload); i++) {
      s = s * 1103515245u + 12345u;
      if ((i / 700) & 1)
         payload[i] = (unsigned char)((s >> 16) & 0xFF);          /* noise */
      else
         payload[i] = (unsigned char)("Icy Tower!  "[i % 12] + ((i / 97) & 3)); /* repeats */
   }
}

static long file_bytes(const char *path, unsigned char *buf, long max)
{
   FILE *fp = fopen(path, "rb");
   long n;
   if (!fp)
      return -1;
   n = (long)fread(buf, 1, (size_t)max, fp);
   fclose(fp);
   return n;
}

static void write_variant(golden_ctx *g, const char *tag, const char *mode, const char *pw, long n)
{
   static unsigned char raw[40000], back[40000];
   char name[128];
   const char *path = tmp_path(tag);
   PACKFILE *f;
   long got;

   packfile_password(pw);
   f = pack_fopen(path, mode);
   sprintf(name, "pack/%s/open", tag);
   golden_int(g, name, f != NULL);
   if (!f) {
      packfile_password(NULL);
      return;
   }
   /* mixture of the writing helpers */
   pack_putc('A', f);
   pack_iputw(0x1234, f);
   pack_iputl(0x89ABCDEFL, f);
   pack_mputw(0x5678, f);
   pack_mputl(0x13579BDFL, f);
   pack_fwrite(payload, n, f);
   pack_mputl(-2L, f);
   sprintf(name, "pack/%s/close", tag);
   golden_int(g, name, pack_fclose(f));
   packfile_password(NULL);

   got = file_bytes(path, raw, sizeof(raw));
   sprintf(name, "pack/%s/file", tag);
   golden_bytes(g, name, raw, got);

   /* read back: packed modes with "rp", plain ones with "r" */
   packfile_password(pw);
   f = pack_fopen(path, strchr(mode, 'p') || strchr(mode, '!') ? F_READ_PACKED : F_READ);
   sprintf(name, "pack/%s/reopen", tag);
   golden_int(g, name, f != NULL);
   if (f) {
      sprintf(name, "pack/%s/getc", tag);
      golden_int(g, name, pack_getc(f));
      sprintf(name, "pack/%s/igetw", tag);
      golden_int(g, name, pack_igetw(f));
      sprintf(name, "pack/%s/igetl", tag);
      golden_int(g, name, pack_igetl(f));
      sprintf(name, "pack/%s/mgetw", tag);
      golden_int(g, name, pack_mgetw(f));
      sprintf(name, "pack/%s/mgetl", tag);
      golden_int(g, name, pack_mgetl(f));
      sprintf(name, "pack/%s/fseek", tag);
      golden_int(g, name, pack_fseek(f, 100));
      memset(back, 0, sizeof(back));
      got = pack_fread(back, sizeof(back), f);
      sprintf(name, "pack/%s/nread", tag);
      golden_int(g, name, got);
      sprintf(name, "pack/%s/back", tag);
      golden_bytes(g, name, back, got);
      sprintf(name, "pack/%s/eof", tag);
      golden_int(g, name, pack_feof(f));
      sprintf(name, "pack/%s/err", tag);
      golden_int(g, name, pack_ferror(f));
      sprintf(name, "pack/%s/getc_eof", tag);
      golden_int(g, name, pack_getc(f));
      pack_fclose(f);
   }
   packfile_password(NULL);
}

static char itr_names[16][1024];
static int itr_n;

static int itr_cb(const char *filename, int attrib, void *param)
{
   (void)attrib; (void)param;
   if (itr_n < 16)
      strcpy(itr_names[itr_n++], filename);
   return 0;
}

static void scen_packfiles(golden_ctx *g)
{
   PACKFILE *f;
   const char *path;
   int i;

   make_payload();
   write_variant(g, "wp", F_WRITE_PACKED, NULL, sizeof(payload));
   write_variant(g, "wp_pw", F_WRITE_PACKED, "CHEESE", sizeof(payload));
   write_variant(g, "wp_small", F_WRITE_PACKED, NULL, 5);
   write_variant(g, "wp_4095", F_WRITE_PACKED, "x", 4095 - 13);
   write_variant(g, "wnopack", F_WRITE_NOPACK, NULL, 3000);
   write_variant(g, "wnopack_pw", F_WRITE_NOPACK, "(c) Free Lunch Design", 9000);
   write_variant(g, "w_pw", F_WRITE, "CHEESE", 5000);
   write_variant(g, "wb", "wb", NULL, 5000);

   /* reading plain files in packed mode fails with EDOM */
   path = tmp_path("w_pw");
   *allegro_errno = 0;
   f = pack_fopen(path, F_READ_PACKED);
   golden_int(g, "pack/rp_plain", f != NULL);
   put_errno(g, "pack/rp_plain_errno");
   if (f) pack_fclose(f);
   *allegro_errno = 0;
   f = pack_fopen(golden_path(g, "data/nothere.bin"), F_READ);
   golden_int(g, "pack/missing", f != NULL);
   put_errno(g, "pack/missing_errno");

   /* packed file read with the wrong password */
   packfile_password("CHEESF");
   f = pack_fopen(tmp_path("wp_pw"), F_READ_PACKED);
   golden_int(g, "pack/wrongpw", f != NULL);
   if (f) pack_fclose(f);
   packfile_password(NULL);

   /* replays are read with "rb" (every replay in the asset directory) */
   itr_n = 0;
   for_each_file_ex(golden_path(g, "replays/*.itr"), 0, FA_DIREC, itr_cb, NULL);
   golden_int(g, "pack/itr_count", itr_n);
   for (i = 0; i < itr_n; i++) {
      char name[64];
      long n;
      f = pack_fopen(itr_names[i], "rb");
      sprintf(name, "pack/itr%d/open", i);
      golden_int(g, name, f != NULL);
      if (f) {
         static unsigned char ibuf[200000];
         n = pack_fread(ibuf, sizeof(ibuf), f);
         sprintf(name, "pack/itr%d/bytes", i);
         golden_bytes(g, name, ibuf, n);
         sprintf(name, "pack/itr%d/eof", i);
         golden_int(g, name, pack_feof(f));
         pack_fclose(f);
      }
   }

   /* the game's tower.cfg ("rp" without password), when provided */
   if (getenv("A4_GOLDEN_TOWER_CFG")) {
      static unsigned char cfg[65536];
      long n;
      packfile_password(NULL);
      f = pack_fopen(getenv("A4_GOLDEN_TOWER_CFG"), F_READ_PACKED);
      golden_int(g, "pack/towercfg/open", f != NULL);
      if (f) {
         n = pack_fread(cfg, sizeof(cfg), f);
         golden_bytes(g, "pack/towercfg/data", cfg, n);
         pack_fclose(f);
      }
   }

   /* data.dat read raw in packed mode */
   packfile_password("CHEESE");
   f = pack_fopen(golden_path(g, "data/data.dat"), F_READ_PACKED);
   golden_int(g, "pack/datadat/open", f != NULL);
   if (f) {
      static unsigned char big[70000];
      long n;
      golden_int(g, "pack/datadat/magic", pack_mgetl(f));
      golden_int(g, "pack/datadat/count", pack_mgetl(f));
      golden_int(g, "pack/datadat/seek", pack_fseek(f, 5000));
      n = pack_fread(big, sizeof(big), f);
      golden_int(g, "pack/datadat/n", n);
      golden_bytes(g, "pack/datadat/bytes", big, n);
      pack_fclose(f);
   }
   packfile_password(NULL);

   for (i = 0; i < 8; i++) {
      static const char *tags[] = { "wp", "wp_pw", "wp_small", "wp_4095", "wnopack", "wnopack_pw", "w_pw", "wb" };
      delete_file(tmp_path(tags[i]));
   }
}

/* ------------------------------------------------------------------ */
/* file utilities                                                     */
/* ------------------------------------------------------------------ */

static const char *path_samples[] = {
   "", "file", "file.txt", "dir/file.txt", "dir\\sub\\file.tar.gz", "c:file.dat",
   "C:\\games\\icytower\\icytower15.exe", "profiles//name/", "noext.", ".hidden",
   "a.b/c", "dir.d/", "weird\xA4.p\xA4ng", "utf\xC3\xA9/\xC3\xA9t\xC3\xA9.png",
   "bad\xC3(seq.tx\xFF", "replays/best 1.itr", "x:", "/abs/path.PNG", "..", "a/./b.c"
};

static void scen_paths(golden_ctx *g)
{
   char buf[1024], name[64];
   int i, n = (int)(sizeof(path_samples) / sizeof(path_samples[0]));
   for (i = 0; i < n; i++) {
      const char *s = path_samples[i];
      sprintf(name, "path/%d/filename", i);
      golden_int(g, name, (long)(get_filename(s) - s));
      sprintf(name, "path/%d/extension", i);
      golden_int(g, name, (long)(get_extension(s) - s));
      replace_extension(buf, s, "itr", sizeof(buf));
      sprintf(name, "path/%d/repext", i);
      golden_bytes(g, name, buf, (long)strlen(buf));
      replace_extension(buf, s, "", sizeof(buf));
      sprintf(name, "path/%d/repext0", i);
      golden_bytes(g, name, buf, (long)strlen(buf));
      replace_filename(buf, s, "new.dat", sizeof(buf));
      sprintf(name, "path/%d/repfile", i);
      golden_bytes(g, name, buf, (long)strlen(buf));
      replace_filename(buf, s, "", sizeof(buf));
      sprintf(name, "path/%d/repfile0", i);
      golden_bytes(g, name, buf, (long)strlen(buf));
      replace_filename(buf, s, "0123456789", 8);
      sprintf(name, "path/%d/repfile_small", i);
      golden_bytes(g, name, buf, (long)strlen(buf));
   }
}

static char enum_log[65536];
static int enum_len, enum_stop_after, enum_calls;

static int enum_cb(const char *filename, int attrib, void *param)
{
   int n = (int)strlen(filename);
   (void)param;
   if (enum_len + n + 16 < (int)sizeof(enum_log)) {
      memcpy(enum_log + enum_len, filename, (size_t)n);
      enum_len += n;
      enum_len += sprintf(enum_log + enum_len, "|%x\n", attrib);
   }
   enum_calls++;
   if (enum_stop_after && enum_calls >= enum_stop_after)
      return 1;
   return 0;
}

static void enum_case(golden_ctx *g, const char *tag, const char *pattern, int in_a, int out_a, int stop)
{
   char name[128];
   int r;
   enum_len = 0;
   enum_calls = 0;
   enum_stop_after = stop;
   r = for_each_file_ex(pattern, in_a, out_a, enum_cb, NULL);
   sprintf(name, "enum/%s/ret", tag);
   golden_int(g, name, r);
   sprintf(name, "enum/%s/log", tag);
   golden_bytes(g, name, enum_log, enum_len);
}

static void exists_case(golden_ctx *g, const char *tag, const char *rel, int attrib)
{
   char name[128];
   int a = -7;
   const char *p = golden_path(g, rel);
   sprintf(name, "exists/%s/r", tag);
   golden_int(g, name, file_exists(p, attrib, &a));
   sprintf(name, "exists/%s/a", tag);
   golden_int(g, name, a);
}

static void scen_files(golden_ctx *g)
{
   FILE *fp;
   const char *tmp;

   exists_case(g, "datadat_std", "data/data.dat", FA_ARCH | FA_RDONLY);
   exists_case(g, "datadat_all", "data/data.dat", FA_ALL);
   exists_case(g, "datadat_none", "data/data.dat", 0);
   exists_case(g, "datadir_std", "data", FA_ARCH | FA_RDONLY);
   exists_case(g, "datadir_direc", "data", FA_DIREC);
   exists_case(g, "datadir_all", "data", FA_ALL);
   exists_case(g, "datadir_m1", "data", -1);
   exists_case(g, "datadir_slash", "data/", FA_DIREC);
   exists_case(g, "chardir_slash", "characters/disco_dave/", FA_DIREC);
   exists_case(g, "chardir", "characters/disco_dave", 16);
   exists_case(g, "chartxt", "characters/disco_dave/disco_dave.txt", FA_ARCH | FA_RDONLY);
   exists_case(g, "chartxt_case", "CHARACTERS/Disco_Dave/DISCO_DAVE.TXT", FA_ARCH | FA_RDONLY);
   exists_case(g, "missing", "data/missing.dat", FA_ALL);
   exists_case(g, "wild_dat", "data/*.dat", FA_ARCH | FA_RDONLY);
   exists_case(g, "wild_q", "data/data.da?", FA_ALL);
   exists_case(g, "wild_none", "data/*.xyz", FA_ALL);
   exists_case(g, "wild_dir", "char*", FA_DIREC);
   exists_case(g, "wild_dir_std", "char*", FA_ARCH | FA_RDONLY);
   golden_int(g, "exists/plain", exists(golden_path(g, "data/sfx15.dat")));
   golden_int(g, "exists/dir", exists(golden_path(g, "characters")));

   golden_int(g, "size/datadat", (long)file_size_ex(golden_path(g, "data/data.dat")));
   golden_int(g, "size/loading", (long)file_size_ex(golden_path(g, "data/loading.dat")));
   golden_int(g, "size/missing", (long)file_size_ex(golden_path(g, "data/missing.dat")));
   golden_int(g, "size/dir", (long)file_size_ex(golden_path(g, "data")));

   enum_case(g, "chars_dirs", golden_path(g, "characters/*"), FA_DIREC, 0, 0);
   enum_case(g, "chars_all", golden_path(g, "characters/*"), 0, 0, 0);
   enum_case(g, "chars_starstar", golden_path(g, "characters/*.*"), 0, 0, 0);
   enum_case(g, "chars_nodirs", golden_path(g, "characters/*"), 0, FA_DIREC, 0);
   enum_case(g, "chars_stop2", golden_path(g, "characters/*"), FA_DIREC, 0, 2);
   enum_case(g, "data_dat", golden_path(g, "data/*.dat"), 0, 0, 0);
   enum_case(g, "data_DAT", golden_path(g, "data/*.DAT"), 0, 0, 0);
   enum_case(g, "data_q", golden_path(g, "data/????.dat"), 0, 0, 0);
   enum_case(g, "data_arch", golden_path(g, "data/*"), FA_ARCH, FA_DIREC, 0);
   enum_case(g, "replays", golden_path(g, "replays/*"), 0, 0, 0);
   enum_case(g, "replays_itr", golden_path(g, "replays/*.itr"), 0, 0, 0);
   enum_case(g, "missing", golden_path(g, "nothere/*"), 0, 0, 0);
   enum_case(g, "dave_txt", golden_path(g, "characters/disco_dave/*.txt"), 0, 0, 0);
   enum_case(g, "exact", golden_path(g, "data/data.dat"), 0, 0, 0);

   /* delete_file */
   tmp = tmp_path("delete.bin");
   fp = fopen(tmp, "wb");
   if (fp) {
      fputs("x", fp);
      fclose(fp);
   }
   golden_int(g, "delete/before", exists(tmp));
   golden_int(g, "delete/ret", delete_file(tmp));
   golden_int(g, "delete/after", exists(tmp));
   golden_int(g, "delete/again", delete_file(tmp));
}

/* ------------------------------------------------------------------ */
/* config                                                             */
/* ------------------------------------------------------------------ */

static const char ini_text[] =
   "# gamepad configuration\r\n"
   "up = jump\r\n"
   "down=down\n"
   "   left   =   left   \n"
   "right\t=\tright # not a comment\n"
   "empty =\n"
   "novalue\n"
   "num = 42\n"
   "hex = 0x1F\n"
   "neg = -17\n"
   "oct = 017\n"
   "junk = 12abc\n"
   "huge = 99999999999\n"
   "UP = duplicate\n"
   "=orphan\n"
   "euro\xA4key = \xA4value\xA4\n"
   "utf = \xC3\xA9t\xC3\xA9\n"
   "bad = \xC3(x\n"
   "\n"
   "[section]\n"
   "up = section-up\n"
   "num = 7\n"
   "[Other Section]\n"
   "a = b\n"
   "[#hash]\n"
   "h = hidden\n"
   "[ spaced ]\n"
   "k = v\r"
   "last = no newline";

static void cfg_get(golden_ctx *g, const char *tag, const char *section, const char *kname)
{
   char name[128];
   const char *s = get_config_string(section, kname, "<def>");
   sprintf(name, "cfg/%s/s", tag);
   golden_bytes(g, name, s, (long)strlen(s));
   sprintf(name, "cfg/%s/i", tag);
   golden_int(g, name, get_config_int(section, kname, -999));
}

static void scen_config(golden_ctx *g)
{
   FILE *fp;
   const char *path = tmp_path("gamepad.txt");

   cfg_get(g, "noconfig", NULL, "up");

   fp = fopen(path, "wb");
   if (fp) {
      fwrite(ini_text, 1, sizeof(ini_text) - 1, fp);
      fclose(fp);
   }
   packfile_password(NULL);
   set_config_file(path);

   cfg_get(g, "up", NULL, "up");
   cfg_get(g, "UP", NULL, "UP");
   cfg_get(g, "down", NULL, "down");
   cfg_get(g, "left", NULL, "left");
   cfg_get(g, "right", NULL, "right");
   cfg_get(g, "empty", NULL, "empty");
   cfg_get(g, "novalue", NULL, "novalue");
   cfg_get(g, "num", NULL, "num");
   cfg_get(g, "hex", NULL, "hex");
   cfg_get(g, "neg", NULL, "neg");
   cfg_get(g, "oct", NULL, "oct");
   cfg_get(g, "junk", NULL, "junk");
   cfg_get(g, "huge", NULL, "huge");
   cfg_get(g, "euro", NULL, "euro\xA4key");
   cfg_get(g, "euro2", NULL, "euro$key");
   cfg_get(g, "utf", NULL, "utf");
   cfg_get(g, "bad", NULL, "bad");
   cfg_get(g, "missing", NULL, "nothing");
   cfg_get(g, "emptysec_up", "", "up");
   cfg_get(g, "sec_up", "section", "up");
   cfg_get(g, "sec_up2", "[section]", "up");
   cfg_get(g, "sec_up3", "SECTION", "up");
   cfg_get(g, "sec_num", "section", "num");
   cfg_get(g, "sec_down", "section", "down");
   cfg_get(g, "other_a", "Other Section", "a");
   cfg_get(g, "hash_h", "#hash", "h");
   cfg_get(g, "hash_h2", NULL, "h");
   cfg_get(g, "spaced_k", " spaced ", "k");
   cfg_get(g, "spaced_k2", "spaced", "k");
   cfg_get(g, "last", " spaced ", "last");
   cfg_get(g, "secname", NULL, "[section]");
   cfg_get(g, "orphan", NULL, "");

   /* the file is read through the packfile layer: the password applies */
   packfile_password("CHEESE");
   set_config_file(path);
   packfile_password(NULL);
   cfg_get(g, "pw_up", NULL, "up");

   /* missing file: empty config */
   set_config_file(tmp_path("does-not-exist.cfg"));
   cfg_get(g, "gone_up", NULL, "up");

   delete_file(path);
}

/* ------------------------------------------------------------------ */
/* synthetic datafiles: every object reader, nesting, packed chunks,  */
/* old V1 format                                                      */
/* ------------------------------------------------------------------ */

typedef struct BUF {
   unsigned char *p;
   long n, cap;
} BUF;

static void bput(BUF *b, const void *d, long n)
{
   if (b->n + n > b->cap) {
      b->cap = (b->n + n) * 2 + 256;
      b->p = (unsigned char *)realloc(b->p, (size_t)b->cap);
   }
   memcpy(b->p + b->n, d, (size_t)n);
   b->n += n;
}

static void bbyte(BUF *b, int c) { unsigned char x = (unsigned char)c; bput(b, &x, 1); }
static void bm16(BUF *b, int v) { bbyte(b, v >> 8); bbyte(b, v); }
static void bi16(BUF *b, int v) { bbyte(b, v); bbyte(b, v >> 8); }
static void bm32(BUF *b, long v) { bbyte(b, (int)(v >> 24)); bbyte(b, (int)(v >> 16)); bbyte(b, (int)(v >> 8)); bbyte(b, (int)v); }
static void bfree(BUF *b) { free(b->p); b->p = NULL; b->n = b->cap = 0; }

static void bprop(BUF *b, const char *id, const char *s)
{
   bm32(b, DAT_PROPERTY);
   bm32(b, DAT_ID(id[0], id[1], id[2], id[3]));
   bm32(b, (long)strlen(s));
   bput(b, s, (long)strlen(s));
}

static void bobj(BUF *b, int type, BUF *body)
{
   bm32(b, type);
   bm32(b, body->n);
   bm32(b, body->n);
   bput(b, body->p, body->n);
}

/* a packed chunk: the LZSS stream of a "wp" file (without its magic) */
static void bobj_packed(BUF *b, int type, BUF *body)
{
   static unsigned char raw[200000];
   const char *path = tmp_path("chunk.lz");
   PACKFILE *f;
   long n;
   packfile_password(NULL);
   f = pack_fopen(path, F_WRITE_PACKED);
   if (!f)
      return;
   pack_fwrite(body->p, body->n, f);
   pack_fclose(f);
   n = file_bytes(path, raw, sizeof(raw));
   delete_file(path);
   bm32(b, type);
   bm32(b, n - 4);
   bm32(b, -body->n);
   bput(b, raw + 4, n - 4);
}

static int pix(int x, int y, int k) { return (x * 37 + y * 91 + k * 53 + ((x ^ y) & 7) * 11) & 0xFF; }

static void bbitmap(BUF *b, int bits, int w, int h)
{
   int x, y, rgba = bits < 0, ab = rgba ? -bits : bits;
   bm16(b, bits);
   bm16(b, w);
   bm16(b, h);
   for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++) {
         int magenta = ((x + y) % 7) == 0;
         switch (ab) {
            case 8:
               bbyte(b, (x + y) % 5 == 0 ? 0 : pix(x, y, 0));
               break;
            case 15:
            case 16:
               bi16(b, magenta ? 0xF81F : (pix(x, y, 1) << 8) | pix(x, y, 2));
               break;
            default:
               bbyte(b, magenta ? 255 : pix(x, y, 3));
               bbyte(b, magenta ? 0 : pix(x, y, 4));
               bbyte(b, magenta ? 255 : pix(x, y, 5));
               if (rgba)
                  bbyte(b, pix(x, y, 6));
               break;
         }
      }
   }
}

static void bmono_glyph(BUF *b, int w, int h, int k)
{
   int i, n = ((w + 7) / 8) * h;
   bm16(b, w);
   bm16(b, h);
   for (i = 0; i < n; i++)
      bbyte(b, (i * 29 + k * 71) ^ 0x5A);
}

static void build_objects(BUF *d, int with_nested, int pack_some)
{
   BUF o = { NULL, 0, 0 };
   /* (24 bpp with alpha is exercised separately: it fails at most depths) */
   int i, depths[8] = { 8, 15, 16, 24, 32, -32, 16, 8 };

   /* bitmaps of every depth */
   for (i = 0; i < 8; i++) {
      char nm[16];
      sprintf(nm, "BMP%d", i);
      bprop(d, "NAME", nm);
      bbitmap(&o, depths[i], depths[i] == 4 ? 16 : 13, depths[i] == 4 ? 2 : 9);
      if (pack_some && (i & 1))
         bobj_packed(d, DAT_BITMAP, &o);
      else
         bobj(d, DAT_BITMAP, &o);
      bfree(&o);
   }

   /* old fixed font (height > 0): 224 glyphs of 8 x height */
   bm16(&o, 7);
   for (i = 0; i < 224 * 7; i++)
      bbyte(&o, (i * 13) ^ (i >> 3));
   bprop(d, "NAME", "FIXEDFONT");
   bobj(d, DAT_FONT, &o);
   bfree(&o);

   /* old proportional font (height < 0): a few 8-bit bitmaps, rest blank */
   bm16(&o, -1);
   for (i = 0; i < 5; i++) {
      int x, y, w = 3 + i, h = 6 + (i & 1);
      bm16(&o, w);
      bm16(&o, h);
      for (y = 0; y < h; y++)
         for (x = 0; x < w; x++)
            bbyte(&o, ((x + y + i) % 3) ? pix(x, y, i) : 0);
   }
   bobj(d, DAT_FONT, &o);
   bfree(&o);

   /* 4.x mono font, two ranges, glyphs wider than 8 */
   bm16(&o, 0);
   bm16(&o, 2);
   bbyte(&o, 1);
   bm32(&o, 0x20);
   bm32(&o, 0x5A);
   for (i = 0x20; i <= 0x5A; i++)
      bmono_glyph(&o, 5 + (i % 9), 7 + (i % 3), i);
   bbyte(&o, 255);
   bm32(&o, 0x100);
   bm32(&o, 0x104);
   for (i = 0x100; i <= 0x104; i++)
      bmono_glyph(&o, 11, 12, i);
   bprop(d, "NAME", "MONORANGES");
   bobj(d, DAT_FONT, &o);
   bfree(&o);

   /* 4.x colour font: an 8-bit range (depth 0) and a 16-bit range */
   bm16(&o, 0);
   bm16(&o, 2);
   bbyte(&o, 0);
   bm32(&o, 0x20);
   bm32(&o, 0x5E);
   for (i = 0x20; i <= 0x5E; i++) {
      int x, y, w = 4 + (i % 6), h = 9 + (i % 2);
      bm16(&o, w);
      bm16(&o, h);
      for (y = 0; y < h; y++)
         for (x = 0; x < w; x++)
            bbyte(&o, ((x * y + i) % 4) ? pix(x, y, i) : 0);
   }
   bbyte(&o, 16);
   bm32(&o, 0x100);
   bm32(&o, 0x101);
   for (i = 0; i < 2; i++) {
      int x, y;
      bm16(&o, 6);
      bm16(&o, 11);
      for (y = 0; y < 11; y++)
         for (x = 0; x < 6; x++)
            bi16(&o, ((x + y) % 3) ? (pix(x, y, 9) << 8) | pix(x, y, 10) : 0xF81F);
   }
   bprop(d, "NAME", "COLORRANGES");
   if (pack_some)
      bobj_packed(d, DAT_FONT, &o);
   else
      bobj(d, DAT_FONT, &o);
   bfree(&o);

   /* samples: 8-bit mono and 16-bit stereo */
   bm16(&o, 8);
   bm16(&o, 11025);
   bm32(&o, 300);
   for (i = 0; i < 300; i++)
      bbyte(&o, pix(i, 1, 2));
   bobj(d, DAT_SAMPLE, &o);
   bfree(&o);
   bm16(&o, -16);
   bm16(&o, 22050);
   bm32(&o, 100);
   for (i = 0; i < 200; i++)
      bi16(&o, (pix(i, 3, 4) << 8) | pix(i, 5, 6));
   bobj(d, DAT_SAMPLE, &o);
   bfree(&o);

   /* MIDI: divisions + 32 tracks */
   bm16(&o, 96);
   for (i = 0; i < 32; i++) {
      bm32(&o, i == 3 ? 5 : 0);
      if (i == 3)
         bput(&o, "\x00\x90\x3C\x40\x00", 5);
   }
   bobj(d, DAT_MIDI, &o);
   bfree(&o);

   /* palette and unknown types as raw data */
   for (i = 0; i < 256 * 4; i++)
      bbyte(&o, (i & 3) == 3 ? 0 : (i * 7) & 63);
   bprop(d, "NAME", "PAL");
   bobj(d, DAT_PALETTE, &o);
   bfree(&o);
   bput(&o, "OggS fake payload", 17);
   bprop(d, "NAME", "UNKNOWN");
   bprop(d, "ORIG", "c:\\x\\y.ogg");
   bprop(d, "DATE", "");
   bobj(d, DAT_ID('O', 'G', 'G', ' '), &o);
   bfree(&o);
   bobj(d, DAT_DATA, &o);   /* empty object */

   if (with_nested) {
      BUF n = { NULL, 0, 0 };
      bm32(&n, 3);
      bprop(&n, "NAME", "INNER_A");
      bput(&o, "inner", 5);
      bobj(&n, DAT_DATA, &o);
      bfree(&o);
      bbitmap(&o, 24, 5, 4);
      bobj_packed(&n, DAT_BITMAP, &o);
      bfree(&o);
      bm32(&o, 1);
      bput(&o, "deep", 4);
      {
         BUF deep = { NULL, 0, 0 };
         bm32(&deep, 1);
         bprop(&deep, "NAME", "DEEPEST");
         bobj(&deep, DAT_DATA, &o);
         bfree(&o);
         bprop(&n, "NAME", "DEEP");
         bobj(&n, DAT_FILE, &deep);
         bfree(&deep);
      }
      bprop(d, "NAME", "NESTED");
      bobj(d, DAT_FILE, &n);
      bfree(&n);
   }
   bprop(d, "NAME", "DANGLING");   /* property without an object: dropped */
}

static int count_objects(int with_nested)
{
   return 8 + 4 + 2 + 1 + 3 + (with_nested ? 1 : 0);
}

static void render_font_sample(golden_ctx *g, const char *name, FONT *f)
{
   static const int depths[2] = { 32, 16 };
   char full[256];
   int i;
   for (i = 0; i < 2; i++) {
      BITMAP *b = create_bitmap_ex(depths[i], 160, 3 * (text_height(f) + 1) + 2);
      clear_to_color(b, makecol_depth(depths[i], 30, 60, 90));
      textout_ex(b, f, "AZ !\"#09 \xC4\x80\xC4\x81\xC4\x84 ~", -2, 0, -1, -1);
      textout_ex(b, f, "Hello \xC4\x81", 3, text_height(f) + 1, makecol_depth(depths[i], 250, 250, 0),
                 makecol_depth(depths[i], 90, 0, 0));
      textout_right_ex(b, f, "[mono]\xC4\x82", 163, 2 * (text_height(f) + 1), -1,
                       makecol_depth(depths[i], 0, 80, 0));
      sprintf(full, "%s/render%d", name, depths[i]);
      golden_bitmap(g, full, b);
      destroy_bitmap(b);
   }
}

static void synth_dump(golden_ctx *g, const char *prefix, DATAFILE *d)
{
   int i;
   char name[128];
   dump_datafile(g, prefix, d);
   if (!d)
      return;
   for (i = 0; d[i].type != DAT_END; i++) {
      if (d[i].type == DAT_FONT) {
         sprintf(name, "%s/%d", prefix, i);
         render_font_sample(g, name, (FONT *)d[i].dat);
      }
   }
}

static void scen_synthetic(golden_ctx *g)
{
   BUF d = { NULL, 0, 0 };
   BUF v1 = { NULL, 0, 0 };
   PACKFILE *f;
   DATAFILE *dat;
   int i;

   set_palette(desktop_palette);

   /* 1: whole file LZSS packed with a password, nested file, packed chunks */
   bm32(&d, DAT_MAGIC);
   bm32(&d, count_objects(1));
   build_objects(&d, 1, 1);
   packfile_password("S3cret \xC3\xA9");
   f = pack_fopen(tmp_path("synth1.dat"), F_WRITE_PACKED);
   if (f) {
      pack_fwrite(d.p, d.n, f);
      pack_fclose(f);
   }
   set_color_depth(32);
   set_color_conversion(0x00ffffff);
   cb_n = 0;
   dat = load_datafile_callback(tmp_path("synth1.dat"), dat_callback);
   dump_cb_log(g, "synth1/callbacks");
   synth_dump(g, "synth1", dat);
   unload_datafile(dat);

   /* the same at 16 bpp with COLORCONV_KEEP_TRANS */
   set_color_depth(16);
   set_color_conversion(COLORCONV_TOTAL | COLORCONV_KEEP_TRANS);
   dat = load_datafile(tmp_path("synth1.dat"));
   synth_dump(g, "synth1_16k", dat);
   unload_datafile(dat);

   /* the same at 24 bpp without conversion */
   set_color_depth(24);
   set_color_conversion(COLORCONV_NONE);
   dat = load_datafile(tmp_path("synth1.dat"));
   synth_dump(g, "synth1_24n", dat);
   unload_datafile(dat);
   packfile_password(NULL);
   bfree(&d);

   /* 2: "w!" (slh. + plain) without nesting, 8 bpp target */
   bm32(&d, DAT_MAGIC);
   bm32(&d, count_objects(0));
   build_objects(&d, 0, 0);
   f = pack_fopen(tmp_path("synth2.dat"), F_WRITE_NOPACK);
   if (f) {
      pack_fwrite(d.p, d.n, f);
      pack_fclose(f);
   }
   set_color_depth(8);
   set_color_conversion(COLORCONV_TOTAL);
   dat = load_datafile(tmp_path("synth2.dat"));
   synth_dump(g, "synth2_8", dat);
   unload_datafile(dat);
   bfree(&d);

   /* 3: truncated datafile (count larger than the content) */
   bm32(&d, DAT_MAGIC);
   bm32(&d, count_objects(0) + 2);
   build_objects(&d, 0, 0);
   f = pack_fopen(tmp_path("synth3.dat"), F_WRITE_PACKED);
   if (f) {
      pack_fwrite(d.p, d.n / 2, f);
      pack_fclose(f);
   }
   set_color_depth(32);
   set_color_conversion(0x00ffffff);
   dat = load_datafile(tmp_path("synth3.dat"));
   golden_int(g, "synth3/loaded", dat != NULL);
   unload_datafile(dat);
   bfree(&d);

   /* 4: old (V1) datafile format */
   bm32(&v1, 0x616C6C2EL);
   bm16(&v1, 6);
   bm16(&v1, 4);                      /* V1_DAT_SPRITE_16: Atari ST planes */
   bm16(&v1, 0);
   bm16(&v1, 16);
   bm16(&v1, 2);
   for (i = 0; i < 16; i++)
      bbyte(&v1, i * 37 + 5);
   bm16(&v1, 8);                      /* V1_DAT_FONT_8x8 */
   for (i = 0; i < 95 * 8; i++)
      bbyte(&v1, (i * 5) ^ 0x33);
   bm16(&v1, 3);                      /* V1_DAT_BITMAP_256 */
   bm16(&v1, 7);
   bm16(&v1, 3);
   for (i = 0; i < 21; i++)
      bbyte(&v1, i * 11);
   bm16(&v1, 6);                      /* V1_DAT_PALETTE_16 */
   for (i = 0; i < 48; i++)
      bbyte(&v1, i * 5);
   bm16(&v1, 12);                     /* V1_DAT_SAMPLE */
   bm16(&v1, 8);
   bm16(&v1, 8000);
   bm32(&v1, 10);
   bput(&v1, "0123456789", 10);
   bm16(&v1, 0);                      /* V1_DAT_DATA */
   bm32(&v1, 6);
   bput(&v1, "v1data", 6);
   f = pack_fopen(tmp_path("synth4.dat"), F_WRITE_PACKED);
   if (f) {
      pack_fwrite(v1.p, v1.n, f);
      pack_fclose(f);
   }
   cb_n = 0;
   dat = load_datafile_callback(tmp_path("synth4.dat"), dat_callback);
   dump_cb_log(g, "synth4/callbacks");
   synth_dump(g, "synth4", dat);
   unload_datafile(dat);
   bfree(&v1);

   /* 5: a 24 bpp bitmap with alpha: _color_load_depth() has no rule for
    * it unless the colour depth is 24 */
   bm32(&d, DAT_MAGIC);
   bm32(&d, 2);
   bbitmap(&v1, -24, 6, 5);
   bobj(&d, DAT_BITMAP, &v1);
   bfree(&v1);
   bput(&v1, "after", 5);
   bobj(&d, DAT_DATA, &v1);
   bfree(&v1);
   f = pack_fopen(tmp_path("synth5.dat"), F_WRITE_PACKED);
   if (f) {
      pack_fwrite(d.p, d.n, f);
      pack_fclose(f);
   }
   bfree(&d);
   for (i = 0; i < 4; i++) {
      static const int dd[4] = { 32, 24, 16, 8 };
      char name[64];
      set_color_depth(dd[i]);
      set_color_conversion(COLORCONV_TOTAL);
      dat = load_datafile(tmp_path("synth5.dat"));
      sprintf(name, "synth5_%d", dd[i]);
      synth_dump(g, name, dat);
      unload_datafile(dat);
   }
   delete_file(tmp_path("synth5.dat"));
   set_color_depth(32);

   /* nested objects through file#object names */
   packfile_password("S3cret \xC3\xA9");
   {
      char obj[1200];
      unsigned char buf[64];
      long n;
      sprintf(obj, "%s#NESTED/DEEP/DEEPEST", tmp_path("synth1.dat"));
      golden_int(g, "synth1/special_size", (long)file_size_ex(obj));
      f = pack_fopen(obj, F_READ);
      golden_int(g, "synth1/special_open", f != NULL);
      if (f) {
         n = pack_fread(buf, sizeof(buf), f);
         golden_bytes(g, "synth1/special_bytes", buf, n);
         pack_fclose(f);
      }
      sprintf(obj, "%s#bmp3", tmp_path("synth1.dat"));
      golden_int(g, "synth1/special_case", (long)file_size_ex(obj));
   }
   packfile_password(NULL);

   delete_file(tmp_path("synth1.dat"));
   delete_file(tmp_path("synth2.dat"));
   delete_file(tmp_path("synth3.dat"));
   delete_file(tmp_path("synth4.dat"));
   set_color_depth(32);
   set_color_conversion(0x00ffffff);
}

/* ------------------------------------------------------------------ */
/* image files: built-in BMP and PCX                                  */
/* ------------------------------------------------------------------ */

static void bi32(BUF *b, long v) { bbyte(b, (int)v); bbyte(b, (int)(v >> 8)); bbyte(b, (int)(v >> 16)); bbyte(b, (int)(v >> 24)); }

static void write_buf(const char *path, BUF *b)
{
   FILE *fp = fopen(path, "wb");
   if (fp) {
      fwrite(b->p, 1, (size_t)b->n, fp);
      fclose(fp);
   }
}

/* BITMAPFILEHEADER + BITMAPINFOHEADER (40 bytes) */
static void bmp_header(BUF *b, long w, long h, int bits, long compression, long off_bits)
{
   bi16(b, 0x4D42);
   bi32(b, 0);
   bi16(b, 0);
   bi16(b, 0);
   bi32(b, off_bits);
   bi32(b, 40);
   bi32(b, w);
   bi32(b, h);
   bi16(b, 1);
   bi16(b, bits);
   bi32(b, compression);
   bi32(b, 0);
   bi32(b, 0);
   bi32(b, 0);
   bi32(b, 0);
   bi32(b, 0);
}

static void bmp_palette(BUF *b, int n)
{
   int i;
   for (i = 0; i < n; i++) {
      bbyte(b, (i * 40) & 0xFF);
      bbyte(b, (i * 90 + 7) & 0xFF);
      bbyte(b, (i * 13 + 100) & 0xFF);
      bbyte(b, 0);
   }
}

static void load_variants(golden_ctx *g, const char *tag, const char *path, int truecolor)
{
   static const int depths[4] = { 32, 8, 16, 24 };
   static const int convs[4] = { 0x00ffffff, COLORCONV_TOTAL, COLORCONV_NONE, COLORCONV_TOTAL | COLORCONV_KEEP_TRANS };
   char name[128];
   int i;
   for (i = 0; i < 4; i++) {
      PALETTE pal;
      BITMAP *b;
      /* truecolor -> 8 bpp uses Allegro's optimized palette / RGB map,
       * which the compat layer does not reproduce (documented) */
      if (truecolor && depths[i] == 8)
         continue;
      memset(pal, 0, sizeof(pal));
      set_color_depth(depths[i]);
      set_color_conversion(convs[i]);
      b = load_bitmap(path, i == 3 ? NULL : pal);
      sprintf(name, "img/%s/load%d", tag, i);
      golden_bitmap(g, name, b);
      if (i != 3) {
         sprintf(name, "img/%s/pal%d", tag, i);
         golden_bytes(g, name, pal, sizeof(pal));
      }
      if (b)
         destroy_bitmap(b);
   }
   set_color_depth(32);
   set_color_conversion(0x00ffffff);
}

static void scen_images(golden_ctx *g)
{
   static unsigned char raw[200000];
   static const int depths[5] = { 8, 15, 16, 24, 32 };
   PALETTE pal;
   BUF b = { NULL, 0, 0 };
   char name[128], fname[64];
   int i, x, y;
   long n;

   set_palette(desktop_palette);
   for (i = 0; i < 256; i++) {
      pal[i].r = (unsigned char)((i * 7) & 63);
      pal[i].g = (unsigned char)((i * 3 + 5) & 63);
      pal[i].b = (unsigned char)((255 - i) & 63);
      pal[i].filler = 0;
   }

   /* save_bitmap -> bytes -> load_bitmap, both formats, every depth */
   for (i = 0; i < 5; i++) {
      int w = 9 + i * 2, h = 5 + i;
      BITMAP *bm = create_bitmap_ex(depths[i], w, h);
      int f;
      for (y = 0; y < h; y++)
         for (x = 0; x < w; x++) {
            int v = depths[i] == 8 ? (x * 11 + y * 29) & 0xFF
                                   : makecol_depth(depths[i], (x * 37) & 0xFF, (y * 53) & 0xFF, ((x + y) * 17) & 0xFF);
            if (x == 3 && y == 1)
               v = 0x3F;   /* a run breaker */
            putpixel(bm, x, y, v);
         }
      for (y = 0; y < h; y++)   /* long runs for the PCX RLE */
         if (y & 1)
            for (x = 0; x < w; x++)
               putpixel(bm, x, y, depths[i] == 8 ? 0xC5 : makecol_depth(depths[i], 200, 10, 10));
      for (f = 0; f < 2; f++) {
         const char *ext = f ? "pcx" : "bmp";
         const char *path;
         sprintf(fname, "img%d.%s", depths[i], ext);
         path = tmp_path(fname);
         sprintf(name, "img/%s%d/save", ext, depths[i]);
         golden_int(g, name, save_bitmap(path, bm, i == 1 ? NULL : pal));
         n = file_bytes(path, raw, sizeof(raw));
         sprintf(name, "img/%s%d/file", ext, depths[i]);
         golden_bytes(g, name, raw, n);
         sprintf(name, "%s%d", ext, depths[i]);
         load_variants(g, name, path, depths[i] != 8);
         delete_file(path);
      }
      destroy_bitmap(bm);
   }

   /* hand made BMP variants */
   /* 1 bpp, 35 wide (two 32-pixel groups per row) */
   bmp_header(&b, 35, 3, 1, 0, 54 + 8);
   bmp_palette(&b, 2);
   for (i = 0; i < 3 * 8; i++)
      bbyte(&b, (i * 73) ^ 0xA5);
   write_buf(tmp_path("h1.bmp"), &b);
   load_variants(g, "h1", tmp_path("h1.bmp"), 0);
   bfree(&b);
   /* 4 bpp, 11 wide */
   bmp_header(&b, 11, 4, 4, 0, 54 + 64);
   bmp_palette(&b, 16);
   for (i = 0; i < 4 * 8; i++)
      bbyte(&b, i * 37);
   write_buf(tmp_path("h4.bmp"), &b);
   load_variants(g, "h4", tmp_path("h4.bmp"), 0);
   bfree(&b);
   /* 8 bpp RLE8 */
   bmp_header(&b, 10, 4, 8, 1, 54 + 1024);
   bmp_palette(&b, 256);
   bput(&b, "\x04\x10\x00\x03\x01\x02\x03\x00\x02\x05\x00\x00", 12);  /* run, absolute (odd), run, eol */
   bput(&b, "\x00\x02\x02\x01\x03\x20\x00\x00", 8);                   /* delta, run, eol */
   bput(&b, "\x0A\x33\x00\x01", 4);                                   /* run, end of picture */
   write_buf(tmp_path("r8.bmp"), &b);
   load_variants(g, "r8", tmp_path("r8.bmp"), 0);
   bfree(&b);
   /* 4 bpp RLE4 */
   bmp_header(&b, 12, 3, 4, 2, 54 + 64);
   bmp_palette(&b, 16);
   bput(&b, "\x05\x12\x00\x06\x34\x56\x78\x9A\x00\x00", 10);          /* run, absolute 6, eol */
   bput(&b, "\x07\xF0\x00\x01", 4);                                   /* run, end */
   write_buf(tmp_path("r4.bmp"), &b);
   load_variants(g, "r4", tmp_path("r4.bmp"), 0);
   bfree(&b);
   /* bitfields 565, 555 and 888 */
   for (i = 0; i < 3; i++) {
      static const long rmask[3] = { 0xF800, 0x7C00, 0xFF0000 };
      static const long gmask[3] = { 0x07E0, 0x03E0, 0x00FF00 };
      static const long bmask[3] = { 0x001F, 0x001F, 0x0000FF };
      int bytes = i == 2 ? 4 : 2, w = 5, h = -3;   /* top-down */
      bmp_header(&b, w, h, bytes * 8, 3, 54 + 12);
      bi32(&b, rmask[i]);
      bi32(&b, gmask[i]);
      bi32(&b, bmask[i]);
      for (y = 0; y < 3; y++) {
         for (x = 0; x < w * bytes; x++)
            bbyte(&b, (x * 31 + y * 97) & 0xFF);
         for (x = (w * bytes) % 4; x && x < 4; x++)
            bbyte(&b, 0xEE);
      }
      sprintf(fname, "bf%d.bmp", i);
      write_buf(tmp_path(fname), &b);
      sprintf(name, "bf%d", i);
      load_variants(g, name, tmp_path(fname), 1);
      delete_file(tmp_path(fname));
      bfree(&b);
   }
   /* 16, 24 (top-down) and 32 bpp BI_RGB */
   for (i = 0; i < 3; i++) {
      static const int bits[3] = { 16, 24, 32 };
      int bytes = bits[i] / 8, w = 7, h = i == 1 ? -4 : 4;
      bmp_header(&b, w, h, bits[i], 0, 54);
      for (y = 0; y < 4; y++) {
         for (x = 0; x < w * bytes; x++)
            bbyte(&b, (x * 45 + y * 23 + i) & 0xFF);
         for (x = (w * bytes) % 4; x && x < 4; x++)
            bbyte(&b, 0x11);
      }
      sprintf(fname, "rgb%d.bmp", bits[i]);
      write_buf(tmp_path(fname), &b);
      sprintf(name, "rgb%d", bits[i]);
      load_variants(g, name, tmp_path(fname), 1);
      delete_file(tmp_path(fname));
      bfree(&b);
   }
   /* OS/2 header, 8 bpp */
   bi16(&b, 0x4D42);
   bi32(&b, 0);
   bi32(&b, 0);
   bi32(&b, 26 + 3 * 256);
   bi32(&b, 12);
   bi16(&b, 6);
   bi16(&b, 2);
   bi16(&b, 1);
   bi16(&b, 8);
   for (i = 0; i < 256; i++) {
      bbyte(&b, i);
      bbyte(&b, 255 - i);
      bbyte(&b, (i * 5) & 0xFF);
   }
   for (i = 0; i < 16; i++)
      bbyte(&b, i * 15);
   write_buf(tmp_path("os2.bmp"), &b);
   load_variants(g, "os2", tmp_path("os2.bmp"), 0);
   bfree(&b);
   /* not a BMP */
   bput(&b, "XX not a bitmap at all", 22);
   write_buf(tmp_path("bad.bmp"), &b);
   load_variants(g, "bad", tmp_path("bad.bmp"), 0);
   bfree(&b);

   delete_file(tmp_path("h1.bmp"));
   delete_file(tmp_path("h4.bmp"));
   delete_file(tmp_path("r8.bmp"));
   delete_file(tmp_path("r4.bmp"));
   delete_file(tmp_path("os2.bmp"));
   delete_file(tmp_path("bad.bmp"));

   golden_int(g, "img/unknown_load", load_bitmap(tmp_path("x.xyz"), NULL) != NULL);
   golden_int(g, "img/missing_load", load_bitmap(tmp_path("missing.bmp"), NULL) != NULL);
}

void scen_data(golden_ctx *g)
{
   set_color_depth(32);
   set_color_conversion(0x00ffffff);
   scen_paths(g);
   scen_config(g);
   scen_packfiles(g);
   scen_files(g);
   scen_datafiles(g);
   scen_synthetic(g);
   scen_images(g);
}

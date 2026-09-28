/*
 * PACKFILE (plain, LZSS packed, password), chunks for the datafile reader,
 * memory packfiles, file utilities and config files.
 *
 * Ported from Allegro 4.4.1 (giftware licence): file.c (packfiles, chunks,
 * special "file.dat#OBJECT" names, path helpers, file_exists/for_each_file_ex),
 * lzss.c (Okumura/Hargreaves LZSS codec, verbatim), win/wfile.c
 * (al_findfirst attribute masks) and config.c (parser and lookup rules).
 *
 * Files are accessed through SDL_IOStream.  Relative (historical) game paths
 * are resolved with the platform layer: plat_resolve_read() for reading,
 * existence, sizes and enumeration (for_each_file_ex enumerates the union
 * of the user root and all asset roots, user root first), and
 * plat_resolve_write() for writing.  Absolute paths pass through.
 */
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "a4_internal.h"
#include "port/platform/platform.h"

#ifdef _WIN32
/* a4_file_win32.c: kept apart because <windows.h> clashes with allegro.h */
int a4_win32_file_attrib(const char *utf8_path, int *exists);
int a4_win32_executable_name(char *out, int size);
#endif

#ifndef EROFS
#define EROFS 30
#endif

#define F_BUF_SIZE      4096
#define F_PACK_MAGIC    0x736C6821L    /* "slh!" packed file */
#define F_NOPACK_MAGIC  0x736C682EL    /* "slh." autodetect, not packed */

#define PACKFILE_FLAG_WRITE      1
#define PACKFILE_FLAG_PACK       2
#define PACKFILE_FLAG_CHUNK      4
#define PACKFILE_FLAG_EOF        8
#define PACKFILE_FLAG_ERROR      16
#define PACKFILE_FLAG_OLD_CRYPT  32
#define PACKFILE_FLAG_EXEDAT     64

#define FA_DAT_FLAGS  (FA_RDONLY | FA_ARCH)

#define LZ_N          4096
#define LZ_F          18
#define LZ_THRESHOLD  2

typedef struct LZSS_PACK_DATA {
   int state;
   int i, c, len, r, s;
   int last_match_length, code_buf_ptr;
   unsigned char mask;
   char code_buf[17];
   int match_position;
   int match_length;
   int lson[LZ_N + 1];
   int rson[LZ_N + 257];
   int dad[LZ_N + 1];
   unsigned char text_buf[LZ_N + LZ_F - 1];
} LZSS_PACK_DATA;

typedef struct LZSS_UNPACK_DATA {
   int state;
   int i, j, k, r, c;
   int flags;
   unsigned char text_buf[LZ_N + LZ_F - 1];
} LZSS_UNPACK_DATA;

/* Allegro's struct _al_normal_packfile_details; the file descriptor is an
 * SDL_IOStream, and `src_*` remember how to reopen it (OLD_CRYPT mode). */
struct PACKFILE {
   int flags;
   unsigned char *buf_pos;
   int buf_size;
   long todo;
   struct PACKFILE *parent;
   LZSS_PACK_DATA *pack_data;
   LZSS_UNPACK_DATA *unpack_data;
   char *passdata;
   char *passpos;
   SDL_IOStream *io;
   char *src_path;
   const void *src_mem;
   long src_mem_size;
   unsigned char buf[F_BUF_SIZE];
};

static char the_password[256];
static int packfile_filesize;
static int packfile_datasize;
int a4_packfile_type;

static void set_errno(int e) { if (allegro_errno) *allegro_errno = e; }
static int get_errno(void) { return allegro_errno ? *allegro_errno : 0; }

/* ================================================================== */
/* LZSS (lzss.c, verbatim algorithm)                                  */
/* ================================================================== */

static LZSS_PACK_DATA *create_lzss_pack_data(void)
{
   LZSS_PACK_DATA *dat = (LZSS_PACK_DATA *)calloc(1, sizeof(LZSS_PACK_DATA));
   if (!dat) {
      set_errno(ENOMEM);
      return NULL;
   }
   dat->state = 0;
   return dat;
}

static LZSS_UNPACK_DATA *create_lzss_unpack_data(void)
{
   LZSS_UNPACK_DATA *dat = (LZSS_UNPACK_DATA *)calloc(1, sizeof(LZSS_UNPACK_DATA));
   if (!dat) {
      set_errno(ENOMEM);
      return NULL;
   }
   dat->state = 0;
   return dat;
}

static void lzss_inittree(LZSS_PACK_DATA *dat)
{
   int i;
   for (i = LZ_N + 1; i <= LZ_N + 256; i++)
      dat->rson[i] = LZ_N;
   for (i = 0; i < LZ_N; i++)
      dat->dad[i] = LZ_N;
}

static void lzss_insertnode(int r, LZSS_PACK_DATA *dat)
{
   int i, p, cmp;
   unsigned char *lzkey;
   unsigned char *text_buf = dat->text_buf;

   cmp = 1;
   lzkey = &text_buf[r];
   p = LZ_N + 1 + lzkey[0];
   dat->rson[r] = dat->lson[r] = LZ_N;
   dat->match_length = 0;

   for (;;) {
      if (cmp >= 0) {
         if (dat->rson[p] != LZ_N)
            p = dat->rson[p];
         else {
            dat->rson[p] = r;
            dat->dad[r] = p;
            return;
         }
      } else {
         if (dat->lson[p] != LZ_N)
            p = dat->lson[p];
         else {
            dat->lson[p] = r;
            dat->dad[r] = p;
            return;
         }
      }
      for (i = 1; i < LZ_F; i++)
         if ((cmp = lzkey[i] - text_buf[p + i]) != 0)
            break;
      if (i > dat->match_length) {
         dat->match_position = p;
         if ((dat->match_length = i) >= LZ_F)
            break;
      }
   }

   dat->dad[r] = dat->dad[p];
   dat->lson[r] = dat->lson[p];
   dat->rson[r] = dat->rson[p];
   dat->dad[dat->lson[p]] = r;
   dat->dad[dat->rson[p]] = r;
   if (dat->rson[dat->dad[p]] == p)
      dat->rson[dat->dad[p]] = r;
   else
      dat->lson[dat->dad[p]] = r;
   dat->dad[p] = LZ_N;
}

static void lzss_deletenode(int p, LZSS_PACK_DATA *dat)
{
   int q;

   if (dat->dad[p] == LZ_N)
      return;
   if (dat->rson[p] == LZ_N)
      q = dat->lson[p];
   else if (dat->lson[p] == LZ_N)
      q = dat->rson[p];
   else {
      q = dat->lson[p];
      if (dat->rson[q] != LZ_N) {
         do {
            q = dat->rson[q];
         } while (dat->rson[q] != LZ_N);
         dat->rson[dat->dad[q]] = dat->lson[q];
         dat->dad[dat->lson[q]] = dat->dad[q];
         dat->lson[q] = dat->lson[p];
         dat->dad[dat->lson[p]] = q;
      }
      dat->rson[q] = dat->rson[p];
      dat->dad[dat->rson[p]] = q;
   }
   dat->dad[q] = dat->dad[p];
   if (dat->rson[dat->dad[p]] == p)
      dat->rson[dat->dad[p]] = q;
   else
      dat->lson[dat->dad[p]] = q;
   dat->dad[p] = LZ_N;
}

static int lzss_write(PACKFILE *file, LZSS_PACK_DATA *dat, int size, unsigned char *buf, int last)
{
   int i = dat->i;
   int c = dat->c;
   int len = dat->len;
   int r = dat->r;
   int s = dat->s;
   int last_match_length = dat->last_match_length;
   int code_buf_ptr = dat->code_buf_ptr;
   unsigned char mask = dat->mask;
   int ret = 0;

   if (dat->state == 2)
      goto pos2;
   else if (dat->state == 1)
      goto pos1;

   dat->code_buf[0] = 0;
   code_buf_ptr = mask = 1;
   s = 0;
   r = LZ_N - LZ_F;
   lzss_inittree(dat);

   for (len = 0; (len < LZ_F) && (size > 0); len++) {
      dat->text_buf[r + len] = *(buf++);
      if (--size == 0) {
         if (!last) {
            dat->state = 1;
            goto getout;
         }
      }
    pos1:
      ;
   }

   if (len == 0)
      goto getout;

   for (i = 1; i <= LZ_F; i++)
      lzss_insertnode(r - i, dat);
   lzss_insertnode(r, dat);

   do {
      if (dat->match_length > len)
         dat->match_length = len;

      if (dat->match_length <= LZ_THRESHOLD) {
         dat->match_length = 1;
         dat->code_buf[0] |= mask;
         dat->code_buf[code_buf_ptr++] = dat->text_buf[r];
      } else {
         dat->code_buf[code_buf_ptr++] = (unsigned char)dat->match_position;
         dat->code_buf[code_buf_ptr++] = (unsigned char)
            (((dat->match_position >> 4) & 0xF0) | (dat->match_length - (LZ_THRESHOLD + 1)));
      }

      if ((mask <<= 1) == 0) {
         if ((file->passpos) && (file->flags & PACKFILE_FLAG_OLD_CRYPT)) {
            dat->code_buf[0] ^= *file->passpos;
            file->passpos++;
            if (!*file->passpos)
               file->passpos = file->passdata;
         }
         for (i = 0; i < code_buf_ptr; i++)
            pack_putc((unsigned char)dat->code_buf[i], file);
         if (pack_ferror(file)) {
            ret = EOF;
            goto getout;
         }
         dat->code_buf[0] = 0;
         code_buf_ptr = mask = 1;
      }

      last_match_length = dat->match_length;

      for (i = 0; (i < last_match_length) && (size > 0); i++) {
         c = *(buf++);
         if (--size == 0) {
            if (!last) {
               dat->state = 2;
               goto getout;
            }
         }
       pos2:
         lzss_deletenode(s, dat);
         dat->text_buf[s] = (unsigned char)c;
         if (s < LZ_F - 1)
            dat->text_buf[s + LZ_N] = (unsigned char)c;
         s = (s + 1) & (LZ_N - 1);
         r = (r + 1) & (LZ_N - 1);
         lzss_insertnode(r, dat);
      }

      while (i++ < last_match_length) {
         lzss_deletenode(s, dat);
         s = (s + 1) & (LZ_N - 1);
         r = (r + 1) & (LZ_N - 1);
         if (--len)
            lzss_insertnode(r, dat);
      }
   } while (len > 0);

   if (code_buf_ptr > 1) {
      if ((file->passpos) && (file->flags & PACKFILE_FLAG_OLD_CRYPT)) {
         dat->code_buf[0] ^= *file->passpos;
         file->passpos++;
         if (!*file->passpos)
            file->passpos = file->passdata;
      }
      for (i = 0; i < code_buf_ptr; i++) {
         pack_putc((unsigned char)dat->code_buf[i], file);
         if (pack_ferror(file)) {
            ret = EOF;
            goto getout;
         }
      }
   }

   dat->state = 0;

 getout:
   dat->i = i;
   dat->c = c;
   dat->len = len;
   dat->r = r;
   dat->s = s;
   dat->last_match_length = last_match_length;
   dat->code_buf_ptr = code_buf_ptr;
   dat->mask = mask;
   return ret;
}

static int lzss_read(PACKFILE *file, LZSS_UNPACK_DATA *dat, int s, unsigned char *buf)
{
   int i = dat->i;
   int j = dat->j;
   int k = dat->k;
   int r = dat->r;
   int c = dat->c;
   unsigned int flags = (unsigned int)dat->flags;
   int size = 0;

   if (dat->state == 2)
      goto pos2;
   else if (dat->state == 1)
      goto pos1;

   r = LZ_N - LZ_F;
   flags = 0;

   for (;;) {
      if (((flags >>= 1) & 256) == 0) {
         if ((c = pack_getc(file)) == EOF)
            break;
         if ((file->passpos) && (file->flags & PACKFILE_FLAG_OLD_CRYPT)) {
            c ^= *file->passpos;
            file->passpos++;
            if (!*file->passpos)
               file->passpos = file->passdata;
         }
         flags = (unsigned int)c | 0xFF00;
      }

      if (flags & 1) {
         if ((c = pack_getc(file)) == EOF)
            break;
         dat->text_buf[r++] = (unsigned char)c;
         r &= (LZ_N - 1);
         *(buf++) = (unsigned char)c;
         if (++size >= s) {
            dat->state = 1;
            goto getout;
         }
       pos1:
         ;
      } else {
         if ((i = pack_getc(file)) == EOF)
            break;
         if ((j = pack_getc(file)) == EOF)
            break;
         i |= ((j & 0xF0) << 4);
         j = (j & 0x0F) + LZ_THRESHOLD;
         for (k = 0; k <= j; k++) {
            c = dat->text_buf[(i + k) & (LZ_N - 1)];
            dat->text_buf[r++] = (unsigned char)c;
            r &= (LZ_N - 1);
            *(buf++) = (unsigned char)c;
            if (++size >= s) {
               dat->state = 2;
               goto getout;
            }
          pos2:
            ;
         }
      }
   }

   dat->state = 0;

 getout:
   dat->i = i;
   dat->j = j;
   dat->k = k;
   dat->r = r;
   dat->c = c;
   dat->flags = (int)flags;
   return size;
}

/* ================================================================== */
/* normal packfile vtable (file.c)                                    */
/* ================================================================== */

static int normal_refill_buffer(PACKFILE *f);
static int normal_flush_buffer(PACKFILE *f, int last);

static PACKFILE *create_packfile(void)
{
   PACKFILE *f = (PACKFILE *)calloc(1, sizeof(PACKFILE));
   if (!f) {
      set_errno(ENOMEM);
      return NULL;
   }
   f->buf_pos = f->buf;
   return f;
}

static void free_packfile(PACKFILE *f)
{
   if (f) {
      free(f->src_path);
      free(f);
   }
}

static int normal_no_more_input(PACKFILE *f)
{
   if (f->parent && (f->flags & PACKFILE_FLAG_PACK) && f->unpack_data && f->unpack_data->state == 2)
      return 0;
   return (f->todo <= 0);
}

static int normal_getc(PACKFILE *f)
{
   f->buf_size--;
   if (f->buf_size > 0)
      return *(f->buf_pos++);
   if (f->buf_size == 0) {
      if (normal_no_more_input(f))
         f->flags |= PACKFILE_FLAG_EOF;
      return *(f->buf_pos++);
   }
   return normal_refill_buffer(f);
}

static int normal_putc(int c, PACKFILE *f)
{
   if (f->buf_size + 1 >= F_BUF_SIZE) {
      if (normal_flush_buffer(f, FALSE))
         return EOF;
   }
   f->buf_size++;
   return (*(f->buf_pos++) = (unsigned char)c);
}

static int normal_refill_buffer(PACKFILE *f)
{
   int i;

   if (f->flags & PACKFILE_FLAG_EOF)
      return EOF;

   if (normal_no_more_input(f)) {
      f->flags |= PACKFILE_FLAG_EOF;
      return EOF;
   }

   if (f->parent) {
      if (f->flags & PACKFILE_FLAG_PACK)
         f->buf_size = lzss_read(f->parent, f->unpack_data, (int)MIN((long)F_BUF_SIZE, f->todo), f->buf);
      else
         f->buf_size = (int)pack_fread(f->buf, MIN((long)F_BUF_SIZE, f->todo), f->parent);
      if (f->parent->flags & PACKFILE_FLAG_EOF)
         f->todo = 0;
      if (f->parent->flags & PACKFILE_FLAG_ERROR)
         goto Error;
   } else {
      size_t want, got;
      f->buf_size = (int)MIN((long)F_BUF_SIZE, f->todo);
      want = (size_t)f->buf_size;
      got = f->io ? SDL_ReadIO(f->io, f->buf, want) : 0;
      if (got != want)
         goto Error;
      if ((f->passpos) && (!(f->flags & PACKFILE_FLAG_OLD_CRYPT))) {
         for (i = 0; i < f->buf_size; i++) {
            f->buf[i] ^= (unsigned char)*(f->passpos++);
            if (!*f->passpos)
               f->passpos = f->passdata;
         }
      }
   }

   f->todo -= f->buf_size;
   f->buf_pos = f->buf;
   f->buf_size--;
   if (f->buf_size <= 0)
      if (normal_no_more_input(f))
         f->flags |= PACKFILE_FLAG_EOF;

   if (f->buf_size < 0)
      return EOF;
   return *(f->buf_pos++);

 Error:
   set_errno(EFAULT);
   f->flags |= PACKFILE_FLAG_ERROR;
   return EOF;
}

static int normal_flush_buffer(PACKFILE *f, int last)
{
   int i;

   if (f->buf_size > 0) {
      if (f->flags & PACKFILE_FLAG_PACK) {
         if (lzss_write(f->parent, f->pack_data, f->buf_size, f->buf, last))
            goto Error;
      } else {
         if ((f->passpos) && (!(f->flags & PACKFILE_FLAG_OLD_CRYPT))) {
            for (i = 0; i < f->buf_size; i++) {
               f->buf[i] ^= (unsigned char)*(f->passpos++);
               if (!*f->passpos)
                  f->passpos = f->passdata;
            }
         }
         if (!f->io || SDL_WriteIO(f->io, f->buf, (size_t)f->buf_size) != (size_t)f->buf_size)
            goto Error;
      }
      f->todo += f->buf_size;
   }
   f->buf_pos = f->buf;
   f->buf_size = 0;
   return 0;

 Error:
   set_errno(EFAULT);
   f->flags |= PACKFILE_FLAG_ERROR;
   return EOF;
}

static int normal_fclose(PACKFILE *f)
{
   int ret;

   if (f->flags & PACKFILE_FLAG_WRITE)
      normal_flush_buffer(f, TRUE);

   if (f->parent) {
      ret = pack_fclose(f->parent);
   } else {
      ret = 0;
      if (f->io && !SDL_CloseIO(f->io)) {
         ret = -1;
         set_errno(EIO);
      }
      f->io = NULL;
   }

   free(f->pack_data);
   f->pack_data = NULL;
   free(f->unpack_data);
   f->unpack_data = NULL;
   free(f->passdata);
   f->passdata = NULL;
   f->passpos = NULL;
   return ret;
}

static int normal_fseek(PACKFILE *f, int offset)
{
   int i;

   if (f->flags & PACKFILE_FLAG_WRITE)
      return -1;

   set_errno(0);

   if (f->buf_size > 0) {
      i = MIN(offset, f->buf_size);
      f->buf_size -= i;
      f->buf_pos += i;
      offset -= i;
      if ((f->buf_size <= 0) && normal_no_more_input(f))
         f->flags |= PACKFILE_FLAG_EOF;
   }

   if (offset > 0) {
      i = (int)MIN((long)offset, f->todo);
      if ((f->flags & PACKFILE_FLAG_PACK) || (f->passpos)) {
         while (i > 0) {
            pack_getc(f);
            i--;
         }
      } else {
         if (f->parent)
            pack_fseek(f->parent, i);
         else if (f->io)
            SDL_SeekIO(f->io, i, SDL_IO_SEEK_CUR);
         f->todo -= i;
         if (normal_no_more_input(f))
            f->flags |= PACKFILE_FLAG_EOF;
      }
   }

   return get_errno() ? -1 : 0;
}

/* ================================================================== */
/* opening                                                            */
/* ================================================================== */

void packfile_password(const char *password)
{
   int i = 0, c;
   if (password) {
      while ((c = a4_ugetx(&password)) != 0) {
         the_password[i++] = (char)c;
         if (i >= (int)sizeof(the_password) - 1)
            break;
      }
   }
   the_password[i] = 0;
}

/* encrypt_id: characters are sign extended like the x86 `char` Allegro
 * was built with */
static int32_t encrypt_id(long x, int new_format)
{
   uint32_t mask = 0;
   int i, pos;

   if (the_password[0]) {
      for (i = 0; the_password[i]; i++)
         mask ^= (uint32_t)(int32_t)(signed char)the_password[i] << ((i & 3) * 8);
      for (i = 0, pos = 0; i < 4; i++) {
         mask ^= (uint32_t)(int32_t)(signed char)the_password[pos++] << (24 - i * 8);
         if (!the_password[pos])
            pos = 0;
      }
      if (new_format)
         mask ^= 42;
   }
   return (int32_t)((uint32_t)x ^ mask);
}

static int clone_password(PACKFILE *f)
{
   if (the_password[0]) {
      size_t n = strlen(the_password) + 1;
      if ((f->passdata = (char *)malloc(n)) == NULL) {
         set_errno(ENOMEM);
         return FALSE;
      }
      memcpy(f->passdata, the_password, n);
      f->passpos = f->passdata;
   } else {
      f->passpos = NULL;
      f->passdata = NULL;
   }
   return TRUE;
}

static char *dup_str(const char *s)
{
   char *d;
   size_t n;
   if (!s)
      return NULL;
   n = strlen(s) + 1;
   d = (char *)malloc(n);
   if (d)
      memcpy(d, s, n);
   return d;
}

/* reopen the source of a leaf packfile (replaces dup(fd) + lseek(0)) */
static SDL_IOStream *reopen_source(const char *path, const void *mem, long mem_size)
{
   if (path)
      return SDL_IOFromFile(path, "rb");
   if (mem)
      return SDL_IOFromConstMem(mem, (size_t)mem_size);
   return NULL;
}

/* _pack_fdopen (file.c).  Takes ownership of `io` (closed on failure). */
static PACKFILE *pack_fdopen(SDL_IOStream *io, const char *mode, const char *path,
                             const void *mem, long mem_size)
{
   PACKFILE *f, *f2;
   long header = FALSE;
   int c;

   if ((f = create_packfile()) == NULL) {
      SDL_CloseIO(io);
      return NULL;
   }

   while ((c = *(mode++)) != 0) {
      switch (c) {
         case 'r': case 'R': f->flags &= ~PACKFILE_FLAG_WRITE; break;
         case 'w': case 'W': f->flags |= PACKFILE_FLAG_WRITE; break;
         case 'p': case 'P': f->flags |= PACKFILE_FLAG_PACK; break;
         case '!': f->flags &= ~PACKFILE_FLAG_PACK; header = TRUE; break;
      }
   }

   if (f->flags & PACKFILE_FLAG_WRITE) {
      if (f->flags & PACKFILE_FLAG_PACK) {
         /* write a packed file */
         f->pack_data = create_lzss_pack_data();
         if (!f->pack_data) {
            free_packfile(f);
            SDL_CloseIO(io);
            return NULL;
         }
         if ((f->parent = pack_fdopen(io, F_WRITE, path, mem, mem_size)) == NULL) {
            free(f->pack_data);
            free_packfile(f);
            return NULL;
         }
         pack_mputl(encrypt_id(F_PACK_MAGIC, TRUE), f->parent);
         f->todo = 4;
      } else {
         /* write a 'real' file */
         if (!clone_password(f)) {
            free_packfile(f);
            SDL_CloseIO(io);
            return NULL;
         }
         f->io = io;
         f->todo = 0;
         if (header)
            pack_mputl(encrypt_id(F_NOPACK_MAGIC, TRUE), f);
      }
   } else {
      if (f->flags & PACKFILE_FLAG_PACK) {
         /* read a packed file */
         f->unpack_data = create_lzss_unpack_data();
         if (!f->unpack_data) {
            free_packfile(f);
            SDL_CloseIO(io);
            return NULL;
         }
         if ((f->parent = pack_fdopen(io, F_READ, path, mem, mem_size)) == NULL) {
            free(f->unpack_data);
            free_packfile(f);
            return NULL;
         }

         header = pack_mgetl(f->parent);

         if ((f->parent->passpos) &&
             ((header == encrypt_id(F_PACK_MAGIC, FALSE)) ||
              (header == encrypt_id(F_NOPACK_MAGIC, FALSE)))) {
            /* backward compatibility mode: reopen the source */
            SDL_IOStream *io2;
            pack_fclose(f->parent);
            f->parent = NULL;
            if (!clone_password(f)) {
               free(f->unpack_data);
               free_packfile(f);
               return NULL;
            }
            f->flags |= PACKFILE_FLAG_OLD_CRYPT;
            io2 = reopen_source(path, mem, mem_size);
            if (!io2 || (f->parent = pack_fdopen(io2, F_READ, path, mem, mem_size)) == NULL) {
               free(f->unpack_data);
               free(f->passdata);
               free_packfile(f);
               set_errno(ENOENT);
               return NULL;
            }
            f->parent->flags |= PACKFILE_FLAG_OLD_CRYPT;
            pack_mgetl(f->parent);
            if (header == encrypt_id(F_PACK_MAGIC, FALSE))
               header = encrypt_id(F_PACK_MAGIC, TRUE);
            else
               header = encrypt_id(F_NOPACK_MAGIC, TRUE);
         }

         if (header == encrypt_id(F_PACK_MAGIC, TRUE)) {
            f->todo = LONG_MAX;
         } else if (header == encrypt_id(F_NOPACK_MAGIC, TRUE)) {
            f2 = f->parent;
            free(f->unpack_data);
            free(f->passdata);
            free_packfile(f);
            return f2;
         } else {
            pack_fclose(f->parent);
            free(f->unpack_data);
            free(f->passdata);
            free_packfile(f);
            set_errno(EDOM);
            return NULL;
         }
      } else {
         /* read a 'real' file */
         Sint64 sz = SDL_GetIOSize(io);
         if (sz < 0) {
            set_errno(EIO);
            free_packfile(f);
            SDL_CloseIO(io);
            return NULL;
         }
         f->todo = (long)sz;
         if (!clone_password(f)) {
            free_packfile(f);
            SDL_CloseIO(io);
            return NULL;
         }
         f->io = io;
         f->src_path = dup_str(path);
         f->src_mem = mem;
         f->src_mem_size = mem_size;
      }
   }
   return f;
}

static PACKFILE *pack_fopen_special_file(const char *filename, const char *mode);

PACKFILE *pack_fopen(const char *filename, const char *mode)
{
   char path[2048];
   SDL_IOStream *io;
   int writing;

   if (!filename || !mode)
      return NULL;
   a4_packfile_type = 0;

   if (strchr(filename, '#')) {
      PACKFILE *special = pack_fopen_special_file(filename, mode);
      if (special)
         return special;
   }

   writing = strpbrk(mode, "wW") != NULL;
   if (writing) {
      if (!plat_resolve_write(filename, path, sizeof(path))) {
         set_errno(ENAMETOOLONG);
         return NULL;
      }
      io = SDL_IOFromFile(path, "wb");
   } else {
      if (!plat_resolve_read(filename, path, sizeof(path))) {
         set_errno(ENAMETOOLONG);
         return NULL;
      }
      io = SDL_IOFromFile(path, "rb");
   }
   if (!io) {
      set_errno(writing ? EACCES : ENOENT);
      return NULL;
   }
   return pack_fdopen(io, mode, writing ? NULL : path, NULL, 0);
}

PACKFILE *a4_pack_fopen_memory(const void *data, long size)
{
   PACKFILE *f;
   SDL_IOStream *io;
   if (!data || size < 0)
      return NULL;
   io = SDL_IOFromConstMem(data, (size_t)size);
   if (!io)
      return NULL;
   f = create_packfile();
   if (!f) {
      SDL_CloseIO(io);
      return NULL;
   }
   /* the data is already decoded: no password is applied */
   f->io = io;
   f->todo = size;
   f->src_mem = data;
   f->src_mem_size = size;
   return f;
}

int pack_fclose(PACKFILE *f)
{
   int ret;
   if (!f)
      return 0;
   ret = normal_fclose(f);
   free_packfile(f);
   return ret;
}

/* pack_fopen_chunk (file.c), read side.  Writing sub-chunks (datafile
 * creation) is not supported. */
PACKFILE *a4_pack_fopen_chunk(PACKFILE *f, int pack)
{
   PACKFILE *chunk;
   (void)pack;

   if (!f)
      return NULL;
   if (f->flags & PACKFILE_FLAG_WRITE) {
      set_errno(EINVAL);
      return NULL;
   }

   packfile_filesize = (int)pack_mgetl(f);
   packfile_datasize = (int)pack_mgetl(f);

   if ((chunk = create_packfile()) == NULL)
      return NULL;

   chunk->flags = PACKFILE_FLAG_CHUNK;
   chunk->parent = f;

   if (f->flags & PACKFILE_FLAG_OLD_CRYPT) {
      if (f->passdata) {
         chunk->passdata = dup_str(f->passdata);
         if (!chunk->passdata) {
            set_errno(ENOMEM);
            free(chunk);
            return NULL;
         }
         chunk->passpos = chunk->passdata + (f->passpos - f->passdata);
         f->passpos = f->passdata;
      }
      chunk->flags |= PACKFILE_FLAG_OLD_CRYPT;
   }

   if (packfile_datasize < 0) {
      chunk->unpack_data = create_lzss_unpack_data();
      if (!chunk->unpack_data) {
         free(chunk->passdata);
         free_packfile(chunk);
         return NULL;
      }
      packfile_datasize = -packfile_datasize;
      chunk->todo = packfile_datasize;
      chunk->flags |= PACKFILE_FLAG_PACK;
   } else {
      chunk->todo = packfile_datasize;
   }
   return chunk;
}

PACKFILE *a4_pack_fclose_chunk(PACKFILE *f)
{
   PACKFILE *parent;
   if (!f)
      return NULL;
   parent = f->parent;
   if (f->flags & PACKFILE_FLAG_WRITE) {
      set_errno(EINVAL);
      return NULL;
   }
   while (f->todo > 0)
      pack_getc(f);
   free(f->unpack_data);
   f->unpack_data = NULL;
   if ((f->passpos) && (f->flags & PACKFILE_FLAG_OLD_CRYPT))
      parent->passpos = parent->passdata + (f->passpos - f->passdata);
   free(f->passdata);
   free_packfile(f);
   return parent;
}

long a4_pack_todo(PACKFILE *f) { return f ? f->todo : 0; }

int a4_pack_is_datafile_chunk(PACKFILE *f)
{
   return f && (f->flags & PACKFILE_FLAG_CHUNK) && !(f->flags & PACKFILE_FLAG_EXEDAT);
}

/* pack_fopen_datafile_object (file.c) */
static PACKFILE *pack_fopen_datafile_object(PACKFILE *f, const char *objname)
{
   char name[512];
   int use_next = FALSE, recurse = FALSE;
   int type, size, pos, c;

   if (!f)
      return NULL;
   pos = 0;
   while ((c = a4_ugetx(&objname)) != 0) {
      if ((c == '#') || (c == '/') || (c == '\\')) {
         recurse = TRUE;
         break;
      }
      if (pos + a4_ucwidth(c) >= (int)sizeof(name) - 1)
         break;
      pos += a4_usetc(name + pos, c);
   }
   a4_usetc(name + pos, 0);

   pack_mgetl(f);

   while (!pack_feof(f)) {
      type = (int)pack_mgetl(f);
      if (type == DAT_PROPERTY) {
         type = (int)pack_mgetl(f);
         size = (int)pack_mgetl(f);
         if (type == DAT_NAME) {
            char *buf = (char *)malloc(size > 0 ? (size_t)size + 1 : 1);
            if (!buf)
               break;
            pack_fread(buf, size > 0 ? size : 0, f);
            buf[size > 0 ? size : 0] = 0;
            if (a4_ustricmp(buf, name) == 0)
               use_next = TRUE;
            free(buf);
         } else {
            pack_fseek(f, size);
         }
      } else {
         if (use_next) {
            if (recurse) {
               if (type == DAT_FILE)
                  return pack_fopen_datafile_object(a4_pack_fopen_chunk(f, FALSE), objname);
               else
                  break;
            } else {
               a4_packfile_type = type;
               return a4_pack_fopen_chunk(f, FALSE);
            }
         } else {
            size = (int)pack_mgetl(f);
            pack_fseek(f, size + 4);
         }
      }
   }

   pack_fclose(f);
   set_errno(ENOENT);
   return NULL;
}

/* pack_fopen_special_file (file.c).  Data appended to the executable
 * ("#" and "#object" names) is not supported. */
static PACKFILE *pack_fopen_special_file(const char *filename, const char *mode)
{
   char fname[1024], objname[512];
   char *p;
   PACKFILE *f;
   int c;

   while ((c = *(mode++)) != 0) {
      if ((c == 'w') || (c == 'W')) {
         set_errno(EROFS);
         return NULL;
      }
   }
   if (filename[0] == '#') {
      set_errno(ENOENT);
      return NULL;
   }
   a4_ustrzcpy(fname, sizeof(fname), filename);
   p = strrchr(fname, '#');
   if (!p)
      return NULL;
   *p = 0;
   a4_ustrzcpy(objname, sizeof(objname), p + 1);

   f = pack_fopen(fname, F_READ_PACKED);
   if (!f)
      return NULL;
   if (pack_mgetl(f) != DAT_MAGIC) {
      pack_fclose(f);
      set_errno(ENOTDIR);
      return NULL;
   }
   return pack_fopen_datafile_object(f, objname);
}

/* ================================================================== */
/* packfile I/O helpers                                               */
/* ================================================================== */

int pack_fseek(PACKFILE *f, int offset)
{
   return normal_fseek(f, offset);
}

int pack_getc(PACKFILE *f) { return normal_getc(f); }
int pack_putc(int c, PACKFILE *f) { return normal_putc(c, f); }
int pack_feof(PACKFILE *f) { return (f->flags & PACKFILE_FLAG_EOF); }
int pack_ferror(PACKFILE *f) { return (f->flags & PACKFILE_FLAG_ERROR); }

long pack_fread(void *p, long n, PACKFILE *f)
{
   unsigned char *cp = (unsigned char *)p;
   long i;
   int c;
   for (i = 0; i < n; i++) {
      if ((c = normal_getc(f)) == EOF)
         break;
      *(cp++) = (unsigned char)c;
   }
   return i;
}

long pack_fwrite(const void *p, long n, PACKFILE *f)
{
   const unsigned char *cp = (const unsigned char *)p;
   long i;
   for (i = 0; i < n; i++) {
      if (normal_putc(*cp++, f) == EOF)
         break;
   }
   return i;
}

int pack_igetw(PACKFILE *f)
{
   int b1, b2;
   if ((b1 = pack_getc(f)) != EOF)
      if ((b2 = pack_getc(f)) != EOF)
         return ((b2 << 8) | b1);
   return EOF;
}

/* 32-bit results are sign extended like Allegro's 32-bit `long` */
long pack_igetl(PACKFILE *f)
{
   int b1, b2, b3, b4;
   if ((b1 = pack_getc(f)) != EOF)
      if ((b2 = pack_getc(f)) != EOF)
         if ((b3 = pack_getc(f)) != EOF)
            if ((b4 = pack_getc(f)) != EOF)
               return (long)(int32_t)(((uint32_t)b4 << 24) | ((uint32_t)b3 << 16) |
                                      ((uint32_t)b2 << 8) | (uint32_t)b1);
   return EOF;
}

int pack_iputw(int w, PACKFILE *f)
{
   int b1 = (w & 0xFF00) >> 8;
   int b2 = w & 0x00FF;
   if (pack_putc(b2, f) == b2)
      if (pack_putc(b1, f) == b1)
         return w;
   return EOF;
}

long pack_iputl(long l, PACKFILE *f)
{
   int b1 = (int)((l & 0xFF000000L) >> 24);
   int b2 = (int)((l & 0x00FF0000L) >> 16);
   int b3 = (int)((l & 0x0000FF00L) >> 8);
   int b4 = (int)l & 0x00FF;
   if (pack_putc(b4, f) == b4)
      if (pack_putc(b3, f) == b3)
         if (pack_putc(b2, f) == b2)
            if (pack_putc(b1, f) == b1)
               return l;
   return EOF;
}

int pack_mgetw(PACKFILE *f)
{
   int b1, b2;
   if ((b1 = pack_getc(f)) != EOF)
      if ((b2 = pack_getc(f)) != EOF)
         return ((b1 << 8) | b2);
   return EOF;
}

long pack_mgetl(PACKFILE *f)
{
   int b1, b2, b3, b4;
   if ((b1 = pack_getc(f)) != EOF)
      if ((b2 = pack_getc(f)) != EOF)
         if ((b3 = pack_getc(f)) != EOF)
            if ((b4 = pack_getc(f)) != EOF)
               return (long)(int32_t)(((uint32_t)b1 << 24) | ((uint32_t)b2 << 16) |
                                      ((uint32_t)b3 << 8) | (uint32_t)b4);
   return EOF;
}

int pack_mputw(int w, PACKFILE *f)
{
   int b1 = (w & 0xFF00) >> 8;
   int b2 = w & 0x00FF;
   if (pack_putc(b1, f) == b1)
      if (pack_putc(b2, f) == b2)
         return w;
   return EOF;
}

long pack_mputl(long l, PACKFILE *f)
{
   int b1 = (int)((l & 0xFF000000L) >> 24);
   int b2 = (int)((l & 0x00FF0000L) >> 16);
   int b3 = (int)((l & 0x0000FF00L) >> 8);
   int b4 = (int)l & 0x00FF;
   if (pack_putc(b1, f) == b1)
      if (pack_putc(b2, f) == b2)
         if (pack_putc(b3, f) == b3)
            if (pack_putc(b4, f) == b4)
               return l;
   return EOF;
}

/* ================================================================== */
/* path helpers (file.c; '\\' and ':' are separators like on Windows)  */
/* ================================================================== */

static int is_sep(int c) { return c == '/' || c == '\\' || c == ':'; }

char *get_filename(const char *path)
{
   int c;
   const char *ptr = path, *ret = path;
   for (;;) {
      c = a4_ugetx(&ptr);
      if (!c)
         break;
      if (is_sep(c))
         ret = ptr;
   }
   return (char *)ret;
}

char *get_extension(const char *filename)
{
   int pos, c;
   pos = a4_ustrlen(filename);
   while (pos > 0) {
      c = a4_ugetat(filename, pos - 1);
      if ((c == '.') || is_sep(c))
         break;
      pos--;
   }
   if ((pos > 0) && (a4_ugetat(filename, pos - 1) == '.'))
      return (char *)filename + a4_uoffset(filename, pos);
   return (char *)filename + a4_ustrsize(filename);
}

char *replace_filename(char *dest, const char *path, const char *filename, int size)
{
   char tmp[1024];
   int pos, c;
   pos = a4_ustrlen(path);
   while (pos > 0) {
      c = a4_ugetat(path, pos - 1);
      if (is_sep(c))
         break;
      pos--;
   }
   a4_ustrzncpy(tmp, sizeof(tmp), path, pos);
   a4_ustrzcat(tmp, sizeof(tmp), filename);
   a4_ustrzcpy(dest, size, tmp);
   return dest;
}

char *replace_extension(char *dest, const char *filename, const char *ext, int size)
{
   char tmp[1024];
   int pos, end, c;
   pos = end = a4_ustrlen(filename);
   while (pos > 0) {
      c = a4_ugetat(filename, pos - 1);
      if ((c == '.') || is_sep(c))
         break;
      pos--;
   }
   if (a4_ugetat(filename, pos - 1) == '.')
      end = pos - 1;
   a4_ustrzncpy(tmp, sizeof(tmp), filename, end);
   a4_ustrzcat(tmp, sizeof(tmp), ".");
   a4_ustrzcat(tmp, sizeof(tmp), ext);
   a4_ustrzcpy(dest, size, tmp);
   return dest;
}

void get_executable_name(char *output, int size)
{
   if (!output || size <= 0)
      return;
   output[0] = 0;
#ifdef _WIN32
   if (a4_win32_executable_name(output, size))
      return;
#endif
   {
      const char *base = SDL_GetBasePath();
      SDL_snprintf(output, (size_t)size, "%sicytower", base ? base : "./");
   }
}

/* ================================================================== */
/* file system: attributes, enumeration                               */
/* ================================================================== */

static int is_dot_name(const char *n)
{
   return n[0] == '.' && (n[1] == 0 || (n[1] == '.' && n[2] == 0));
}

/* Attributes as reported by _findfirst: the Win32 attribute word on
 * Windows (FILE_ATTRIBUTE_NORMAL reported as 0), synthesized elsewhere
 * (directories FA_DIREC, files FA_ARCH, dot files FA_HIDDEN). */
static int path_attrib(const char *path, const char *name, int *exists)
{
#ifdef _WIN32
   (void)name;
   return a4_win32_file_attrib(path, exists);
#else
   SDL_PathInfo info;
   int attrib;
   *exists = 0;
   if (!SDL_GetPathInfo(path, &info) || info.type == SDL_PATHTYPE_NONE)
      return 0;
   *exists = 1;
   attrib = (info.type == SDL_PATHTYPE_DIRECTORY) ? FA_DIREC : FA_ARCH;
   if (name && name[0] == '.' && !is_dot_name(name))
      attrib |= FA_HIDDEN;
   return attrib;
#endif
}

static int platform_ready(void)
{
   const char *u = plat_user_dir();
   return u && u[0];
}

/* root + relative path, dropping leading "./" like the platform layer */
static void join_path(char *out, size_t n, const char *root, const char *rel)
{
   while (rel[0] == '.' && (rel[1] == '/' || rel[1] == '\\'))
      rel += 2;
   SDL_snprintf(out, n, "%s%s", root, rel);
}

/* case-insensitive (ASCII) wildcard match with '*' and '?' */
static int wild_match(const char *pat, const char *name)
{
   const char *star_p = NULL, *star_n = NULL;
   while (*name) {
      unsigned char pc = (unsigned char)*pat, nc = (unsigned char)*name;
      if (pc == '*') {
         star_p = ++pat;
         star_n = name;
         continue;
      }
      if (pc == '?' || (pc && SDL_tolower(pc) == SDL_tolower(nc))) {
         if (pc == '?' && (nc & 0x80)) {
            /* '?' matches one whole UTF-8 character */
            name += a4_uwidth(name);
            pat++;
            continue;
         }
         pat++;
         name++;
         continue;
      }
      if (star_p) {
         pat = star_p;
         name = ++star_n;
         continue;
      }
      return 0;
   }
   while (*pat == '*')
      pat++;
   return *pat == 0;
}

/* Windows wildcard rules we reproduce: "*.*" matches every name, and a
 * trailing ".*" also matches names without an extension. */
static int name_matches(const char *pat, const char *name)
{
   size_t pl = strlen(pat);
   if (!strcmp(pat, "*.*"))
      return 1;
   if (wild_match(pat, name))
      return 1;
   if (pl >= 2 && pat[pl - 2] == '.' && pat[pl - 1] == '*' && !strchr(name, '.')) {
      char tmp[1024];
      if (pl - 2 < sizeof(tmp)) {
         memcpy(tmp, pat, pl - 2);
         tmp[pl - 2] = 0;
         return wild_match(tmp, name);
      }
   }
   return 0;
}

static int has_wildcards(const char *s)
{
   return strchr(s, '*') || strchr(s, '?');
}

typedef struct A4_DIRENT {
   char *name;
   int attrib;
} A4_DIRENT;

typedef struct A4_DIRLIST {
   A4_DIRENT *e;
   int n, cap;
   const char *pattern;
   const char *physdir;
} A4_DIRLIST;

static int dirlist_has(const A4_DIRLIST *l, const char *name)
{
   int i;
   for (i = 0; i < l->n; i++)
      if (!SDL_strcasecmp(l->e[i].name, name))
         return 1;
   return 0;
}

static void dirlist_add(A4_DIRLIST *l, const char *name, int attrib)
{
   if (l->n == l->cap) {
      int cap = l->cap ? l->cap * 2 : 32;
      A4_DIRENT *e = (A4_DIRENT *)realloc(l->e, sizeof(A4_DIRENT) * (size_t)cap);
      if (!e)
         return;
      l->e = e;
      l->cap = cap;
   }
   l->e[l->n].name = dup_str(name);
   if (!l->e[l->n].name)
      return;
   l->e[l->n].attrib = attrib;
   l->n++;
}

static SDL_EnumerationResult SDLCALL enum_cb(void *userdata, const char *dirname, const char *fname)
{
   A4_DIRLIST *l = (A4_DIRLIST *)userdata;
   char full[2048];
   int exists, attrib;
   (void)dirname;
   if (is_dot_name(fname) || !name_matches(l->pattern, fname) || dirlist_has(l, fname))
      return SDL_ENUM_CONTINUE;
   SDL_snprintf(full, sizeof(full), "%s%s", l->physdir, fname);
   attrib = path_attrib(full, fname, &exists);
   if (exists)
      dirlist_add(l, fname, attrib);
   return SDL_ENUM_CONTINUE;
}

/* NTFS index order: upper-cased names, "." and ".." first */
static int dirent_cmp(const void *a, const void *b)
{
   const unsigned char *x = (const unsigned char *)((const A4_DIRENT *)a)->name;
   const unsigned char *y = (const unsigned char *)((const A4_DIRENT *)b)->name;
   int dx = is_dot_name((const char *)x), dy = is_dot_name((const char *)y);
   if (dx || dy) {
      if (dx && dy)
         return (int)strlen((const char *)x) - (int)strlen((const char *)y);
      return dx ? -1 : 1;
   }
   for (;; x++, y++) {
      int cx = (*x < 128) ? SDL_toupper(*x) : *x;
      int cy = (*y < 128) ? SDL_toupper(*y) : *y;
      if (cx != cy || !cx)
         return cx - cy;
   }
}

static void dirlist_free(A4_DIRLIST *l)
{
   int i;
   for (i = 0; i < l->n; i++)
      free(l->e[i].name);
   free(l->e);
   l->e = NULL;
   l->n = l->cap = 0;
}

/* Lists the entries matching `pattern` (wildcards allowed in the last
 * component) over the user root and the asset roots.  Returns 0 when the
 * directory exists in no root. */
static int list_matches(const char *pattern, A4_DIRLIST *l)
{
   char dirpart[1024], physdir[2048];
   const char *namepat = get_filename(pattern);
   size_t dlen = (size_t)(namepat - pattern);
   const char *roots[8];
   int nroots = 0, i, found = 0;

   memset(l, 0, sizeof(*l));
   if (dlen >= sizeof(dirpart) || !*namepat)
      return 0;
   memcpy(dirpart, pattern, dlen);
   dirpart[dlen] = 0;
   l->pattern = namepat;

   if (!platform_ready() || plat_is_absolute(dirpart)) {
      roots[nroots++] = "";
   } else {
      roots[nroots++] = plat_user_dir();
      for (i = 0; i < plat_asset_dir_count() && nroots < 8; i++)
         roots[nroots++] = plat_asset_dir(i);
   }

   for (i = 0; i < nroots; i++) {
      SDL_PathInfo info;
      join_path(physdir, sizeof(physdir), roots[i], dirpart);
      if (!physdir[0])
         SDL_strlcpy(physdir, "./", sizeof(physdir));
      if (!SDL_GetPathInfo(physdir, &info) || info.type != SDL_PATHTYPE_DIRECTORY)
         continue;
      if (!found) {
         /* "." and ".." as returned by FindFirstFile */
         char full[2100];
         int exists, attrib;
         if (name_matches(namepat, ".")) {
            SDL_snprintf(full, sizeof(full), "%s.", physdir);
            attrib = path_attrib(full, ".", &exists);
            dirlist_add(l, ".", exists ? attrib : FA_DIREC);
         }
         if (name_matches(namepat, "..")) {
            SDL_snprintf(full, sizeof(full), "%s..", physdir);
            attrib = path_attrib(full, "..", &exists);
            dirlist_add(l, "..", exists ? attrib : FA_DIREC);
         }
      }
      found = 1;
      l->physdir = physdir;
      SDL_EnumerateDirectory(physdir, enum_cb, l);
   }
   l->physdir = NULL;
   if (l->n > 1)
      qsort(l->e, (size_t)l->n, sizeof(A4_DIRENT), dirent_cmp);
   return found;
}

/* attribute mask test of al_findfirst()/al_findnext() on Windows */
static int attrib_allowed(int attrib, int mask)
{
   return (attrib & ~(mask | (int)0xFFFFFF00)) == 0;
}

int file_exists(const char *filename, int attrib, int *aret)
{
   char path[2048];
   int a, exists;

   if (!filename)
      return FALSE;

   if (strchr(filename, '#')) {
      PACKFILE *f = pack_fopen_special_file(filename, F_READ);
      if (f) {
         pack_fclose(f);
         if (aret)
            *aret = FA_DAT_FLAGS;
         return ((attrib & FA_DAT_FLAGS) == FA_DAT_FLAGS) ? TRUE : FALSE;
      }
   }

   set_errno(0);
   if (has_wildcards(get_filename(filename))) {
      A4_DIRLIST l;
      int i, ok = FALSE;
      list_matches(filename, &l);
      for (i = 0; i < l.n; i++) {
         if (attrib_allowed(l.e[i].attrib, attrib)) {
            if (aret)
               *aret = l.e[i].attrib;
            ok = TRUE;
            break;
         }
      }
      dirlist_free(&l);
      return ok;
   }

   /* FindFirstFile fails for an empty name or a trailing separator */
   if (!*get_filename(filename))
      return FALSE;
   if (!plat_resolve_read(filename, path, sizeof(path)))
      return FALSE;
   a = path_attrib(path, get_filename(filename), &exists);
   if (!exists || !attrib_allowed(a, attrib))
      return FALSE;
   if (aret)
      *aret = a;
   return TRUE;
}

int exists(const char *filename)
{
   return file_exists(filename, FA_ARCH | FA_RDONLY, NULL);
}

int64_t file_size_ex(const char *filename)
{
   char path[2048];
   SDL_PathInfo info;

   if (!filename)
      return 0;
   if (strchr(filename, '#')) {
      PACKFILE *f = pack_fopen_special_file(filename, F_READ);
      if (f) {
         long ret = f->todo;
         pack_fclose(f);
         return ret;
      }
   }
   if (!plat_resolve_read(filename, path, sizeof(path)) ||
       !SDL_GetPathInfo(path, &info) || info.type == SDL_PATHTYPE_NONE) {
      set_errno(ENOENT);
      return 0;
   }
   if (info.type != SDL_PATHTYPE_FILE)
      return 0;
   return (int64_t)info.size;
}

int delete_file(const char *filename)
{
   char path[2048];
   SDL_PathInfo info;

   if (!filename)
      return -1;
   /* deletions only ever touch the writable user root */
   if (!platform_ready() || plat_is_absolute(filename))
      SDL_strlcpy(path, filename, sizeof(path));
   else
      join_path(path, sizeof(path), plat_user_dir(), filename);
   if (!SDL_GetPathInfo(path, &info) || info.type == SDL_PATHTYPE_NONE) {
      set_errno(ENOENT);
      return -1;
   }
   if (info.type == SDL_PATHTYPE_DIRECTORY) {
      set_errno(EACCES);   /* unlink() does not remove directories */
      return -1;
   }
   if (!SDL_RemovePath(path)) {
      set_errno(EACCES);
      return -1;
   }
   return 0;
}

int for_each_file_ex(const char *name, int in_attrib, int out_attrib,
                     int (*callback)(const char *filename, int attrib, void *param),
                     void *param)
{
   char buf[1024];
   A4_DIRLIST l;
   int i, ret, c = 0;
   int mask = ~out_attrib;

   if (!name || !callback)
      return 0;
   set_errno(0);
   list_matches(name, &l);
   for (i = 0; i < l.n; i++) {
      int a = l.e[i].attrib;
      if (!attrib_allowed(a, mask))
         continue;
      if ((~a & in_attrib) == 0) {
         replace_filename(buf, name, l.e[i].name, sizeof(buf));
         ret = callback(buf, a, param);
         if (ret != 0)
            break;
         c++;
      }
   }
   dirlist_free(&l);
   if (get_errno() == ENOENT)
      set_errno(0);
   return c;
}

/* ================================================================== */
/* config files (config.c)                                            */
/* ================================================================== */

typedef struct CONFIG_ENTRY {
   char *name;                  /* NULL for comments and blank lines */
   char *data;
   struct CONFIG_ENTRY *next;
} CONFIG_ENTRY;

typedef struct CONFIG {
   CONFIG_ENTRY *head;
   char *filename;
} CONFIG;

static CONFIG *config0;
static int config_default_tried;

static void destroy_config(CONFIG *cfg)
{
   CONFIG_ENTRY *pos, *prev;
   if (!cfg)
      return;
   free(cfg->filename);
   pos = cfg->head;
   while (pos) {
      prev = pos;
      pos = pos->next;
      free(prev->name);
      free(prev->data);
      free(prev);
   }
   free(cfg);
}

static char *ustrdup(const char *s)
{
   size_t n = (size_t)a4_ustrsize(s) + 1;
   char *d = (char *)malloc(n);
   if (d)
      a4_ustrzcpy(d, (int)n, s);
   return d;
}

/* get_line (config.c) */
static int get_line(const char *data, int length, char **name, char **val)
{
   char *buf;
   int buf_size = 256;
   int inpos = 0, outpos = 0, i, j, c, c2;

   buf = (char *)malloc((size_t)buf_size);
   if (!buf)
      return -1;

   while (inpos < length) {
      c = a4_ugetc(data + inpos);
      if ((c == '\r') || (c == '\n')) {
         inpos += a4_uwidth(data + inpos);
         if (inpos < length) {
            c2 = a4_ugetc(data + inpos);
            if (((c == '\r') && (c2 == '\n')) || ((c == '\n') && (c2 == '\r')))
               inpos += a4_uwidth(data + inpos);
         }
         break;
      }
      if (outpos >= buf_size - 8) {
         char *nb;
         buf_size *= 2;
         nb = (char *)realloc(buf, (size_t)buf_size);
         if (!nb) {
            free(buf);
            return -1;
         }
         buf = nb;
      }
      outpos += a4_usetc(buf + outpos, c);
      inpos += a4_uwidth(data + inpos);
   }
   a4_usetc(buf + outpos, 0);

   /* skip leading spaces */
   i = 0;
   c = a4_ugetc(buf);
   while ((c) && (a4_uisspace(c))) {
      i += a4_uwidth(buf + i);
      c = a4_ugetc(buf + i);
   }

   /* name */
   j = 0;
   while ((c) && (!a4_uisspace(c)) && (c != '=') && (c != '#')) {
      j += a4_ucwidth(c);
      i += a4_uwidth(buf + i);
      c = a4_ugetc(buf + i);
   }

   if (j) {
      *name = (char *)malloc((size_t)j + 1);
      if (!*name) {
         free(buf);
         return -1;
      }
      a4_ustrzcpy(*name, j + 1, buf + i - j);
      while ((c) && ((a4_uisspace(c)) || (c == '='))) {
         i += a4_uwidth(buf + i);
         c = a4_ugetc(buf + i);
      }
      *val = ustrdup(buf + i);
      if (!*val) {
         free(*name);
         free(buf);
         return -1;
      }
      /* strip trailing spaces (usetat(val, i, 0) truncates) */
      i = a4_ustrlen(*val) - 1;
      while ((i >= 0) && (a4_uisspace(a4_ugetat(*val, i)))) {
         (*val)[a4_uoffset(*val, i)] = 0;
         i--;
      }
   } else {
      *name = NULL;
      *val = ustrdup(buf);
      if (!*val) {
         free(buf);
         return -1;
      }
   }
   free(buf);
   return inpos;
}

static CONFIG *set_config(const char *data, int length, const char *filename)
{
   CONFIG *cfg;
   CONFIG_ENTRY **prev, *p;
   char *name, *val;
   int ret, pos;

   cfg = (CONFIG *)calloc(1, sizeof(CONFIG));
   if (!cfg)
      return NULL;
   cfg->filename = filename ? dup_str(filename) : NULL;
   prev = &cfg->head;
   pos = 0;
   while (pos < length) {
      ret = get_line(data + pos, length - pos, &name, &val);
      if (ret < 0)
         break;
      pos += ret;
      p = (CONFIG_ENTRY *)malloc(sizeof(CONFIG_ENTRY));
      if (!p) {
         free(name);
         free(val);
         break;
      }
      p->name = name;
      p->data = val;
      p->next = NULL;
      *prev = p;
      prev = &p->next;
   }
   return cfg;
}

/* load_config_file (config.c): the file is read through pack_fopen("r"),
 * so the current packfile password applies, exactly like Allegro. */
void set_config_file(const char *filename)
{
   int64_t length;
   CONFIG *cfg = NULL;

   if (!filename)
      return;
   destroy_config(config0);
   config0 = NULL;
   config_default_tried = TRUE;

   length = file_size_ex(filename);
   if (length > 0 && length < INT_MAX) {
      PACKFILE *f = pack_fopen(filename, F_READ);
      if (f) {
         char *tmp = (char *)malloc((size_t)length + 1);
         if (tmp) {
            pack_fread(tmp, (long)length, f);
            tmp[length] = 0;
            cfg = set_config(tmp, (int)length, filename);
            free(tmp);
         }
         pack_fclose(f);
      }
   }
   if (!cfg)
      cfg = set_config(NULL, 0, filename);
   config0 = cfg;
}

/* init_config(TRUE): Allegro loads allegro.cfg from the program directory
 * when no config file has been selected */
static void init_config_default(void)
{
   char filename[1024];
   if (config0 || config_default_tried)
      return;
   config_default_tried = TRUE;
   get_executable_name(filename, sizeof(filename));
   *get_filename(filename) = 0;
   a4_ustrzcat(filename, sizeof(filename), "allegro.cfg");
   set_config_file(filename);
}

static void prettify_section_name(const char *in, char *out, int out_size)
{
   int p;
   if ((in) && (a4_ustrlen(in))) {
      if (a4_ugetc(in) != '[') {
         p = a4_usetc(out, '[');
         a4_usetc(out + p, 0);
      } else {
         a4_usetc(out, 0);
      }
      a4_ustrzcat(out, out_size - a4_ucwidth(']'), in);
      out += a4_uoffset(out, -1);
      if (a4_ugetc(out) != ']') {
         out += a4_uwidth(out);
         out += a4_usetc(out, ']');
         a4_usetc(out, 0);
      }
   } else {
      a4_usetc(out, 0);
   }
}

static CONFIG_ENTRY *find_config_string(CONFIG *cfg, const char *section, const char *name)
{
   CONFIG_ENTRY *p;
   int in_section;

   if (!cfg)
      return NULL;
   p = cfg->head;
   in_section = (section && a4_ugetc(section)) ? FALSE : TRUE;
   while (p) {
      if (p->name) {
         if ((section) && (a4_ugetc(p->name) == '[') && (a4_ugetat(p->name, -1) == ']'))
            in_section = (a4_ustricmp(section, p->name) == 0);
         if ((in_section) || (a4_ugetc(name) == '[')) {
            if (a4_ustricmp(p->name, name) == 0)
               return p;
         }
      }
      p = p->next;
   }
   return NULL;
}

const char *get_config_string(const char *section, const char *name, const char *def)
{
   char section_name[256];
   CONFIG_ENTRY *p = NULL;

   if (!name)
      return def;
   init_config_default();
   prettify_section_name(section, section_name, sizeof(section_name));

   /* names starting with '#' live in the (empty) system config */
   if (!((a4_ugetc(name) == '#') ||
         ((a4_ugetc(section_name) == '[') && (a4_ugetat(section_name, 1) == '#'))))
      p = find_config_string(config0, section_name, name);

   if (p && p->data && (a4_ustrlen(p->data) != 0))
      return p->data;
   return def;
}

int get_config_int(const char *section, const char *name, int def)
{
   char section_name[256];
   const char *s;

   prettify_section_name(section, section_name, sizeof(section_name));
   s = get_config_string(section_name, name, NULL);
   if ((s) && (a4_ugetc(s))) {
      /* ustrtol(): ASCII copy (64 bytes) + strtol with a 32-bit long */
      char tmp[64];
      long long v;
      a4_utoascii(s, tmp, sizeof(tmp));
      v = strtoll(tmp, NULL, 0);
      if (v > INT32_MAX)
         v = INT32_MAX;
      if (v < INT32_MIN)
         v = INT32_MIN;
      return (int)v;
   }
   return def;
}

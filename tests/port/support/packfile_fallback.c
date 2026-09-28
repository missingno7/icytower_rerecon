/*
 * TEST SCAFFOLDING ONLY.  A minimal read-only stdio PACKFILE with exactly the
 * calls the sample loaders in a4_sound.c use, so the sound tests link while
 * compat/allegro4-sdl3/src/a4_file.c (the real PACKFILE implementation,
 * owned by the file worker) does not exist yet.  CMakeLists.txt adds this
 * file only when a4_file.c is absent; once it exists the tests use it.
 * Semantics follow Allegro 4.4.1 src/file.c for these calls.
 */
#include <stdio.h>
#include <stdlib.h>
#include <allegro.h>

struct PACKFILE { FILE *fp; };

PACKFILE *pack_fopen(const char *filename, const char *mode)
{
   PACKFILE *f;
   FILE *fp;
   if (!filename || !mode || (mode[0] != 'r' && mode[0] != 'R'))
      return NULL;
   fp = fopen(filename, "rb");
   if (!fp)
      return NULL;
   f = (PACKFILE *)malloc(sizeof(PACKFILE));
   if (!f) {
      fclose(fp);
      return NULL;
   }
   f->fp = fp;
   return f;
}

int pack_fclose(PACKFILE *f)
{
   if (!f)
      return 0;
   fclose(f->fp);
   free(f);
   return 0;
}

long pack_fread(void *p, long n, PACKFILE *f)
{
   if (n <= 0)
      return 0;
   return (long)fread(p, 1, (size_t)n, f->fp);
}

int pack_getc(PACKFILE *f)
{
   return getc(f->fp);
}

int pack_igetw(PACKFILE *f)
{
   int b1, b2;
   if ((b1 = pack_getc(f)) != EOF)
      if ((b2 = pack_getc(f)) != EOF)
         return ((b2 << 8) | b1);
   return EOF;
}

long pack_igetl(PACKFILE *f)
{
   int b1, b2, b3, b4;
   if ((b1 = pack_getc(f)) != EOF)
      if ((b2 = pack_getc(f)) != EOF)
         if ((b3 = pack_getc(f)) != EOF)
            if ((b4 = pack_getc(f)) != EOF)
               return (long)(((unsigned long)b4 << 24) | ((unsigned long)b3 << 16) |
                             ((unsigned long)b2 << 8) | (unsigned long)b1);
   return EOF;
}

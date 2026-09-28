/*
 * Win32 helpers for a4_file.c.  Kept in a separate unit because
 * <windows.h> clashes with allegro.h (BITMAP, key, ...).
 */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Attributes as _findfirst() reports them to Allegro's al_findfirst():
 * the Win32 attribute word, with FILE_ATTRIBUTE_NORMAL reported as 0. */
int a4_win32_file_attrib(const char *utf8_path, int *exists)
{
   wchar_t w[2048];
   DWORD a;
   *exists = 0;
   if (MultiByteToWideChar(CP_UTF8, 0, utf8_path, -1, w, 2048) <= 0)
      return 0;
   a = GetFileAttributesW(w);
   if (a == INVALID_FILE_ATTRIBUTES)
      return 0;
   *exists = 1;
   if (a == FILE_ATTRIBUTE_NORMAL)
      a = 0;
   return (int)a;
}

int a4_win32_executable_name(char *out, int size)
{
   wchar_t w[1024];
   DWORD n = GetModuleFileNameW(NULL, w, 1024);
   if (n == 0 || n >= 1024)
      return 0;
   return WideCharToMultiByte(CP_UTF8, 0, w, -1, out, size, NULL, NULL) > 0;
}
#else
typedef int a4_file_win32_unused;
#endif

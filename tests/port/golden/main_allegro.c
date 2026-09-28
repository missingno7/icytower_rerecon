/* Golden runner linked against real Allegro 4.4.1 (historical GCC 4.4 toolchain). */
#include <stdlib.h>
#include <string.h>
#include "golden.h"

int main(int argc, char **argv)
{
   golden_ctx g;
   if (argc < 3) { fprintf(stderr, "usage: %s DATA_DIR OUT_FILE [area]\n", argv[0]); return 2; }
   if (install_allegro(SYSTEM_NONE, &errno, atexit) != 0) return 1;
   /* SYSTEM_NONE keeps Allegro's generic pixel layouts (red at bit 0); the
    * game ran under the Windows driver, whose set_gdi_color_format()
    * (src/win/gdi.c) selects the layouts the compat layer implements. */
   _rgb_r_shift_15 = 10; _rgb_g_shift_15 = 5; _rgb_b_shift_15 = 0;
   _rgb_r_shift_16 = 11; _rgb_g_shift_16 = 5; _rgb_b_shift_16 = 0;
   _rgb_r_shift_24 = 16; _rgb_g_shift_24 = 8; _rgb_b_shift_24 = 0;
   _rgb_r_shift_32 = 16; _rgb_g_shift_32 = 8; _rgb_b_shift_32 = 0;
   _rgb_a_shift_32 = 24;
   g.data_dir = argv[1];
   g.out = fopen(argv[2], "wb");
   if (!g.out) return 1;
   if (argc < 4 || !strcmp(argv[3], "gfx")) scen_gfx(&g);
   if (argc < 4 || !strcmp(argv[3], "text")) scen_text(&g);
   if (argc < 4 || !strcmp(argv[3], "data")) scen_data(&g);
   if (argc < 4 || !strcmp(argv[3], "sound")) scen_sound(&g);
   fclose(g.out);
   return 0;
}

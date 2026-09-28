/* Golden runner linked against compat/allegro4-sdl3 (modern toolchain). */
#include <stdlib.h>
#include <string.h>
#include "golden.h"

int main(int argc, char **argv)
{
   golden_ctx g;
   if (argc < 3) { fprintf(stderr, "usage: %s DATA_DIR OUT_FILE [area]\n", argv[0]); return 2; }
   if (allegro_init() != 0) return 1;
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

#ifndef ICYTOWER_CUSTOM_H
#define ICYTOWER_CUSTOM_H
#include <allegro.h>
#include "recovered/Tcustom.h"
extern RGB black, pink;
void custom_alert(char *txt1, char *txt2);
char *get_string_data(char *key, char *string);
BITMAP *load_character_bmp(const char *name, int *uses_datafile, RGB *pal);
int init_custom(Tcustom *c, const char *name, int uses_datafile);
int load_frames(Tcustom *c);
void clear_trailing_whitespace(char *data);
SAMPLE *loadCustomSoundDF(DATAFILE *df, int id);
SAMPLE *loadCustomSoundFILE(char *tag, char *filename);
int load_sounds(Tcustom *c);
int destroy_custom_data(Tcustom *c);
#endif

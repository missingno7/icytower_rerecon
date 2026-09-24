/* Original cross-CU interfaces, recovered from DWARF. Implementations remain
 * in their historical CUs; this header introduces no substitute behavior. */
#ifndef ICYTOWER_GAME_SERVICES_H
#define ICYTOWER_GAME_SERVICES_H
#include <stddef.h>
#include <allegro.h>
#include "directories.h"
void log2file(const char *format, ...);
SAMPLE *getSampleFromOggDatafile(DATAFILE *df, int id);
SAMPLE *logg_load(const char *filename);
SAMPLE *logg_load_memory(void *pData, size_t iSize);
int new_rand(void);
void new_srand(int s);
#endif

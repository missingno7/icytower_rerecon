#ifndef ICYTOWER_MAP_H
#define ICYTOWER_MAP_H

#include "recovered/Tfloor.h"

#include "recovered/Tmap.h"

void reset_map(Tmap *m);
int is_solid(Tmap *m, int cx, int cy);
int get_level(Tmap *m, int cy);
void getFloorData(Tmap *m, int cy, int *fy, int *fx1, int *fx2);
void add_floor(Tmap *m);
#endif

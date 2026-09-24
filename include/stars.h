#ifndef ICYTOWER_STARS_H
#define ICYTOWER_STARS_H
#include <allegro.h>

#include "recovered/Tstar.h"

#include "recovered/Tstar_field.h"

void init_star_field(Tstar_field *sf, int w, int h, int num, int first_col,
                     int last_col, int dep, int cc);
void draw_star_field(Tstar_field *sf, BITMAP *bmp, int x, int y);
void scroll_star_field(Tstar_field *sf, double xstep, double ystep);
#endif

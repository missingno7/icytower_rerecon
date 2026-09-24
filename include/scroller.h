#ifndef ICYTOWER_SCROLLER_H
#define ICYTOWER_SCROLLER_H

#include <allegro.h>

#include "recovered/Tscroller.h"

void init_scroller(Tscroller *sc, FONT *f, char *t, int w, int h, int horiz);
int draw_scroller(Tscroller *sc, BITMAP *bmp, int x, int y, int color);
void scroll_scroller(Tscroller *sc, int step);
void restart_scroller(Tscroller *sc);
#endif

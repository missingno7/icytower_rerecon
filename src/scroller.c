#include <string.h>
#include <allegro.h>
#include "scroller.h"

void init_scroller(Tscroller *sc, FONT *f, char *t, int w, int h, int horiz)
{
    sc->fnt = f;
    sc->font_height = text_height(f);
    sc->height = h;
    sc->horizontal = horiz;
    sc->text = t;
    sc->width = w;
    if (horiz) {
        sc->length = text_length(sc->fnt, t);
        sc->offset = sc->width;
        return;
    }
    else {
    int i;
    int len;
    len = (int)strlen(t);
    sc->lines[0] = t;
    sc->rows = 1;
    for (i = 0; i < len; i++) {
        if (sc->text[i] == '\n' && sc->rows <= 511) {
            sc->lines[sc->rows] = sc->text + i + 1;
            sc->rows++;
            sc->text[i] = '\0';
        }
    }
    sc->offset = sc->height;
    }
}

int draw_scroller(Tscroller *sc, BITMAP *bmp, int x, int y, int color)
{
    if (sc->horizontal) {
        if (sc->offset < -sc->length || sc->offset > sc->width)
            return 0;
        set_clip_rect(bmp, x, y, x + sc->width, y + sc->height);
        textout_ex(bmp, sc->fnt, sc->text, x + sc->offset, y, color, -1);
    }
    else
    {

        int i;
        if (sc->offset < -sc->rows * sc->font_height || sc->offset > sc->height) return 0;
        set_clip_rect(bmp, x, y, x + sc->width, y + sc->height);
        for (i = 0; i < sc->rows; i++) {
            if (i * sc->font_height + sc->offset > sc->height)
                continue;
            if ((i + 1) * sc->font_height + sc->offset < 0)
                continue;
            textout_centre_ex(bmp, sc->fnt, sc->lines[i],
                              x + (sc->width >> 1),
                              y + i * sc->font_height + sc->offset, color, -1);
        }
    }
    set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);
    return -1;
}

void scroll_scroller(Tscroller *sc, int step)
{
    sc->offset += step;
}

void restart_scroller(Tscroller *sc)
{
    if (sc->horizontal)
        sc->offset = sc->width;
    else
        sc->offset = sc->height;
}

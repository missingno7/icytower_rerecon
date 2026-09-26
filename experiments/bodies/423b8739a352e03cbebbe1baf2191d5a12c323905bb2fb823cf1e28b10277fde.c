{
    int clip_right;
    int clip_bottom;

    if (sc->horizontal) {
        if (sc->offset < -sc->length || sc->offset > sc->width)
            return 0;
        clip_right = x + sc->width;
        clip_bottom = y + sc->height;
        set_clip_rect(bmp, x, y, clip_right, clip_bottom);
        textout_ex(bmp, sc->fnt, sc->text, x + sc->offset, y, color, -1);
    } else {
        int i;

        if (sc->offset < -sc->rows * sc->font_height || sc->offset > sc->height)
            return 0;
        clip_right = x + sc->width;
        clip_bottom = y + sc->height;
        set_clip_rect(bmp, x, y, clip_right, clip_bottom);
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
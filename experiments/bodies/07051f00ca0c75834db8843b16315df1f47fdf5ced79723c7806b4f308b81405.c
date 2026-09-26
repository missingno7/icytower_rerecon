{
    int pos;
    char str[256];
    int h;
    int y_offset;
    int y_row;

    stepIn = dx;
    h = mp->font_height - 12;
    y_offset = h;
    y_row = y;
    pos = -1;
    do {
        int x;

        pos++;
        x = cx + stepIn * pos;
        build_menu_string(m, str);
        if (m->flags & 64) {
            char key_str[32];

            key_to_str(*(int *)m->data, key_str);
            textprintf_ex(bmp, (FONT *)mp->font, x, y_row, -1, -1, "%s:", m->caption);
            textprintf_ex(bmp, (FONT *)mp->font, x + 101, y_row, -1, -1, "%s", key_str);
        }
        if (m->flags & 1)
            draw_sprite(bmp, (BITMAP *)mp->bullet,
                        x - 3 - ((BITMAP *)mp->bullet)->w, y_row - 3);
        textout_ex(bmp, (FONT *)mp->font, str, x, y_row, -1, -1);
        if (m->flags & 16) {
            DATAFILE *assets;

            assets = (DATAFILE *)mp->data + mp->fo;
            draw_sprite(bmp, (BITMAP *)assets[0].dat, x + 215, y_row + 10);
            draw_sprite(bmp, (BITMAP *)assets[1].dat, x + 236, y_row + 10);
            draw_sprite(bmp, (BITMAP *)assets[2].dat, x + 252, y_row + 10);
        }
        if (m->flags & 32) {
            BITMAP *b;

            b = ((BITMAP **)m->data)[2];
            draw_sprite(bmp, b, x + 244 - b->w / 2, y - b->h + y_offset + 10);
        }
        m++;
        y_row += h;
        y_offset += h;
    } while ((signed char)m[-1].flags >= 0);
}
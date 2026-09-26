{
    int pos;
    char str[256];
    int h;
    int y_offset;
    int y_row;
    Tmenu *row;

    stepIn = dx;
    h = mp->font_height - 12;
    y_row = y;
    y_offset = h;
    row = m;
    pos = -1;
    draw_menu_rows:
    {
        int x;

        pos++;
        x = cx + stepIn * pos;
        build_menu_string(row, str);
        if (row->flags & 64) {
            char key_str[32];

            key_to_str(*(int *)row->data, key_str);
            textprintf_ex(bmp, (FONT *)mp->font, x, y_row, -1, -1, "%s:", row->caption);
            textprintf_ex(bmp, (FONT *)mp->font, x + 101, y_row, -1, -1, "%s", key_str);
            if (row->flags & 1)
                draw_sprite(bmp, (BITMAP *)mp->bullet,
                            x - 3 - ((BITMAP *)mp->bullet)->w, y_row - 3);
        } else {
            if (row->flags & 1)
                draw_sprite(bmp, (BITMAP *)mp->bullet,
                            x - 3 - ((BITMAP *)mp->bullet)->w, y_row - 3);
            textout_ex(bmp, (FONT *)mp->font, str, x, y_row, -1, -1);
        }
        if (row->flags & 16) {
            draw_sprite(bmp, (BITMAP *)((DATAFILE *)mp->data)[mp->fo + 0].dat,
                        x + 215, y_row + 10);
            draw_sprite(bmp, (BITMAP *)((DATAFILE *)mp->data)[mp->fo + 1].dat,
                        x + 236, y_row + 10);
            draw_sprite(bmp, (BITMAP *)((DATAFILE *)mp->data)[mp->fo + 2].dat,
                        x + 252, y_row + 10);
        }
        if (row->flags & 32) {
            BITMAP *b;

            b = ((BITMAP **)m[pos].data)[2];
            draw_sprite(bmp, b, x + 244 - b->w / 2, y - b->h + y_offset + 10);
        }
    }
        row++;
        y_offset += h;
        y_row += h;
        if ((signed char)row[-1].flags < 0)
            return;
        goto draw_menu_rows;
}
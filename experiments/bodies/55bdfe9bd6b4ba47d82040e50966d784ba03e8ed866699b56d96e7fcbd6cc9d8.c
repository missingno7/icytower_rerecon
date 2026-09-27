{
    stepIn = dx;
    int pos = -1;
    char str[256];
    int h = mp->font_height - 12;


    int nextpos = 1;
    do {
        int x;
        pos++;
        x = cx + stepIn * pos;

        int row_delta=pos*h;
        build_menu_string(&m[pos], str);

        if (m[pos].flags & 64) {
            char key_str[32];
            key_to_str(*(int *)m[pos].data, key_str);
            textprintf_ex(bmp, mp->font, x, y + row_delta, -1, -1, "%s:", m[pos].caption);
            textprintf_ex(bmp, mp->font, x + 101, y + row_delta, -1, -1, "%s", key_str);

            if (m[pos].flags & 1)
                draw_sprite(bmp, mp->bullet, x - 3 - mp->bullet->w, y + row_delta - 3);
        }
        else {
            if ((m[pos].flags & 1) || (signed char)(m[pos].flags & 1)>0)
                draw_sprite(bmp, mp->bullet, x - 3 - mp->bullet->w, y + row_delta - 3);



            textout_ex(bmp, mp->font, str, x, y + row_delta, -1, -1);
        }

        if (m[pos].flags & 16) {
            draw_sprite(bmp, (BITMAP *)mp->data[mp->fo].dat, x + 215, y + row_delta + 10);
            draw_sprite(bmp, (BITMAP *)mp->data[mp->fo + 1].dat, x + 236, y + row_delta + 10);
            draw_sprite(bmp, (BITMAP *)mp->data[mp->fo + 2].dat, x + 252, y + row_delta + 10);
        }

        if (m[pos].flags & 32) {
            BITMAP *b = ((BITMAP **)m->data)[2];
            draw_sprite(bmp, b, x + 244 - b->w / 2, y + nextpos * h - b->h + 10);
        }

        nextpos++;
    } while (!(m[pos].flags & 128));
}
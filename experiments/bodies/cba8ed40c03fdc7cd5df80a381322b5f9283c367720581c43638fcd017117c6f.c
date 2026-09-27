{
    stepIn = dx;
    int pos = 0;
    char str[256];
    int h = mp->font_height - 12;


    do {
        int x;
        x = cx + stepIn * pos;

        pos++;
        build_menu_string(&m[pos - 1], str);

        if (m[pos - 1].flags & 64) {
            char key_str[32];
            key_to_str(*(int *)m[pos - 1].data, key_str);
            textprintf_ex(bmp, mp->font, x, y + (pos - 1) * h, -1, -1, "%s:", m[pos - 1].caption);
            textprintf_ex(bmp, mp->font, x + 101, y + (pos - 1) * h, -1, -1, "%s", key_str);

            if (m[pos - 1].flags & 1)
                draw_sprite(bmp, mp->bullet, x - 3 - mp->bullet->w, y + (pos - 1) * h - 3);
        }
        else {
            if (m[pos - 1].flags & 1)
                draw_sprite(bmp, mp->bullet, x - 3 - mp->bullet->w, y + (pos - 1) * h - 3);



            textout_ex(bmp, mp->font, str, x, y + (pos - 1) * h, -1, -1);
        }

        if (m[pos - 1].flags & 16) {
            draw_sprite(bmp, (BITMAP *)mp->data[mp->fo].dat, x + 215, y + (pos - 1) * h + 10);
            draw_sprite(bmp, (BITMAP *)mp->data[mp->fo + 1].dat, x + 236, y + (pos - 1) * h + 10);
            draw_sprite(bmp, (BITMAP *)mp->data[mp->fo + 2].dat, x + 252, y + (pos - 1) * h + 10);
        }

        if (m[pos - 1].flags & 32) {
            BITMAP *b = ((BITMAP **)m->data)[2];
            draw_sprite(bmp, b, x + 244 - b->w / 2, y + pos * h - b->h + 10);
        }

    } while (!(m[pos - 1].flags & 128));
}
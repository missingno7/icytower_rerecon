{
    stepIn = dx;
    int pos = -1;
    char str[256];
    int h = mp->font_height - 12;


    do {
        pos++;
        int x = cx + stepIn * pos;

        build_menu_string(&m[pos], str);

        if (m[pos].flags & 64) {
            char key_str[32];
            key_to_str(*(int *)m[pos].data, key_str);
            textprintf_ex(bmp, mp->font, x, y + pos * h, -1, -1, "%s:", m[pos].caption);
            textprintf_ex(bmp, mp->font, x + 101, y + pos * h, -1, -1, "%s", key_str);

            if (m[pos].flags & 1)
                draw_sprite(bmp, mp->bullet, x - 3 - mp->bullet->w, y + pos * h - 3);
        }
        else {
            if (m[pos].flags & 1)
                draw_sprite(bmp, mp->bullet, x - 3 - mp->bullet->w, y + pos * h - 3);



            textout_ex(bmp, mp->font, str, x, y + pos * h, -1, -1);
        }

        if (m[pos].flags & 16) {
            draw_sprite(bmp, (BITMAP *)mp->data[mp->fo].dat, x + 215, y + pos * h + 10);
            draw_sprite(bmp, (BITMAP *)mp->data[mp->fo + 1].dat, x + 236, y + pos * h + 10);
            draw_sprite(bmp, (BITMAP *)mp->data[mp->fo + 2].dat, x + 252, y + pos * h + 10);
        }

        if (m[pos].flags & 32) {
            BITMAP *b = ((Tmenu_char_selection *)m->data)->bmp;
            draw_sprite(bmp, b, x + 244 - b->w / 2, y + (pos + 1) * h - b->h + 10);
        }


    } while (!(m[pos].flags & 128));
}
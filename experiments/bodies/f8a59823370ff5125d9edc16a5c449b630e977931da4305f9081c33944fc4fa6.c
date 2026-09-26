{
    BITMAP *block = create_bitmap(w, text_height(f) + 2);
    char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz 0123456789.!_";
    int tick = 0;
    int c;
    int i = strlen(string);


    if (!block)
        return -1;


    blit(bmp, block, pos_x - 1, pos_y - 1, 0, 0, block->w, block->h);

    while (key[KEY_ENTER] || key[KEY_SPACE]);
    clear_keybuf();


    while (!closeButtonClicked) {
        tick++;
        cycle_count = 0;

        checkMenuFocus();


        string[i] = (tick & 8) ? '|' : ' ';
        string[i + 1] = 0;
        vsync();
        blit(block, bmp, 0, 0, pos_x - 1, pos_y - 1, block->w, block->h);
        if (bg_color >= 0) rectfill(bmp, pos_x, pos_y, pos_x + block->w - 1, pos_y + block->h - 3, bg_color);
        textout_ex(bmp, f, string, pos_x + 2, pos_y, colour, -1);
        blit_to_screen(bmp);


        if (keypressed()) {
            c = readkey();
            switch (c >> 8) {
                case KEY_ESC:
                    string[i] = 0;
                    destroy_bitmap(block);
                    return -1;
                case KEY_TAB:
                case KEY_UP:
                case KEY_DOWN:

                    string[i] = 0;
                    destroy_bitmap(block);
                    return -2;


                case KEY_BACKSPACE:

                    if (--i < 0) i = 0;
                    break;

                case KEY_ENTER:
                    string[i] = 0;
                    destroy_bitmap(block);
                    return 0;


                default:
                    if (i < max_chars - 2 && strchr(letters, c)) {

                        if ((c >> 8) != KEY_SPACE || i) {

                            if (w - 9 > text_length(f, string)) {


                                string[i] = c;
                                i++;
                            }
                        }
                    }
            }
        }



        while (!cycle_count) rest(2);
    }


    destroy_bitmap(block);
    return 0;
}
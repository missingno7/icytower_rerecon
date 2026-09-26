{
    BITMAP *block = create_bitmap(w, text_height(f) + 2);
    char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz 0123456789.!_";
    int i = strlen(string);
    int tick;
    int c;

    if (!block)
        goto get_string_return_minus_one;
    blit(bmp, block, pos_x - 1, pos_y - 1, 0, 0, block->w, block->h);
    while (key[KEY_ENTER] || key[KEY_SPACE])
        ;
    clear_keybuf();
    tick = 0;
    goto get_string_first_iteration;
    for (;;) {
        while (!cycle_count)
            rest(2);
    get_string_first_iteration:
        switch (closeButtonClicked) {
        case 0:
            break;
        default:
            goto get_string_close_button;
        }
        tick++;
        cycle_count = 0;
        checkMenuFocus();
        string[i] = (tick & 8) ? '|' : ' ';
        string[i + 1] = 0;
        vsync();
        blit(block, bmp, 0, 0, pos_x - 1, pos_y - 1, block->w, block->h);
        if (bg_color >= 0)
            rectfill(bmp, pos_x, pos_y, pos_x + block->w - 1,
                     pos_y + block->h - 3, bg_color);
        textout_ex(bmp, f, string, pos_x + 2, pos_y, colour, -1);
        blit_to_screen(bmp);
        if (keypressed()) {
            c = readkey();
            switch (c >> 8) {
        case KEY_ESC:
            string[i] = 0;
            destroy_bitmap(block);
            goto get_string_return_minus_one;
        case KEY_TAB:
        case KEY_UP:
        case KEY_DOWN:
            string[i] = 0;
            destroy_bitmap(block);
            return -2;
        case KEY_ENTER:
            string[i] = 0;
            destroy_bitmap(block);
            return 0;
        case KEY_BACKSPACE:
            i--;
            if (i < 0)
                i = 0;
            break;
        default:
            if (i < max_chars - 2 && strchr(letters, c) &&
                ((c >> 8) != KEY_SPACE || i) &&
                w - 9 > text_length(f, string))
                string[i++] = (char)c;
            }
        }
        }
get_string_return_minus_one:
    return -1;
get_string_close_button:
    destroy_bitmap(block);
    return 0;
}
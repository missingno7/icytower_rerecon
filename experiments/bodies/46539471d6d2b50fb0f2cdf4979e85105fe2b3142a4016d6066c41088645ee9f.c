{
    double vol = options.msc_volume;
    double vol_step = vol / 150.0f;
    int gc;
    BITMAP *logoBMP;

    clear(swap_screen);
    logoBMP = data[125].dat;

    blit(data[126].dat, swap_screen, 0, 0, 0, 0, 640, 480);
    draw_sprite(swap_screen, logoBMP, 320 - logoBMP->w / 2, 10);

    textout_centre_ex(swap_screen, data[50].dat, "Thanks for playing!", 320, 280, -1, -1);
    textout_centre_ex(swap_screen, data[52].dat, "DESIGN & CODING: Johan Peitz", 320, 360, -1, -1);

    textout_centre_ex(swap_screen, data[52].dat, "GRAPHICS: Emanuel Garnheim", 320, 390, -1, -1);

    fadeIn(swap_screen, 16);

    closeButtonClicked = 0;
    cycle_count = 0;
    while (!closeButtonClicked && !key[KEY_ESC] && cycle_count <= 149) {
        gc = cycle_count;

        checkMenuFocus();

        if (bg_menu) adjust_sample(bg_menu, (int)vol, 128, 1000, 1);
        vol -= vol_step;
        while (gc == cycle_count) rest(2);
    }


    fadeOut(16);
}
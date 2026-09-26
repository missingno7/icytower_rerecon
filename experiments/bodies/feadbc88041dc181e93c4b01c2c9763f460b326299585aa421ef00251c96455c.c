{

    int i;
    BITMAP *bg = create_bitmap(SCREEN_W, SCREEN_H);
    blit(screen, bg, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

    clear_keybuf();
    while (is_any(get_controls()) || key[KEY_SPACE]) poll_control(get_controls(), 0);
    clear_keybuf();

    int listHeight = 0;
    int th, mh, bh;
    int bmpHeight;
    BITMAP *bmp;
    int yPos;
    int pageY, targetY, dark, targetDark;
    int done, canDone;

    for (i = 0; i < 15; i++)
        if (tables[i]->posts[0].value)
            listHeight = draw_table(0, 0, listHeight, names[i], tables[i]) + 18;




    th = ((BITMAP *)data[66].dat)->h;
    mh = ((BITMAP *)data[65].dat)->h;
    bh = ((BITMAP *)data[64].dat)->h;

    bmpHeight = listHeight / bh > 2 ? listHeight / bh - 1 : 2;



    bmp = create_bitmap(((BITMAP *)data[66].dat)->w, bh + th + bmpHeight * mh);
    clear_to_color(bmp, makecol(255, 0, 255));

    draw_sprite(bmp, (BITMAP *)data[66].dat, 0, 0);
    for (i = 0; i < bmpHeight; i++)
        draw_sprite(bmp, (BITMAP *)data[65].dat, 0, th + i * mh);

    draw_sprite(bmp, (BITMAP *)data[64].dat, 0, bmp->h - bh);



    yPos = 80;
    for (i = 0; i < 15; i++)
        if (tables[i]->posts[0].value)
            yPos = draw_table(bmp, 40, yPos, names[i], tables[i]) + 18;

    pageY = 500;
    targetY = 0;
    dark = 0;
    targetDark = 158;
    done = 0;
    canDone = 0;
    while (!closeButtonClicked && !done) {
        cycle_count = 0;

        checkMenuFocus();
        blit(bg, swap_screen, 0, 0, 0, 0, bg->w, bg->h);

        set_trans_blender(0, 0, 0, dark);
        drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
        rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, makecol(0, 0, 0));
        solid_mode();

        draw_sprite(swap_screen, bmp, 160, pageY);

        pageY = (int)((targetY - pageY) * 0.2 + pageY);
        dark = (int)((targetDark - dark) * 0.2 + dark);

        draw_sprite(swap_screen, (BITMAP *)data[9].dat, 658 - dark, 380);
        draw_sprite(swap_screen, (BITMAP *)data[6].dat, 658 - dark, 40);


        blit_to_screen(swap_screen);

        poll_control(get_controls(), 0);

        if (is_up(get_controls())) targetY = MIN(0, targetY + 16);
        if (is_down(get_controls())) targetY = MAX(470 - bmp->h, targetY - 16);

        done = is_fire(get_controls()) ? 1 : 0;
        if ((key[KEY_ESC] || key[KEY_ENTER] || key[KEY_SPACE]) && canDone) done = 1;
        if (!key[KEY_ESC] && !key[KEY_ENTER] && !key[KEY_SPACE]) canDone = 1;
        while (cycle_count <= 0) rest(2);
    }


    targetY = 500;
    targetDark = 0;
    while (pageY <= 480) {
        cycle_count = 0;

        checkMenuFocus();
        draw_sprite(swap_screen, bg, 0, 0);

        set_trans_blender(0, 0, 0, dark);
        drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
        rectfill(swap_screen, 0, 0, SCREEN_W, SCREEN_H, makecol(0, 0, 0));
        solid_mode();

        draw_sprite(swap_screen, bmp, 160, pageY);

        pageY = (int)((targetY - pageY) * 0.2 + pageY);
        dark = (int)((targetDark - dark) * 0.2 + dark);

        draw_sprite(swap_screen, (BITMAP *)data[9].dat, 658 - dark, 380);
        draw_sprite(swap_screen, (BITMAP *)data[6].dat, 658 - dark, 40);


        blit_to_screen(swap_screen);

        keypressed();
        while (cycle_count <= 0) rest(2);
    }



    clear_keybuf();



    destroy_bitmap(bmp);
    destroy_bitmap(bg);
}
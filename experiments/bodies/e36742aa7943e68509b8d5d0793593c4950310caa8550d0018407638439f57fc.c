{
    char *data_basic;
    char *data_advanced;
    char *data_general;
    int pageY;
    int targetY;
    void *bg;
    void *bmp;
    int y;
    int y1;
    int y2;
    char nextRankMessage[1024];
    char totalNextRankMessage[1024];
    int done;
    int rank;

    clear_keybuf();
    while (is_any(get_controls()) || key[KEY_SPACE])
        poll_control(get_controls(), 0);
    clear_keybuf();

    bg = create_bitmap(640, 480);
    draw_sprite(bg, screen, 0, 0);
    bmp = create_bitmap(((Tprofile_bitmap *)data[86].dat)->w + 50,
                        ((Tprofile_bitmap *)data[86].dat)->h + 50);
    clear_to_color(bmp, makecol(255, 0, 255));
    draw_sprite(bmp, data[86].dat, 0, 50);

    data_general = profile_data_page_general((Tprofile_general *)profile,
                                             "");
    sprintf(data_general,
            "%s-------------------------------------------------------------\n\n",
            data_general);
    data_basic = profile_data_page_basic((Tprofile_basic *)profile);
    data_advanced = profile_data_page_advanced((Tprofile_advanced *)profile);

    textprintf_ex(bmp, data[51].dat, 75, 10, -1, -1, "profile: %s",
                  (char *)profile + 6);
    y = draw_buffer(bmp, data_general, 85, 45);
    y1 = draw_buffer(bmp, data_basic, 85, y);
    y2 = draw_buffer(bmp, data_advanced, 290, y);
    if (y2 < y1)
        y2 = y1;
    draw_buffer(bmp,
                "-------------------------------------------------------------\n\n",
                85, y2);

    rank = get_rank_id((Tprofile_rank *)profile);
    draw_sprite(bmp, data[74 + rank].dat, 35,
                ((Tprofile_bitmap *)bmp)->h - 140);
    set_next_rank_message(nextRankMessage, (Tprofile_rank *)profile);
    if (strlen(nextRankMessage) > 1) {
        sprintf(totalNextRankMessage, "To reach next rank:%s\n",
                nextRankMessage);
        draw_buffer(bmp, totalNextRankMessage, 150,
                    ((Tprofile_bitmap *)bmp)->h - 120);
    }

    pageY = 500;
    targetY = 15;
    done = 0;
    while (!closeButtonClicked && !done) {
        cycle_count = 0;
        checkMenuFocus();
        draw_sprite(swap_screen, bg, 0, 0);
        set_trans_blender(0, 0, 0, (500 - pageY) / 3);
        drawing_mode(5, 0, 0, 0);
        rectfill(swap_screen, 0, 0, SCREEN_W,
                 SCREEN_H, makecol(0, 0, 0));
        solid_mode();
        draw_sprite(swap_screen, bmp, 70, pageY);
        blit_to_screen(swap_screen);
        poll_control(get_controls(), 0);
        done = is_fire(get_controls());
        if (keypressed())
            done = 1;
        while (cycle_count <= 0)
            rest(2);
        pageY += (targetY - pageY) * 0.2;
    }

    targetY = 500;
    while (pageY <= 480) {
        cycle_count = 0;
        checkMenuFocus();
        draw_sprite(swap_screen, bg, 0, 0);
        set_trans_blender(0, 0, 0, (500 - pageY) / 3);
        drawing_mode(5, 0, 0, 0);
        rectfill(swap_screen, 0, 0, SCREEN_W,
                 SCREEN_H, makecol(0, 0, 0));
        solid_mode();
        draw_sprite(swap_screen, bmp, 70, pageY);
        pageY += (targetY - pageY) * 0.2;
        blit_to_screen(swap_screen);
        while (cycle_count <= 0)
            rest(2);
    }

    free(data_basic);
    free(data_advanced);
    free(data_general);
    destroy_bitmap(bmp);
    destroy_bitmap(bg);
    clear_keybuf();
}
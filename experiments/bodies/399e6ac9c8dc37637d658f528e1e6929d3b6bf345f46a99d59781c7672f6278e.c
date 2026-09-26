{
    int fh = text_height(font);

    int fg = makecol(25, 25, 25);

    double view_percentage = MIN(1.0, (double)max_posts / numProfiles);
    double view_offset;
    int i;
    int profile_index;
    int row_y;
    int selected;
    char c;
    int right;



    draw_sprite(swap_screen, (BITMAP *)data[86].dat, x - 15, y - 15);


    set_trans_blender(0, 0, 0, 150);
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
    right = x + 300;
    rect(bmp, x + 5, y + 30, x + 265, y + 329, fg);
    rect(bmp, x + 255, y + 30, x + 265, y + 329, fg);

    view_offset = (1.0 - view_percentage) * 328 / (numProfiles - max_posts) * offset;
    rectfill(bmp, x + 257, y + 32 + view_offset, x + 263, y + view_percentage * 328 + view_offset, fg);
    solid_mode();


    textout_ex(bmp, (FONT *)data[51].dat, "SELECT PROFILE", x + 10, y - 12, -1, -1);

    draw_sprite(bmp, (BITMAP *)data[73].dat, x + 270, y + 24);


    set_clip_rect(bmp, x + 6, 0, x + 290, bmp->h - 1);
    for (i = 1; i <= max_posts; i++) {
        profile_index = offset + i - 1;
        if (profile_index >= numProfiles) break;
        row_y = y + fh * i + 32 - fh;


        c = profile_index ? '{' : '~';


        selected = 0;
        if (profile_index == selection) {
            drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
            set_trans_blender(0, 0, 0, 50);
            rectfill(bmp, x + 7, row_y, right - 47, row_y + fh - 1, fg);
            solid_mode();
            selected = 1;
        }


        textprintf_ex(bmp, font, x + 8, row_y, fg, -1, "%c %c %s %s", selected ? '>' : ' ', c, profiles[profile_index].handle, !stricmp(profiles[profile_index].handle, current_profile->handle) ? "(current)" : "");
    }

    set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);

}
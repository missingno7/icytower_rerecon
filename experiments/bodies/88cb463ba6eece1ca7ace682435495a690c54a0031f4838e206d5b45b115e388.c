{
    int fh = text_height(font);
    int i;
    int fg = makecol(25, 25, 25);

    double view_percentage = MIN(1.0, (double)max_posts / numProfiles);
    int width = 300;
    int height = 300;
    int scrollHeight = 328;
    double view_offset;



    draw_sprite(swap_screen, (BITMAP *)data[86].dat, x - 15, y - 15);


    set_trans_blender(0, 0, 0, 150);
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
    rect(bmp, x + 5, y + 30, x + width - 35, y + 29 + height, fg);


    rect(bmp, x + width - 45, y + 30, x + width - 35, y + 29 + height, fg);
    view_offset = (1.0 - view_percentage) * scrollHeight / (numProfiles - max_posts) * offset; rectfill(bmp, x + width - 43, y + 32 + view_offset, x + width - 37, y + view_percentage * scrollHeight + view_offset, fg);
    solid_mode();


    textout_ex(bmp, (FONT *)data[51].dat, "SELECT PROFILE", x + 10, y - 12, -1, -1);

    draw_sprite(bmp, (BITMAP *)data[73].dat, x + 270, y + 24);


    set_clip_rect(bmp, x + 6, 0, x + 290, bmp->h - 1);
    for (i = 0; i < max_posts; i++) {

        if (i + offset >= numProfiles) break;

        char icon;

        icon = (i + offset) ? '{' : '~';



        int selected = 0;
        if (i + offset == selection) {
            drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
            set_trans_blender(0, 0, 0, 50);
            rectfill(bmp, x + 7, y + i * fh + 32, x + width - 47, y + 32 + (i + 1) * fh - 1, fg);
            solid_mode(); selected = 1;
        }

        textprintf_ex(bmp, font, x + 8, y + i * fh + 32, fg, -1, "%c %c %s %s", selected ? '>' : ' ', icon, profiles[i + offset].handle, !stricmp(profiles[i + offset].handle, current_profile->handle) ? "(current)" : "");
    }
    set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);

}
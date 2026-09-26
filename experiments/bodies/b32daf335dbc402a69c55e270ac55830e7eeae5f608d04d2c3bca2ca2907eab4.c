{
    int w = 310;
    int h = 305;
    int fh = text_height(font);
    int i;
    char *curr_filename = NULL;
    int fg = makecol(25, 25, 25);
    int mg = makecol(85, 85, 85);
    double view_percentage;
    double view_offset;
    int show_directory = 0;
    int selected_version = 0;
    int isCustom = 0;

    i = num_itr_files;
    view_percentage = (double)max_posts / i;
    if (view_percentage > 1.0)
        view_percentage = 1.0;

    draw_sprite(bmp, data[86].dat, x - 15, y - 15);
    set_trans_blender(0, 0, 0, 150);
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
    rect(bmp, x + 5, y + 30, x + 305, y + 300, fg);
    rect(bmp, x + 295, y + 30, x + 305, y + 300, fg);
    rectfill(bmp, x + 297, y + 32 + (1.0 - view_percentage) * 266.0 / (i - max_posts) * offset, x + 303, y + 32 + 266.0 * view_percentage + (1.0 - view_percentage) * 266.0 / (i - max_posts) * offset, fg);
    w = 400;
    h = 400;
    rect(bmp, x + 5, y + 310, x + w - 6, y + h - 6, fg);
    solid_mode();

    textout_ex(bmp, data[51].dat, "SELECT REPLAY", x + 10, y - 12, -1, -1);
    draw_sprite(bmp, data[89].dat, x + 310, y + 34);
    draw_sprite(bmp, data[112].dat, x + 311, y + 260);
    switch (sort_method) {
    case 1:
        draw_sprite(bmp, data[115].dat, x + 321, y + 274);
        break;
    case 4:
        draw_sprite(bmp, data[114].dat, x + 321, y + 286);
        break;
    case 2:
        draw_sprite(bmp, data[116].dat, x + 355, y + 274);
        break;
    case 3:
        draw_sprite(bmp, data[113].dat, x + 355, y + 286);
        break;
    }

    set_clip_rect(bmp, x + 6, 0, x + 290, bmp->h - 1);
    for (i = 0; i < max_posts; i++) {
        char name[1024];
        int is_dir;

        if (i + offset >= num_itr_files)
            break;

        if (file_list[i + offset].parent)
            strcpy(name, ".. (parent directory)");
        else
            strcpy(name, get_filename(file_list[i + offset].full_path));

        is_dir = file_list[i + offset].directory;

        if (i + offset != selection)
            textprintf_ex(bmp, font, x + 8, y + 32 + i * fh, is_dir ? mg : fg, -1, "  %c %s", is_dir ? '}' : '{', name);
        else {
            set_trans_blender(0, 0, 0, 50);
            drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
            rectfill(bmp, x + 7, y + 32 + i * fh, x + 293, y + 32 + (i + 1) * fh - 1, fg);
            solid_mode();
            textprintf_ex(bmp, font, x + 8, y + 32 + i * fh, mg, -1, "> %c %s", is_dir ? '}' : '{', name);
            curr_filename = get_filename(file_list[i + offset].full_path);
            show_directory = is_dir;
            selected_version = file_list[i + offset].version;
        }
    }
    set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);

    if (rep)
        isCustom = is_custom_replay(rep);
    set_clip_rect(bmp, x + 6, 0, x + w - 10 - (isCustom ? text_length(font, "CUSTOM GAME  ") : 0), bmp->h - 1);
    textprintf_ex(bmp, font, x + 10, y + 315, mg, -1, "%s", curr_filename ? curr_filename : "n/a");
    set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);

    textout_ex(bmp, font, "NAME", x + 10, y + 335, fg, -1);
    textout_right_ex(bmp, font, "SCORE", x + 180, y + 335, fg, -1);
    textout_right_ex(bmp, font, "FLOOR", x + 230, y + 335, fg, -1);
    textout_right_ex(bmp, font, "COMBO", x + 280, y + 335, fg, -1);
    textout_ex(bmp, font, "DATE ", x + 300, y + 335, fg, -1);

    if (rep) {
        set_clip_rect(bmp, x + 9, 0, x + 133, bmp->h - 1);
        textprintf_ex(bmp, font, x + 10, y + 350, fg, -1, "%s", rep->name);
        set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);
        textprintf_right_ex(bmp, font, x + 180, y + 350, fg, -1, "%6d", rep->score);
        textprintf_right_ex(bmp, font, x + 230, y + 350, fg, -1, "%4d", rep->floor);
        textprintf_right_ex(bmp, font, x + 280, y + 350, fg, -1, "%3d", rep->combo);
        textprintf_ex(bmp, font, x + 300, y + 350, fg, -1, "%s ", rep->date);
        if (isCustom)
            textprintf_right_ex(bmp, font, x + w - 10, y + 315, mg, -1, "%s", "CUSTOM GAME");
        if (rep->comment[0])
            textprintf_ex(bmp, font, x + 10, y + 370, mg, -1, "%s", rep->comment);
    }
    else {
        if (show_directory)
            textout_ex(bmp, font, "This is a folder. Press ENTER to open it.", x + 10, y + 350, fg, -1);
        else {
            char rbuf[129];
            switch (selected_version) {
            case 1:
                strcpy(rbuf, "You need Icy Tower 1.2 to view this replay.");
                break;
            case 0x82:
                strcpy(rbuf, "You need Icy Tower 1.3 to view this replay.");
                break;
            default:
                strcpy(rbuf, "Replay is broken.");
            }
            textout_ex(bmp, font, rbuf, x + 10, y + 350, makecol(80, 20, 20), -1);
        }
    }
}
{
    int w = 310;
    int h = 305;
    /* 462 */
    int fh = text_height(font);
    int i;
    /* 465 */
    int fg = makecol(25, 25, 25);
    /* 466 */
    int mg = makecol(85, 85, 85);
    /* 467 */
    double view_percentage = max_posts ? (double)num_itr_files / max_posts : 1.0;
    double view_offset = max_posts ? (double)offset / max_posts : 0.0;
    /* Original DWARF identifies curr_filename as char * at EBP-0x448.
     * The selected-row path stores get_filename(post->full_path) there after
     * rendering the row. */
    char *curr_filename;
    int is_dir;
    int show_directory = 0;
    int selected_version = 0;
    int isCustom = 0;

    /* 467 */
    if (view_percentage > 1.0f)
        view_percentage = 1.0f;
    /* 473 */
    if (data && data[86].dat)
        stretch_sprite(bmp, data[86].dat, x - 15, y - 15, w + 30, h + 30);
    else
        rectfill(bmp, x - 15, y - 15, x + w + 15, y + h + 15, mg);

    /* 476 */
    set_trans_blender(0, 0, 0, 150);
    /* 477 */
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
    /* 479 */
    rect(bmp, x + 5, y + 30, x + w - 5, y + 305, fg);
    /* 479 */
    rect(bmp, x + 7, y + 32, x + w - 7, y + 303, fg);
    /* 487 */
    solid_mode();

    /* 490: textout_ex(bmp, data[51].dat, "SELECT REPLAY", x+10, y-12, -1, -1) --
     * slots 0=bmp,4=data[51].dat (0x330/0x10=51),8=text,0xc=x+10,0x10=y-12,
     * 0x14=-1,0x18=-1, all read directly off the movl/mov-before-call chain. */
    textout_ex(bmp, data[51].dat, "SELECT REPLAY", x + 10, y - 12, -1, -1);

    draw_sprite(bmp, data[89].dat, x + 0x136, y + 0x22);
    draw_sprite(bmp, data[112].dat, x + 0x137, y + 0x104);
    switch (sort_method) {
    case 1:
        draw_sprite(bmp, data[115].dat, x + 0x112, y + 0x141);
        break;
    case 2:
        draw_sprite(bmp, data[116].dat, x + 0x112, y + 0x163);
        break;
    case 3:
        draw_sprite(bmp, data[113].dat, x + 0x11e, y + 0x163);
        break;
    case 4:
        draw_sprite(bmp, data[114].dat, x + 0x11e, y + 0x141);
        break;
    }

    /* 509: set_clip_rect(bmp,x+6,0,x+0x122,bmp->h-1) -- the current reconstruction's
     * (x+10,y+35,x+185,y+300) does not match a single one of these four operands. */
    set_clip_rect(bmp, x + 6, 0, x + 290, bmp->h - 1);
    for (i = offset; i < num_itr_files && i < offset + max_posts; i++) {
        char name[1024];
        /* 516: Treplay_post *post = &file_list[i]; cmpb $0,0x5(edx) tests offset 5,
         * which is Treplay_post.parent (src/replay.c:30), not .directory (offset 4). */
        Treplay_post *post = &file_list[i];
        int row = y + 40 + (i - offset) * fh;
        char marker;

        if (post->parent) {
            /* 517: rep movsb copying the literal ".. (parent directory)" (22 bytes)
             * into the buffer later reused as name (same -0x418(%ebp) slot as the
             * warning messages at 591/594/597 below). */
            strcpy(name, ".. (parent directory)");
        } else {
            /* 519: strcpy(name, get_filename(post->full_path)) -- a real buffer,
             * not a bare `char *name` as the current source has it. */
            strcpy(name, get_filename(post->full_path));
        }
        /* 521: is_dir = post->directory (movsbl 0x4(%edx),%esi) -- the named-but-
         * missing local; offset 4, confirmed against the Treplay_post typedef. */
        is_dir = post->directory;
        marker = is_dir ? '}' : '{';
        if (i == selection) {
            /* Original offsets 0x568..0x5ec: alpha-50 mode, the inlined
             * rectfill(vtable+0x3c), solid mode, then selected text. */
            set_trans_blender(0, 0, 0, 50);
            drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
            rectfill(bmp, x + 7, row, x + 0x125, row + fh - 9, fg);
            solid_mode();
            textprintf_ex(bmp, font, x + 8, row, mg, -1, "> %c %s", marker, name);
        } else if (is_dir) {
            /* 525 (predecessor A): color mg when is_dir. */
            textprintf_ex(bmp, font, x + 8, row, mg, -1, "  %c %s", marker, name);
        } else {
            /* 525 (predecessor B): color fg when !is_dir. */
            textprintf_ex(bmp, font, x + 8, row, fg, -1, "  %c %s", marker, name);
        }
        if (i == selection) {
            curr_filename = get_filename(post->full_path);
            show_directory = post->directory;
            selected_version = post->version;
        }
    }
    set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);

    /* 486/487 (bar): unchanged from the existing reconstruction -- not
     * re-verified against fresh instruction evidence this pass. */
    if (num_itr_files > max_posts && max_posts > 0) {
        int bar_top = y + 40 + (int)(view_offset * 260.0);
        int bar_height = (int)(view_percentage * 260.0);
        rectfill(bmp, x + 190, bar_top, x + 196, bar_top + bar_height, fg);
    }

    /* 546 */
    if (rep) {
        /* 547 */
        isCustom = is_custom_replay(rep);
        /* 551: set_clip_rect(bmp,x+6,0,x+390-(isCustom?text_length(font,"CUSTOM GAME  "):0),bmp->h-1) --
         * x+390 from x+0x190 (established for the scrollbar/table region) minus 10;
         * two historical predecessors (one measuring the label, one using 0) merge
         * into this one call. */
        set_clip_rect(bmp, x + 6, 0,
                      x + 390 - (isCustom ? text_length(font, "CUSTOM GAME  ") : 0),
                      bmp->h - 1);
        /* 552: rep->comment falls back to the literal "n/a" when falsy; fg,
         * y+315, x+10. */
        textprintf_ex(bmp, font, x + 10, y + 315, fg, -1, "%s",
                      rep->comment[0] ? rep->comment : "n/a");

        /* 562: a second, explicit `if (rep)` gates the results table below --
         * confirmed by its own independent null test on the rep parameter. */
        if (rep) {
            /* 563: a narrow clip around the NAME column, x+9..x+133. */
            set_clip_rect(bmp, x + 9, 0, x + 133, bmp->h - 1);
            /* 564: textprintf_ex(bmp,font,x+10,y+350,fg,-1,"%s",rep->name) --
             * rep+0xc is rep->name (offsetof(Treplay,name)==12, confirmed
             * against include/recovered/Treplay.h). */
            textprintf_ex(bmp, font, x + 10, y + 350, fg, -1, "%s", rep->name);
            /* 565: full clip reset before the unclipped table. */
            set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);
            /* 555..559: NAME/SCORE/FLOOR/COMBO/DATE column headers, all row y+335. */
            textout_ex(bmp, font, "NAME", x + 10, y + 335, fg, -1);
            textout_right_ex(bmp, font, "SCORE", x + 180, y + 335, fg, -1);
            textout_right_ex(bmp, font, "FLOOR", x + 230, y + 335, fg, -1);
            textout_right_ex(bmp, font, "COMBO", x + 280, y + 335, fg, -1);
            textout_ex(bmp, font, "DATE ", x + 300, y + 335, fg, -1);
            /* 566..569: the value row for `rep` itself, aligned under the headers
             * above, row y+350 (offsets 0x50/0x54/0x58/0x2c confirmed against
             * offsetof(Treplay,score/floor/combo/date) == 80/84/88/44). */
            textprintf_right_ex(bmp, font, x + 180, y + 350, fg, -1, "%6d", rep->score);
            textprintf_right_ex(bmp, font, x + 230, y + 350, fg, -1, "%4d", rep->floor);
            textprintf_right_ex(bmp, font, x + 280, y + 350, fg, -1, "%3d", rep->combo);
            textprintf_ex(bmp, font, x + 300, y + 350, fg, -1, "%s ", rep->date);
            if (isCustom) {
                /* 572/573: right-aligned "CUSTOM GAME" at the same narrowed clip
                 * edge computed at 551, color mg, row y+315 (the comment row's y). */
                textprintf_right_ex(bmp, font,
                                    x + 390 - text_length(font, "CUSTOM GAME  "),
                                    y + 315, mg, -1, "%s", "CUSTOM GAME");
            }
            if (rep->comment[0]) {
                /* 576/577: a second comment display, distinct from 552's (mg not
                 * fg, y+370 not y+315, no "n/a" fallback). */
                textprintf_ex(bmp, font, x + 10, y + 370, mg, -1, "%s", rep->comment);
            }
        }
    }

    /* 583: show_directory gates the folder hint; its own set-site inside the
     * loop was not located this pass (flagged, not guessed). selected_version's
     * set-site (presumably file_list[selection].version, matching the prior
     * reconstruction's guess) was likewise not re-confirmed against fresh
     * instruction evidence this pass -- flagged, not guessed. */
    if (show_directory) {
        /* 585 */
        textout_ex(bmp, font, "This is a folder. Press ENTER to open it.",
                  x + 10, y + 350, fg, -1);
    } else {
        char rbuf[129];
        /* 589/591/594/597: selected_version selects one of three messages into
         * rbuf (a third reuse of the same buffer as the loop rows and the
         * ".. (parent directory)" copy), then 599 prints it in a shared call. */
        if (selected_version == 1)
            strcpy(rbuf, "You need Icy Tower 1.2 to view this replay.");
        else if (selected_version == 0x82)
            strcpy(rbuf, "You need Icy Tower 1.3 to view this replay.");
        else
            strcpy(rbuf, "Replay is broken.");
        /* 599: makecol(0x50,0x14,0x14) = makecol(80,20,20), a color not used
         * anywhere else in this function. */
        textout_ex(bmp, font, rbuf, x + 10, y + 350, makecol(80, 20, 20), -1);
    }
}
{
    const int scroller_step = -1;
    BITMAP *head_bmp;
    BITMAP *head_shadow;
    BITMAP *head;
    int headX;
    int headY;
    char welcomeMessage[512];
    int mouseInAd;
    int i;

    count++;                                                        /* 5142 */
    if (new_rand() % 198 == 1)                                       /* 5143 */
        face++;                                                      /* 5144 */
    if (face == 3)
        face = 0;

    if (key[KEY_F1]) {                                               /* 5147 */
        take_screenshot(swap_screen);                                /* 5148 */
        while (key[KEY_F1])                                          /* 5149 */
            rest(2);                                                 /* 5161 */
    }
    testWindowResolution();                                          /* 5152 */

    if (pFLDAd) {                                                    /* 5156 */
        mouseInAd = mouse_x < pFLDAdBitmap->w && gfx_driver &&        /* 5158 */
                    mouse_y > gfx_driver->h - pFLDAdBitmap->h;

        if (key[KEY_F5]) {                                           /* 5160 */
            while (key[KEY_F5])
                rest(2);
            options.full_screen = 0;                                 /* 5165 */
            testWindowResolution();                                  /* 5166 */
            open_web_browser((char *)pFLDAd->pVisitURL);              /* 5169 */
            my_alert("Icy Tower", "Your web browser has been opened.", 0, 1); /* 5170 */
        } else if (mouseInAd) {
            if ((mouse_b & 1) && !(lastMouseB & 1)) {
                testWindowResolution();
                open_web_browser((char *)pFLDAd->pVisitURL);
                my_alert("Icy Tower", "Your web browser has been opened.", 0, 1);
            }
        }
        lastMouseB = mouse_b;                                        /* 5172 */
        _win_hcursor = LoadCursorA(NULL, mouseInAd ? IDC_HAND : IDC_ARROW);
    } else {
        lastMouseB = mouse_b;
    }

    blit(data[126].dat, swap_screen, 0, 0, 0, 0, 640, 480);
    draw_sprite(swap_screen, data[71].dat, 330, 280);
    if (pFLDAdBitmap) {                                               /* 5200 */
        set_alpha_blender();                                         /* 5201 */
        draw_trans_sprite(swap_screen, pFLDAdBitmap, 0, 280);         /* 5202 */
    }

    head = data[58 + face].dat;
    head_shadow = data[61].dat;
    head_bmp = create_bitmap(640, 320);
    clear_to_color(head_bmp, makecol(255, 0, 255));
    headX = fixsin(itofix(count * 3) + 10);
    headY = fixtoi(fixsin(itofix(count * 5)) * 10) + 16;
    rotate_sprite(head_bmp, head_shadow, 402, headY, headX * 5);
    set_trans_blender(0, 0, 0, 110);
    draw_trans_sprite(swap_screen, head_bmp, 0, 0);
    destroy_bitmap(head_bmp);
    headX = fixsin(itofix(count * 3) + 10);
    headY = fixtoi(fixsin(itofix(count * 5)) * 10) + 12;
    rotate_sprite(swap_screen, head, 400, headY, headX * 5);

    scroll_scroller(&greeting_scroller, scroller_step);               /* 5220 */
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);                           /* 5221 */
    set_trans_blender(0, 0, 0, 110);                                  /* 5222 */
    rectfill(swap_screen, 0, 0, 639, 20, makecol(0, 0, 0));           /* 5223 */
    rectfill(swap_screen, 0, 0, 639, 18, makecol(0, 0, 0));           /* 5224 */
    rectfill(swap_screen, 0, 0, 639, 16, makecol(0, 0, 0));           /* 5225 */
    solid_mode();                                                     /* 5226 */
    draw_scroller(&greeting_scroller, swap_screen, 1, 0, makecol(150, 150, 150)); /* 5227 */
    if (!draw_scroller(&greeting_scroller, swap_screen, 0, 0,        /* 5228 */
                       makecol(200, 200, 200)))
        restart_scroller(&greeting_scroller);

    textprintf_ex(swap_screen, data[54].dat, 5, 3, makecol(100, 21, 20), -1,
                  "v%s %s", "1.5.1", debug ? " FUN MODE" : "");
    textprintf_ex(swap_screen, data[54].dat, 4, 2, makecol(162, 90, 51), -1,
                  "v%s %s", "1.5.1", debug ? " FUN MODE" : "");
    if (stricmp(profile->handle, "guest")) {
        sprintf(welcomeMessage, "Welcome, %%s! %s",
                get_rank_id(profile) ? "Your rank is %s." : "");
        textprintf_right_ex(swap_screen, data[54].dat, 638, 3,
                            makecol(50, 50, 50), -1, welcomeMessage,
                            profile->handle, get_rank(profile));
        textprintf_right_ex(swap_screen, data[54].dat, 637, 3,
                            makecol(50, 50, 50), -1, welcomeMessage,
                            profile->handle, get_rank(profile));
        textprintf_right_ex(swap_screen, data[54].dat, 637, 2,
                            makecol(200, 200, 200), -1, welcomeMessage,
                            profile->handle, get_rank(profile));
        textprintf_right_ex(swap_screen, data[54].dat, 636, 2,
                            makecol(255, 255, 255), -1, welcomeMessage,
                            profile->handle, get_rank(profile));
    } else {
        strcpy(welcomeMessage,
               "Welcome to Icy Tower! Start a profile in the profile menu.");
        textprintf_right_ex(swap_screen, data[54].dat, 638, 3,
                            makecol(50, 50, 50), -1, "%s", welcomeMessage);
        textprintf_right_ex(swap_screen, data[54].dat, 637, 3,
                            makecol(50, 50, 50), -1, "%s", welcomeMessage);
        textprintf_right_ex(swap_screen, data[54].dat, 637, 2,
                            makecol(200, 200, 200), -1, "%s", welcomeMessage);
        textprintf_right_ex(swap_screen, data[54].dat, 636, 2,
                            makecol(255, 255, 255), -1, "%s", welcomeMessage);
    }

    options.snd_volume = get_slider_value(&snd_volume_slider);        /* 5255 */
    options.msc_volume = get_slider_value(&msc_volume_slider);        /* 5256 */

    if (characters[play_char.value].ok) {                             /* 5259 */
        for (i = 0; i < 256; i++) {                                   /* 5261 */
            play_char.pal[i].r = characters[play_char.value].pal[i].r; /* 5262 */
            play_char.pal[i].g = characters[play_char.value].pal[i].g; /* 5263 */
            play_char.pal[i].b = characters[play_char.value].pal[i].b; /* 5264 */
        }
    }
    play_char.bmp = characters[play_char.value].bmp;                  /* 5266 */
    curr_char = play_char.value;                                      /* 5267 */
    strcpy(profile->last_avatar, characters[play_char.value].name);   /* 5268 */

    floors.max = profile->best_floor > 999 ? 9 : profile->best_floor / 100; /* 5275 */
    profile->start_floor = floors.value < floors.max ? floors.value : floors.max; /* 5276 */
    menu_params.fo = floors.value * 3 + 17;                           /* 5277 */

    options.flash = get_selection_value(&eyecandy_selection);         /* 5282 */
    options.floor_shrink = floors.value;
    options.start_speed = get_selection_value(&scroll_speed_selection); /* 5285 */
    options.floor_size = get_selection_value(&floor_size_selection);  /* 5284 */
    options.gravity = get_selection_value(&gravity_selection);        /* 5283 */
    if (bg_menu)                     /* 5288 */
        adjust_sample(bg_menu, options.msc_volume, 128, 1000, 1);

    syncProfileFromOptions();                                         /* 972 */
}
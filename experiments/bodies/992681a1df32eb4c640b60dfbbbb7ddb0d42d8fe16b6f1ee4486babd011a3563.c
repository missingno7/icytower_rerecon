{
    Tcontrol *menu_ctrl = &menu_params.ctrl;
    int status;
    int done;
    int w;

    w = MAX(text_length(data[51].dat, func ? func : " "),      /* 467 */
            text_length(data[51].dat, txt ? txt : " "));
    gui_fg_color = makecol(0, 0, 0);                           /* 470 */
    gui_bg_color = makecol(255, 255, 255);                     /* 471 */
    set_trans_blender(0, 0, 0, 158);                           /* 473 */
    drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);                    /* 474 */
    rectfill(screen, 0, 0, gfx_driver ? gfx_driver->w : 0,
             gfx_driver ? gfx_driver->h : 0, makecol(0, 0, 0)); /* 475 */
    solid_mode();                                              /* 476 */
    blit(screen, swap_screen, 0, 0, 0, 0, 639, 479); /* 478 */
    acquire_bitmap(screen);
    draw_sprite(screen, data[88].dat, 103, 130);                /* 485 */
    textprintf_centre_ex(screen, data[51].dat, 320, 135, -1, -1, "%s", func); /* 486 */
    if (txt)
        textout_centre_ex(screen, data[54].dat, txt, 320, 180, makecol(0, 0, 0), -1); /* 487 */
    if (enter_hint)                                            /* 488 */
        textout_right_ex(screen, data[54].dat, "(enter to continue)", 520, 200,
                         makecol(80, 80, 80), -1);              /* 489 */
    release_screen();
    poll_control(&ctrl, 0);                                    /* 493 */
    poll_control(menu_ctrl, 0);                                 /* 494 */
    while (is_any(&ctrl) || is_any(menu_ctrl) || key[KEY_ESC]) { /* 495 */
        poll_control(&ctrl, 0); poll_control(menu_ctrl, 0); rest(2);
    }
    clear_keybuf();                                            /* 500 */
    done = 0;
    status = 0;
    while (!closeButtonClicked && !done) {                     /* 502 */
        cycle_count = 0;                                       /* 503 */
        poll_control(&ctrl, 0); poll_control(menu_ctrl, 0);     /* 504 */
        if (is_left(&ctrl)) status = -1;
        else if (is_left(menu_ctrl)) status = -1;
        if (is_right(&ctrl)) status = 0;
        else if (is_right(menu_ctrl)) status = 0;
        if (key[KEY_ESC]) {                                    /* 514 */
            done = -1;
            status = 0;
        }
        if (is_fire(&ctrl)) done = -1;
        else if (is_fire(menu_ctrl) || is_enter(menu_ctrl)) done = -1;
        if (choice) {                                          /* 523 */
            vsync();                                           /* 525 */
            draw_sprite(screen, data[status == -1 ? 11 : 10].dat, 240, 220); /* 527 */
            draw_sprite(screen, data[status == -1 ? 8 : 7].dat, 365, 220);   /* 529 */
        }
        if (!cycle_count) rest(2);                             /* 532 */
    }
    poll_control(&ctrl, 0);                                    /* 535 */
    poll_control(menu_ctrl, 0);                                 /* 536 */
    while (is_any(&ctrl) || is_any(menu_ctrl) || key[KEY_ESC] || key[KEY_ENTER]) { /* 537 */
        poll_control(&ctrl, 0); poll_control(menu_ctrl, 0); rest(2); /* 538 */
    }
    clear_keybuf();                                            /* 543 */
    blit(swap_screen, screen, 0, 0, 0, 0, 639, 479); /* 545 */
    return status;                                              /* 548 */
}
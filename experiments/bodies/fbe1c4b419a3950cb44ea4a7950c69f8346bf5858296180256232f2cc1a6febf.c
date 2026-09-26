{
    FONT *old_font = font;
    BITMAP *bg = create_bitmap(SCREEN_W, SCREEN_H);
    Treplay *rep = NULL;
    int curr_file_id = 0;
    int i;
    int done = 0;
    int ctrl_wait = 1000;
    int page_size;
    int offset = 0;
    int need_to_update = 1;
    int pageY = 500;
    int targetY = 0;

    if (!bg)
        return NULL;
    blit(screen, bg, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    if (data)
        font = data[54].dat;
    page_size = 270 / text_height(font);
    clear_keybuf();

    while (!done) {
        int kp = 0;

        if (need_to_update) {
            if (rep) {
                destroy_replay(rep);
                rep = NULL;
            }
            if (num_itr_files > 0 && curr_file_id >= 0 &&
                curr_file_id < num_itr_files && !itr_file_list[curr_file_id].directory)
                rep = load_replay(itr_file_list[curr_file_id].full_path);
            need_to_update = 0;
        }

        poll_control(ctrl, 1);
        if (is_any(ctrl) && ctrl_wait == 0) {
            if (is_down(ctrl))
                simulate_keypress(KEY_DOWN << 8);
            else if (is_up(ctrl))
                simulate_keypress(KEY_UP << 8);
            else if (is_fire(ctrl))
                simulate_keypress(KEY_ENTER << 8);
        }
        if (is_any(ctrl)) {
            if (ctrl_wait > 0)
                ctrl_wait--;
        } else {
            ctrl_wait = 0;
        }

        kp = keypressed();
        if (!kp) {
        /* 889: pageY interpolates toward the literal 25 at a fixed 0.2 rate --
         * NOT the existing `pageY -= (pageY - targetY) / 3 + 1;` formula this
         * replaces. Evidenced directly (fldl 0.2; fimul; fiadd; fistp),
         * unguarded by any `pageY > targetY` test in this fragment (the test
         * that gated the old formula was not found in this instruction
         * range and may not exist at all -- flagged, not confirmed absent). */
        pageY = (int)(0.2 * (25 - pageY) + pageY);
        /* 892: blit target is swap_screen, not screen -- confirmed by the
         * 0x4dd194 operand at every call in this whole present sequence
         * (blit, draw_replay_selector's bmp arg, blit_to_screen). */
        blit(bg, swap_screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);           /* 892 */
        set_trans_blender(0, 0, 0, (500 - pageY) / 3);                    /* 894 */
        drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);                            /* 895 */
        /* 896: rectfill's bottom-right corner is gfx_driver->w/->h (0x70/0x6c)
         * when a driver is installed, not the SCREEN_W/SCREEN_H constants. */
        rectfill(swap_screen, 0, 0, gfx_driver ? gfx_driver->w : 0,          /* 896 */
                 gfx_driver ? gfx_driver->h : 0, makecol(0, 0, 0));
        solid_mode();                                                        /* 897 */
        /* 899: y is pageY itself, with no "+120" added at the call site --
         * the existing reconstruction's `pageY + 120` does not match this
         * call's own operand (a bare register carrying pageY's value). */
        draw_replay_selector(swap_screen, rep, itr_file_list, curr_file_id,   /* 899 */
                             offset, page_size, 120, pageY);
        blit_to_screen(swap_screen);                                          /* 900 */
        /* 902: a wait loop distinct from the plain checkMenuFocus()/rest(2)
         * pair the current source has -- spins on cycle_count via rest(2),
         * and snapshots curr_file_id into a second local (used as the
         * *second* draw_replay_selector call's `selection` argument below)
         * before the wait. */
        if (cycle_count > 0) {
            i = curr_file_id;                              /* 902 */

            do {
                rest(2);                                                        /* 902 */
            } while (cycle_count <= 0);

            /* 904: skip the second pass entirely when rep is NULL. */
            if (rep) {
                /* 906/907/908: pageY interpolates toward 510 (not targetY) at
                 * a fixed 0.2 rate, only while pageY <= 499; cycle_count is
                 * reset to 0 here (distinct from the reset already implicit
                 * in the wait loop above). */
                if (pageY <= 499) {
                    cycle_count = 0;                                             /* 907 */
                    pageY = (int)(0.2 * (510 - pageY) + pageY);                    /* 908 */
                }
                /* 911..916: a second blit/blend/drawing_mode/rectfill/solid_mode
                 * pass, identical in shape to 892..897 above. */
                blit(bg, swap_screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);             /* 911 */
                set_trans_blender(0, 0, 0, (500 - pageY) / 3);                      /* 913 */
                drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);                             /* 914 */
                rectfill(swap_screen, 0, 0, gfx_driver ? gfx_driver->w : 0,           /* 915 */
                         gfx_driver ? gfx_driver->h : 0, makecol(0, 0, 0));
                solid_mode();                                                        /* 916 */
                /* 918: rep forced NULL and selection is the frozen snapshot,
                 * not the live curr_file_id -- confirmed by comparing this
                 * call's argument slots against 899's directly. */
                draw_replay_selector(swap_screen, NULL, itr_file_list,               /* 918 */
                                     i, offset, page_size, 120, pageY);
                blit_to_screen(swap_screen);                                          /* 919 */
            }
        }
        checkMenuFocus();
        rest(2);
            kp = keypressed();
        }

        if (kp) {
            kp = readkey() >> 8;
            ctrl_wait = 20;
            switch (kp) {
            case KEY_N:
                set_sort_method(1);
                need_to_update = 1;
                break;
            case KEY_S:
                set_sort_method(2);
                need_to_update = 1;
                break;
            case KEY_C:
                set_sort_method(3);
                need_to_update = 1;
                break;
            case KEY_F:
                set_sort_method(4);
                need_to_update = 1;
                break;
            case KEY_F1: {
                char p[1024];

                play_menu_select();
                if (rep)
                    destroy_replay(rep);
                gui_bg_color = makecol(255, 255, 255);
                gui_fg_color = makecol(0, 0, 0);
                install_mouse();
                file_select_ex("Select a new folder and press OK.", path, "itr",
                               sizeof(p), 400, 400);
                remove_mouse();
                replace_filename(p, path, "", sizeof(p));
                strcpy(path, p);
                need_to_update = 1;
                offset = 0;
                curr_file_id = 0;
                break;
            }
            case KEY_UP:
                if (curr_file_id > 0) {
                    curr_file_id--;
                    if (curr_file_id < offset)
                        offset--;
                    play_menu_move();
                    need_to_update = 1;
                }
                break;
            case KEY_DOWN:
                if (curr_file_id + 1 < num_itr_files) {
                    curr_file_id++;
                    if (curr_file_id >= offset + page_size)
                        offset++;
                    play_menu_move();
                    need_to_update = 1;
                }
                break;
            case KEY_ESC:
                play_menu_select();
                destroy_replay(rep);
                rep = NULL;
                done = -1;
                break;
            case KEY_ENTER:
            case KEY_SPACE: {
                char p[1024];

                if (curr_file_id >= 0 && curr_file_id < num_itr_files) {
                    Treplay_post *post = &itr_file_list[curr_file_id];
                    if (post->directory) {
                        strcpy(path, post->full_path);
                        canonicalize_filename(p, path, sizeof(p));
                        strcpy(path, p);
                        curr_file_id = offset = 0;
                        need_to_update = 1;
                    } else if (rep) {
                        play_menu_select();
                        done = 1;
                    }
                }
                break;
            }
            case KEY_DEL: {
                char fname[512];

                if (curr_file_id >= 0 && curr_file_id < num_itr_files &&
                    !itr_file_list[curr_file_id].directory && rep) {
                    play_menu_select();
                    strcpy(fname, "Really delete replay?");
                    if (my_alert(fname, "WARNING: It will be gone forever.", 1, 0)) {
                        delete_file(itr_file_list[curr_file_id].full_path);
                        need_to_update = 1;
                    }
                }
                break;
            }
            default:
                break;
            }
        }

        if (need_to_update) {
            update_file_list(path);
            if (curr_file_id >= num_itr_files)
                curr_file_id = num_itr_files - 1;
            if (curr_file_id < 0)
                curr_file_id = 0;
            if (offset > curr_file_id)
                offset = curr_file_id;
        }


    }

    destroy_bitmap(bg);
    font = old_font;
    for (i = 0; i < num_itr_files; i++) {
        free(itr_file_list[i].full_path);
        itr_file_list[i].parent = 0;
        itr_file_list[i].directory = 0;
    }
    num_itr_files = 0;
    return rep;
}
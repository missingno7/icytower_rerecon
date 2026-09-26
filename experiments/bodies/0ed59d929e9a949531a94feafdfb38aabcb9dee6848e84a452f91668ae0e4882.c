{
    stepIn = dx;
    int menu_return;

    int handle_keys;
    int done = 0;
    void *data = NULL;
    int key_counter;


    reset_menu(menu, mp, mp->pos);

    handle_keys = 0; key_counter = 0; menu_return = 0;
    while (!closeButtonClicked) {
        cycle_count = 0;


        checkMenuFocus();

        if (callback) callback();
        else clear(bmp);


        menu_return = update_game_menu(bmp, menu, mp, handle_keys ? ctrl : NULL, x, y, &data);
        blit_to_screen(bmp);


        if (is_any(ctrl) || is_any(&mp->ctrl) || key[KEY_ESC]) {





            if (!key_counter) { key_counter = 39; handle_keys = 0; }




            else if (!--key_counter) handle_keys = -1;


        }
        else { key_counter = 0; handle_keys = -1; }
        poll_control(ctrl, 1);
        poll_control(&mp->ctrl, 0);

        if (menu_return) play_menu_select();



        switch (menu_return) {
            case 0:
                break;
            case 101:
            case 133:
                return 133;




            case 132:
                change_profile();
                break;

            case 131:
                view_profile(profile);
                break;
            case 122:
                return 122;

            case 123:
                return 123;


            case 124:
                return 124;


            case 104:
                return 104;


            case 105:
                return 105;



            case 107:
                return 107;
            case 103:
                mp->pos = 0;
                { int sub_ret = handle_menu((Tmenu *)data, mp, ctrl, bmp, callback, x, y, dx);
                if (sub_ret && sub_ret != 108) return sub_ret; }
                break;




            case 108:
                return 108;







            case 109: {
                Tmenu_slider *sld = (Tmenu_slider *)data;
                sld->value = MIN(sld->value + sld->step, sld->max);
                break;
            }
            case 110: {
                Tmenu_slider *sld = (Tmenu_slider *)data;
                sld->value = MAX(sld->value - sld->step, sld->min);
                break;
            }
            case 112: {
                Tmenu_selection *sel = (Tmenu_selection *)data;
                sel->value = MIN(sel->value + 1, sel->size - 1);
                break;
            }


















            case 119:
            case 121: {
                Tmenu_char_selection *sel = (Tmenu_char_selection *)data;
                sel->value = MIN(sel->value + 1, sel->max);
                break;
            }
            case 111: case 118: case 120: {
                Tmenu_char_selection *sel = (Tmenu_char_selection *)data;
                sel->value = MAX(sel->value - 1, 0);
                break;
            }
            case 113: {
                int *box = (int *)data;
                *box = *box < 1 ? -1 : 0;
                break;
            }
            case 114: {

                int kp, k;


                char txt[256];
                sprintf(txt, "press key for %s", menu[mp->pos].caption);
                line_alert(txt);

                for (k = 0; k < 128; k++) key[k] = 0;
                for (;;) { k = 0;
                    for (kp = 0; kp < 128; kp++) if (key[kp]) k = kp;
                    if (k == KEY_ESC) k = *(int *)data;

                    rest(2);
                    if (k) break; }

                for (kp = 0; kp < 128; kp++) key[kp] = 0;
                *(int *)data = k;

                play_menu_select();

                break;
            }


            default: {
                char buf[128];
                sprintf(buf, "unknown return value: %d", menu_return);
                my_alert("handle_menu", buf, 0, 0);
                break;
            }
        }


        if (!(key[KEY_TAB] && key[KEY_LSHIFT]))


            while (!cycle_count) rest(2);
    }


    return menu_return;
}
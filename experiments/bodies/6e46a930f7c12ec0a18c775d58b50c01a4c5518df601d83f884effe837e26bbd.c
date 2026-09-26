{
    Tprofile_create *selectedProfile;
    int kp;
    int done;
    void *old_font;
    int profileIndex;
    int offset;
    int page_size;
    int ctrl_wait;
    int pageY;
    int targetY;
    void *bgbmp;

    old_font = font;
    font = data[54].dat;
    bgbmp = create_bitmap(640, 480);
    blit(screen, bgbmp, 0, 0, 0, 0, 640, 480);
    clear_keybuf();
    selectedProfile = 0;
    profileIndex = 0;
    offset = 0;
    page_size = 16;
    pageY = 500;
    done = 0;
    ctrl_wait = 1000;
    targetY = 50;

    while (!closeButtonClicked && !done) {
        cycle_count = 0;
        checkMenuFocus();
        poll_control(ctrl, 1);
        if (is_any(ctrl) && !ctrl_wait) {
            if (is_down(ctrl))
                simulate_keypress(0x5500);
            if (is_up(ctrl))
                simulate_keypress(0x5400);
            if (is_fire(ctrl))
                simulate_keypress(0x4300);
            ctrl_wait = 20;
        }
        if (is_any(ctrl)) {
            if (ctrl_wait > 0)
                ctrl_wait--;
        } else {
            ctrl_wait = 0;
        }

        if (keypressed()) {
            kp = readkey() >> 8;
            switch (kp) {
            case 85: {
                if (profileIndex < numProfiles - 1) {
                    profileIndex++;
                    if (profileIndex >= offset + page_size)
                        offset++;
                    play_menu_move();
                } else {
                    profileIndex = numProfiles - 1;
                    offset = numProfiles - page_size;
                    if (offset < 0)
                        offset = 0;
                }
                break;
            }
            case 84: {
                if (profileIndex > 0) {
                    profileIndex--;
                    if (offset > profileIndex)
                        offset--;
                    play_menu_move();
                } else {
                    profileIndex = 0;
                    offset = 0;
                }
                break;
            }
            case 77: {
                if (stricmp(profiles[profileIndex].handle, "guest") &&
                    stricmp(profiles[profileIndex].handle, current_profile->handle)) {
                    char buff[256];
                    sprintf(buff, "Really delete '%s'?", profiles[profileIndex].handle);
                    if (my_alert(buff, "WARNING: It will be gone forever.",
                                 1, 0)) {
                        delete_profile(profiles[profileIndex].handle);
                        numProfiles = rebuild_profile_list(&profiles);
                    }
                }
                break;
            }
            case 67:
            case 75: {
                play_menu_select();
                if (!stricmp("CREATE NEW PROFILE", profiles[profileIndex].handle)) {
                    set_trans_blender(0, 0, 0, 158);
                    drawing_mode(5, 0, 0, 0);
                    rectfill(swap_screen, 0, 0,
                             SCREEN_W,
                             SCREEN_H,
                             makecol(0, 0, 0));
                    solid_mode();
                    char new_name[32];
                    new_name[0] = 0;
                    draw_sprite(swap_screen, data[88].dat, 100, 140);
                    textout_ex(swap_screen, data[51].dat, "Enter profile name:",
                               140, 140, -1, -1);
                    textout_right_ex(swap_screen, data[54].dat,
                                     "...and press enter.", 480, 210, 0, -1);
                    rect(swap_screen, 139, 191, 480, 210,
                         makecol(255, 255, 255));
                    rectfill(swap_screen, 139, 191, 480, 210,
                             makecol(80, 80, 80));
                    if (get_string(swap_screen, new_name, 340, 32, data[54].dat,
                                   140, 191, makecol(0, 0, 0), -1) >= 0 &&
                        new_name[0]) {
                        replaceBadCharacters(new_name, '_');
                        selectedProfile = create_profile(new_name, 0);
                        if (selectedProfile) {
                            my_alert("CREATE PROFILE", "Profile created!", 0, 1);
                            done = -1;
                        } else {
                            my_alert("CREATE PROFILE", "Failed to create profile.", 0, 1);
                        }
                    }
                } else {
                    selectedProfile = load_profile(profiles[profileIndex].handle);
                    if (selectedProfile)
                        done = -1;
                    else
                        my_alert("SELECT PROFILE",
                                 "The profile you selected is broken.", 0, 1);
                }
                break;
            }
            case 59: {
                play_menu_select();
                clear_keybuf();
                done = -1;
                break;
            }
            }
        }

        pageY += (targetY - pageY) * 0.2;
        blit(bgbmp, swap_screen, 0, 0, 0, 0, 640, 480);
        set_trans_blender(0, 0, 0, (500 - pageY) / 3);
        drawing_mode(5, 0, 0, 0);
        rectfill(swap_screen, 0, 0, SCREEN_W,
                 SCREEN_H, makecol(0, 0, 0));
        solid_mode();
        draw_profile_selector(swap_screen, current_profile,
                              profiles, numProfiles, profileIndex, offset,
                              page_size, 16, pageY);
        blit_to_screen(swap_screen);
        while (cycle_count <= 0)
            rest(2);
    }

    if (selectedProfile) {
        char buf[128];
        sprintf(buf, "Now using profile '%s'", selectedProfile->handle);
        my_alert("Profile Changed!", buf, 0, 1);
    }

    targetY = 510;
    while (pageY <= 499) {
        cycle_count = 0;
        pageY += (targetY - pageY) * 0.2;
        blit(bgbmp, swap_screen, 0, 0, 0, 0, 640, 480);
        set_trans_blender(0, 0, 0, (500 - pageY) / 3);
        drawing_mode(5, 0, 0, 0);
        rectfill(swap_screen, 0, 0, SCREEN_W,
                 SCREEN_H, makecol(0, 0, 0));
        solid_mode();
        draw_profile_selector(swap_screen, current_profile,
                              profiles, numProfiles, profileIndex, offset,
                              page_size, 16, pageY);
        blit_to_screen(swap_screen);
        while (cycle_count <= 0)
            rest(2);
    }
    destroy_bitmap(bgbmp);
    font = old_font;
    return selectedProfile;
}
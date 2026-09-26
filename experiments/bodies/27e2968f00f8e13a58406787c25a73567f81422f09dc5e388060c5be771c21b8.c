{
    char full_path[1024];
    FILE *f;
    int i;
    int ret;
    int must_fade;
    int play_again;
    HMODULE hDebugLibrary;

    hDebugLibrary = LoadLibraryA("exchndl.dll");
    if (!hDebugLibrary)
        printf("No exception handler present, RPTs will not be generated");
    allegro_init();
    register_png_file_type();
    get_executable_name(full_path, sizeof(full_path));
    replace_filename(working_directory, full_path, "", sizeof(working_directory));
    chdir(working_directory);
    char logfilename[256] = {0};
    get_logfile_path(logfilename, sizeof(logfilename));
    f = fopen(logfilename, "wt");
    if (f) {
        fprintf(f, "Icy Tower v%s - log file\n----------------------------\n", "1.5.1");
        fclose(f);
    }
    for (i = 0; i < argc; i++)
        if (!stricmp(argv[i], "-check"))
            itrcheck = 1;
    log2file("Game started with the following commands:");
    for (i = 0; i < argc; i++)
        log2file("   %s", argv[i]);
    log2file("Working directory is:\n   %s", working_directory);
    if (!init_game(argc, argv)) {
        if (!dropped_file_is_not_a_replay) {
            log2file("* Failed to initialize the game *");
            allegro_message("Failed to initialize the game.");
        }
        log2file("Cleaning up Allegro");
        uninit_game();
        log2file("Done...");
        return 1;
    }
    if (!itrcheck) {
        log2file("Initiating scroller");
        init_scroller(&greeting_scroller, data[54].dat, scroller_greetings, 640, 30, -1);
        menu_params.font = data[51].dat;
        menu_params.bullet = data[72].dat;
        menu_params.pos = 0;
        menu_params.data = data;
        log2file("Initiating menu controls");
        init_control(&menu_params.ctrl);
        if (got_joystick)
            menu_params.ctrl.use_joy = 1;
        log2file("Resetting menu");
        reset_menu(main_menu, &menu_params, 0);
    }
    if (demo) {
        log2file("Running replay.");
        run_demo(NULL);
        if (itrcheck) {
            log2file("Exiting Allegro");
            allegro_exit();
            log2file("\nDone...");
            exit(0);
        }
    }
    load_new_ad_image();
    startMenuMusic();
    log2file("\nMAIN MENU LOOP");
    clear_keybuf();
    if (options.timesStarted == 1 && !stricmp("guest", options.lastProfile)) {
        main_menu_callback();
        draw_menu(swap_screen, main_menu, &menu_params, 355, 285, 0);
        fadeIn(swap_screen, 16);
        force_create_profile();
        syncOptionsFromProfile();
        must_fade = 0;
    }
    else
        must_fade = 1;
    options.msc_volume = profile->msc_volume;
    options.snd_volume = profile->snd_volume;
    ret = 0;
    while (!closeButtonClicked && ret != 'k') {
        main_menu_callback();
        draw_menu(swap_screen, main_menu, &menu_params, 355, 285, 0);
        if (must_fade)
            fadeIn(swap_screen, 16);
        else
            blit_to_screen(swap_screen);
        ret = handle_menu(main_menu, &menu_params, &ctrl, swap_screen, main_menu_callback, 355, 285, 0);
        if (ret == 'e' || ret == 0x85) {
            log2file(" new game selected");
            is_playing_custom_game = (ret != 'e');
            fadeOut(16);
            stopMenuMusic();
            play_again = 1;
            while (play_again) {
                if (demo) destroy_replay(demo);
                demo = NULL;
                play_again = 0;
                if (new_game()) {
                    play_again = play();
                    end_game();
                }
                fadeOut(16);
            }
            if (bg_menu)
                play_sample(bg_menu, options.msc_volume, 128, 1000, 1);
            menu_params.pos = 0;
            must_fade = 1;
        }
        else if (ret == 'i') {
            log2file(" high scores selected");
            view_scores(hisc_tables, hisc_names);
            menu_params.pos = 3;
            must_fade = 0;
        }
        else if (ret == 'h') {
            log2file(" instructions selected");
            fadeOut(16);
            show_instructions();
            menu_params.pos = 1;
            must_fade = 1;
        }
        else if (ret == 'z') {
            log2file(" load replay selected");
            if (demo) destroy_replay(demo);
            log2file("   opening %s", replay_directory);
            while ((demo = replay_selector(&ctrl, replay_directory))) {
                fadeOut(16);
                stopMenuMusic();
                run_demo(NULL);
                fadeOut(16);
                main_menu_callback();
                draw_menu(swap_screen, main_menu, &menu_params, 355, 285, 0);
                fadeIn(swap_screen, 32);
                if (bg_menu && options.msc_volume)
                    play_sample(bg_menu, options.msc_volume, 128, 1000, 1);
            }
            menu_params.pos = 4;
            must_fade = 0;
        }
        rest(2);
    }
    fadeOut(16);
    log2file("\nShowing credits");
    show_credits();
    stopMenuMusic();
    uninit_game();
    log2file("\nDone...");
    return 0;
}
{
    char file[1024];
    Tprofile_create *p;
    int i;
    time_t now;
    struct tm *my_time;
    int year, month, day;

    get_profile_dir_for_profile(file, 1024, handle);
    sprintf(file, "%s%s.itp", file, handle);
    if (!overwrite && exists(file)) {
        log2file("Overwrite is false and %s already exists", file);
        return 0;
    }
    p = malloc(sizeof(Tprofile_create));
    p->header[0] = 'I';
    p->header[1] = 'T';
    p->header[2] = 'P';
    p->header[3] = '1';
    p->header[4] = '4';
    p->header[5] = '0';
    strcpy(p->handle, handle);
    p->custom_games_played = 0;
    p->games_played = 0;
    p->games_quit = 0;
    p->seconds_spent_playing = 0;
    p->best_score = 0;
    p->best_combo = 0;
    p->best_floor = 0;
    p->total_combo_floors = 0;
    p->total_combos = 0;
    p->total_score = 0;
    p->total_floors = 0;
    p->no_combo_top_floor = 0;
    p->biggest_lost_combo = 0;
    p->cccTotal[4] = 0;
    p->cccTotal[3] = 0;
    p->cccTotal[2] = 0;
    p->cccTotal[1] = 0;
    p->cccTotal[0] = 0;
    p->cccNum[4] = 0;
    p->cccNum[3] = 0;
    p->cccNum[2] = 0;
    p->cccNum[1] = 0;
    p->cccNum[0] = 0;
    p->ccc[4] = 0;
    p->ccc[3] = 0;
    p->ccc[2] = 0;
    p->ccc[1] = 0;
    p->ccc[0] = 0;
    p->jc[4] = 0;
    p->jc[3] = 0;
    p->jc[2] = 0;
    p->jc[1] = 0;
    p->jc[0] = 0;
    for (i = 0; i < 10; i++)
        p->rewards[i] = 0;
    for (i = 0; i < 32; i++)
        *p->best_replay_names[i] = 0;
    p->total_jumps = 0;
    strcpy(p->last_avatar, "harold_the_homeboy");
    p->jump_hold = 1;
    p->start_floor = 0;
    p->flash = 0;
    p->msc_volume = 150;
    p->snd_volume = 150;
    now = time(0);
    my_time = localtime(&now);
    year = my_time->tm_year;
    month = my_time->tm_mon + 1;
    day = my_time->tm_mday;
    sprintf(p->creationDate, "%d-%s%d-%s%d", year + 1900,
            month < 10 ? "0" : "", month, day < 10 ? "0" : "", day);
    strcpy(p->saveDate, p->creationDate);
    init_control(get_controls());
    if (save_profile(p) < 0) {
        log2file("failed to create profile %s", handle);
        free(p);
        return 0;
    }
    return p;
}
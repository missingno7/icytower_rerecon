{
    void *pf;
    int i;
    time_t my_time;
    struct tm *timer;
    char *months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    char filename[2048];
    int cs;

    sprintf(filename, "%s%s", path, file);
    log2file("  saving replay: %s", filename);
    if (make_new_date) {
        my_time = time(0);
        timer = localtime(&my_time);
        strcpy(r->date, "              ICYTOWERISGREAT ");
        sprintf(r->date, "%2d %3s %4d", timer->tm_mday,
                months[timer->tm_mon], timer->tm_year + 1900);
    } else {
        Treplay *tmp;

        tmp = load_replay(filename);
        if (tmp)
            strcpy(r->date, tmp->date);
    }
    r->size = size;
    cs = calc_replay_checksum(r);
    r->checksum = cs;
    pf = pack_fopen(filename, "wb");
    if (!pf)
        return -1;
    pack_fwrite(r->header, 6, pf);
    pack_fwrite(&r->size, 4, pf);
    pack_fwrite(r->name, 32, pf);
    pack_fwrite(r->date, 32, pf);
    pack_fwrite(&r->score, 4, pf);
    pack_fwrite(&r->floor, 4, pf);
    pack_fwrite(&r->combo, 4, pf);
    pack_fwrite(&r->no_combo_top_floor, 4, pf);
    pack_fwrite(&r->biggest_lost_combo, 4, pf);
    for (i = 0; i < 5; i++)
        pack_fwrite(&r->ccc[i], 4, pf);
    for (i = 0; i < 5; i++)
        pack_fwrite(&r->jc[i], 4, pf);
    pack_fwrite(&r->floor_shrink, 4, pf);
    pack_fwrite(&r->floor_size, 4, pf);
    pack_fwrite(&r->start_speed, 4, pf);
    pack_fwrite(&r->speed_increase, 4, pf);
    pack_fwrite(&r->gravity, 4, pf);
    pack_fwrite(&r->rejump, 4, pf);
    pack_fwrite(&r->random_seed, 4, pf);
    pack_fwrite(r->comment, 42, pf);
    pack_fwrite(&r->checksum, 4, pf);
    pack_fwrite(&r->tc_posts, 4, pf);
    for (i = 0; i < 100; i++) {
        pack_fwrite(&r->tc_c_data[i], 4, pf);
        pack_fwrite(&r->tc_q_data[i], 4, pf);
        pack_fwrite(&r->tc_t_data[i], 4, pf);
        pack_fwrite(&r->tc_s_data[i], 4, pf);
        pack_fwrite(&r->tc_f_data[i], 4, pf);
    }
    if (r->size > 0) {
        i = 0;
        do {
            pack_fwrite(&r->data[i].cycle_count, 4, pf);
            pack_fwrite(&r->data[i].key_flags, 1, pf);
            i++;
        } while (i < r->size);
    }
    pack_fclose(pf);
    return 0;
}
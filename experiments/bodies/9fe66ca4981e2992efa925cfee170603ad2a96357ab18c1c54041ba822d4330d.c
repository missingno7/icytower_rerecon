{
    Treplay_post *a;
    Treplay_post *b;
    int av, bv;

    a = (Treplay_post *)c;
    b = (Treplay_post *)d;
    if (a->directory != b->directory) {
        if (a->directory == 1)
            return -1;
        return 1;
    }
    /* The original forms this as an unsigned interval: modes 2 through 4
     * select a replay property, while every other value sorts by pathname. */
    if (!a->directory && sort_method >= 2 && sort_method <= 4) {
        av = get_replay_property(a->full_path, sort_method);
        bv = get_replay_property(b->full_path, sort_method);
        if (av <= bv)
            return 1;
        return -1;
    }
    return stricmp(a->full_path, b->full_path);
}
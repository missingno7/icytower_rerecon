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
    if (a->directory || (unsigned int)(sort_method - 2) > 2U)
        return stricmp(a->full_path, b->full_path);
    av = get_replay_property(a->full_path, sort_method);
    bv = get_replay_property(b->full_path, sort_method);
    if (av > bv)
        return -1;
    return 1;
}
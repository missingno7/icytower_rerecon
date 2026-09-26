{
    Treplay_post *a;
    Treplay_post *b;
    int av, bv;

    a = (Treplay_post *)c;
    b = (Treplay_post *)d;
    if (a->directory == b->directory) {
    if (!a->directory) {
        if (sort_method >= 2 && sort_method <= 4) {
            av = get_replay_property(a->full_path, sort_method);
            bv = get_replay_property(b->full_path, sort_method);
            return av > bv ? -1 : 1;
        }
    }
    return stricmp(a->full_path, b->full_path);
    }
    if (a->directory == 1)
        return -1;
    return 1;
}
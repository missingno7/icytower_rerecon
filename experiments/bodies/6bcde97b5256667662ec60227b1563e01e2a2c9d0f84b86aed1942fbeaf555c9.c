{
    Treplay_post *a;
    Treplay_post *b;
    int av, bv;

    a = (Treplay_post *)c;
    b = (Treplay_post *)d;
    if (a->directory != b->directory)
        return a->directory - 1 ? 1 : -1;

    if (!a->directory)
        goto property_check;

compare_path:
    return stricmp(a->full_path, b->full_path);

property_check:
    switch (sort_method) {
    case 2:
    case 3:
    case 4:
        av = get_replay_property(a->full_path, sort_method);
        bv = get_replay_property(b->full_path, sort_method);
        if (av <= bv)
            return 1;
        return -1;
    default:
        goto compare_path;
    }
}
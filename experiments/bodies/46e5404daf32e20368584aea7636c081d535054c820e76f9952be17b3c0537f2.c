{
    Treplay_post *a;
    Treplay_post *b;
    int av, bv;

    a = (Treplay_post *)c;
    b = (Treplay_post *)d;
    if (a->directory != b->directory)
        goto directory_mismatch;
    if (!a->directory)
        goto property_check;
compare_path:
    return stricmp(a->full_path, b->full_path);
property_check:
    if ((unsigned int)(sort_method - 2) > 2U)
        goto compare_path;
    av = get_replay_property(a->full_path, sort_method);
    bv = get_replay_property(b->full_path, sort_method);
    if (av > bv)
        return -1;
    return 1;
directory_mismatch:
    return a->directory - 1 ? 1 : -1;
}
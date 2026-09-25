{
    Treplay *r;
    int i;

    r = malloc(sizeof(Treplay));
    if (!r)
        return 0;
    memcpy(r->header, "ITR140", 6);
    r->comment[0] = 0;
    r->size = size;
    r->combo = 0;
    r->floor = 0;
    r->score = 0;
    for (i = 0; i < 32; i++)
        r->name[i] = 0;
    for (i = 0; i < 32; i++)
        r->name[i] = 0;
    strcpy(r->name, "Harold");
    strcpy(r->date, "no date");
    r->data = malloc(r->size * sizeof(Treplay_data) + 32);
    if (!r->data) {
        free(r);
        return 0;
    }
    for (i = 0; i < size; i++) {
        r->data[i].key_flags = 0;
        r->data[i].cycle_count = 0;
    }
    return r;
}
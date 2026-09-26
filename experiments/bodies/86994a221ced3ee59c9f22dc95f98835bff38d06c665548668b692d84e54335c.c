{
    static int count;
    char *name;
    Tcharacter *call_slot;

    name = get_filename(filename);
    if ((attrib & FA_DIREC) && *name != '.') {
        char buf[1024];

        sprintf(buf, "%s/%s.txt", filename, name);
        if (exists(buf)) {
            call_slot = &characters[count];
            characters[count].bmp = load_character_bmp(name,
                &call_slot->uses_datafile, call_slot->pal);
            log2file(" %s (%s): %s", name, buf,
                characters[count].bmp ? "ok" : "error");
            if (!characters[count].bmp) {
                num_chars--;
                *allegro_errno = 0;
                return 0;
            }
            strcpy(characters[count].name, name);
            count++;
        }
    }
    return 0;
}
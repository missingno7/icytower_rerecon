{
    static int count;
    char *name;

    name = get_filename(filename);
    if ((attrib & FA_DIREC) && *name != '.') {
        char buf[1024];

        sprintf(buf, "%s/%s.txt", filename, name);
        if (exists(buf)) {
            characters[count].bmp = load_character_bmp(name,
                &characters[count].uses_datafile, characters[count].pal);
            log2file(" %s (%s): %s", name, buf,
                characters[count].bmp ? "ok" : "error");
            {
                Tcharacter *record = &characters[count];
                if (!record->bmp) {
                    num_chars--;
                    *allegro_errno = 0;
                    return 0;
                }
                strcpy(record->name, name);
            }
            count++;
        }
    }
    return 0;
}
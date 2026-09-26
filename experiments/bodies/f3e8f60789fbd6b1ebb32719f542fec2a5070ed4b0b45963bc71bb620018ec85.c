{
    static int count;
    char *name;
    Tcharacter *character;

    name = get_filename(filename);
    if ((attrib & FA_DIREC) && *name != '.') {
        char buf[1024];

        sprintf(buf, "%s/%s.txt", filename, name);
        if (exists(buf)) {
            characters[count].bmp = load_character_bmp(name,
                &characters[count].uses_datafile, characters[count].pal);
            character = &characters[count];
            log2file(" %s (%s): %s", name, buf,
                character->bmp ? "ok" : "error");
            character = &characters[count];
            if (!character->bmp) {
                num_chars--;
                *allegro_errno = 0;
                return 0;
            }
            strcpy(character->name, name);
            count++;
        }
    }
    return 0;
}
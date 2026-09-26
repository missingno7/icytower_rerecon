{
    static int count;
    char *name;
    BITMAP **bitmap;

    name = get_filename(filename);
    if ((attrib & FA_DIREC) && *name != '.') {
        char buf[1024];

        sprintf(buf, "%s/%s.txt", filename, name);
        if (exists(buf)) {
            bitmap = &characters[count].bmp;
            *bitmap = load_character_bmp(name,
                &characters[count].uses_datafile, characters[count].pal);
            log2file(" %s (%s): %s", name, buf,
                *bitmap ? "ok" : "error");
            bitmap = &characters[count].bmp;
            if (!*bitmap) {
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
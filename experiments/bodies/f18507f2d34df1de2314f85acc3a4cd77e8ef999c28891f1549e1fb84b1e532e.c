{
    static int count;
    char *name;
    BITMAP **bitmap_for_log;
    BITMAP **bitmap_after_log;

    name = get_filename(filename);
    if ((attrib & FA_DIREC) && *name != '.') {
        char buf[1024];

        sprintf(buf, "%s/%s.txt", filename, name);
        if (exists(buf)) {
            bitmap_for_log = &characters[count].bmp;
            *bitmap_for_log = load_character_bmp(name,
                &characters[count].uses_datafile, characters[count].pal);
            log2file(" %s (%s): %s", name, buf,
                *bitmap_for_log ? "ok" : "error");
            bitmap_after_log = &characters[count].bmp;
            if (!*bitmap_after_log) {
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
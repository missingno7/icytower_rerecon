{
    static int count;
    char *name;

    name = get_filename(filename);
    if ((attrib & FA_DIREC) && *name != '.') {
        char buf[1024];

        sprintf(buf, "%s/%s.txt", filename, name);
        if (exists(buf)) {
            int *use_datafile = &characters[count].uses_datafile;
            RGB *palette = characters[count].pal;
            characters[count].bmp = load_character_bmp(name,
                use_datafile, palette);
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
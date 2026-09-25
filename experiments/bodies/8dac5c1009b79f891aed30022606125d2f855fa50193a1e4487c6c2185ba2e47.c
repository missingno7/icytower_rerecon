{
    static int count;
    char *name;

    name = get_filename(filename);
    if (!(attrib & FA_DIREC) || *name == '.')
        return 0;
    {
        char buf[1024];

        sprintf(buf, "%s/%s.txt", filename, name);
        if (exists(buf)) {
            characters[count].bmp = load_character_bmp(name,
                &characters[count].uses_datafile, characters[count].pal);
            log2file(" %s (%s): %s", name, filename,
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
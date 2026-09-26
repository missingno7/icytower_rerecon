{
    static int count;
    char *name;
    int index;

    name = get_filename(filename);
    if ((attrib & FA_DIREC) && *name != '.') {
        char buf[1024];

        sprintf(buf, "%s/%s.txt", filename, name);
        if (exists(buf)) {
            index = count;
            characters[index].bmp = load_character_bmp(name,
                &characters[index].uses_datafile, characters[index].pal);
            index = count;
            log2file(" %s (%s): %s", name, buf,
                characters[index].bmp ? "ok" : "error");
            index = count;
            if (!characters[index].bmp) {
                num_chars--;
                *allegro_errno = 0;
                return 0;
            }
            index = count;
            strcpy(characters[index].name, name);
            count++;
        }
    }
    return 0;
}
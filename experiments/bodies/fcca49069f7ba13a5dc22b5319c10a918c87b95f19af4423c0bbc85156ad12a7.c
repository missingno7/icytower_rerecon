{
    int length;
    char *name;
    int res;

    length = strlen(filename) + 10;
    name = get_filename(filename);
    if (!stricmp(name, "."))
        return 0;

    /* The ordinary replay case is the fallthrough body; directories leave
     * the main path and share the callback return after it. */
    if (!(attrib & FA_DIREC)) {
        if (stricmp(get_extension(filename), "itr"))
            return 0;
        itr_file_list[num_itr_files].full_path = malloc(length);
        res = get_replay_property(filename, 0);
        if (res < 0) {
            if (res == -1 || res == -1000)
                return 0;
            itr_file_list[num_itr_files].version = -1000 - res;
        }
        strcpy(itr_file_list[num_itr_files].full_path, filename);
        itr_file_list[num_itr_files].directory = 0;
        num_itr_files++;
        return 0;
    }

    itr_file_list[num_itr_files].full_path = malloc(length);
    strcpy(itr_file_list[num_itr_files].full_path, filename);
    itr_file_list[num_itr_files].directory = 1;
    if (!strncmp(name, "..", 3))
        itr_file_list[num_itr_files].parent = 1;
    num_itr_files++;
    return 0;
}
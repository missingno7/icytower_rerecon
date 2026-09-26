{
    int length;
    char *name;

    length = strlen(filename) + 10;
    name = get_filename(filename);
    if (!stricmp(name, "."))
        return 0;
    if (attrib & FA_DIREC) {
        int slot = num_itr_files;
        itr_file_list[slot].full_path = malloc(length);
        strcpy(itr_file_list[slot].full_path, filename);
        itr_file_list[slot].directory = 1;
        if (!strncmp(name, "..", 3))
            itr_file_list[slot].parent = 1;
        num_itr_files++;
        return 0;
    }
    if (stricmp(get_extension(filename), "itr"))
        return 0;
    {
        int res;
        itr_file_list[num_itr_files].full_path = malloc(length);
        res = get_replay_property(filename, 0);
        if (res < 0)
            goto bad_replay;
copy_replay:
        strcpy(itr_file_list[num_itr_files].full_path, filename);
        itr_file_list[num_itr_files].directory = 0;
        num_itr_files++;
        return 0;
bad_replay:
        if (res == -1 || res == -1000)
            return 0;
        itr_file_list[num_itr_files].version = -1000 - res;
        goto copy_replay;
    }
}
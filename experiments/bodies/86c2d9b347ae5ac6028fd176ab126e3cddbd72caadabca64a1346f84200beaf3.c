{
    int length = strlen(filename) + 10;
    char *name = get_filename(filename);
    int res;

    if (!stricmp(name, "."))
        return 0;
    if (!(attrib & FA_DIREC))
        if (stricmp(get_extension(filename), "itr"))
            return 0;
    itr_file_list[num_itr_files].full_path = malloc(length);
    if (attrib & FA_DIREC) {
            strcpy(itr_file_list[num_itr_files].full_path, filename);
            itr_file_list[num_itr_files].directory = 1;
            if (!strcmp(name, ".."))
                itr_file_list[num_itr_files].parent = 1;
    }
    else {
            res = get_replay_property(filename, 0);
            if (res < 0) {
                if (res == -1 || res == -1000)
                    return 0;
                itr_file_list[num_itr_files].version = -1000 - res;
            }
            strcpy(itr_file_list[num_itr_files].full_path, filename);
            itr_file_list[num_itr_files].directory = 0;
    }
    num_itr_files++;
    return 0;
}
{
    int length;
    char *name;

    length = strlen(filename) + 10;
    name = get_filename(filename);
    if (!stricmp(name, "."))
        goto done;
    if (attrib & FA_DIREC) {
        itr_file_list[num_itr_files].full_path = malloc(length);
        strcpy(itr_file_list[num_itr_files].full_path, filename);
        itr_file_list[num_itr_files].directory = 1;
        if (!strncmp(name, "..", 3))
            itr_file_list[num_itr_files].parent = 1;
        num_itr_files++;
    } else {
        if (stricmp(get_extension(filename), "itr"))
            goto done;
        itr_file_list[num_itr_files].full_path = malloc(length);
        {
            int res;
            res = get_replay_property(filename, 0);
            if (res < 0)
                goto bad_replay;
copy_replay:
            strcpy(itr_file_list[num_itr_files].full_path, filename);
            itr_file_list[num_itr_files].directory = 0;
            num_itr_files++;
            goto done;
bad_replay:
            if (res == -1 || res == -1000)
                goto done;
            itr_file_list[num_itr_files].version = -1000 - res;
            goto copy_replay;
        }
    }
done:
    return 0;
}
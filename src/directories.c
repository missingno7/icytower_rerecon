/* Historical directory policy, recovered from this CU's DWARF and code. */
#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include "directories.h"

int get_profiles_dir(char *buffer, size_t buflen)
{
    strncpy(buffer, "profiles", buflen);
    return 1;
}
int get_custom_characters_dir(char *buffer, size_t buflen)
{
    buffer[0] = 0;
    return 0;
}
int get_logfile_path(char *buffer, size_t buflen)
{
    strncpy(buffer, "log.txt", buflen);
    return 1;
}
int get_configfile_path(char *buffer, size_t buflen)
{
    strncpy(buffer, "tower.cfg", buflen);
    return 1;
}
int get_character_dir(char *buffer, size_t buflen, const char *charactername)
{
    snprintf(buffer, buflen, "characters/%s/", charactername);
    return file_exists(buffer, FA_DIREC, NULL);
}
int get_adcache_dir(char *buffer, size_t buflen)
{
    strncpy(buffer, "cache/", buflen);
    return 1;
}
int get_profile_dir_for_profile(char *buffer, size_t buflen, const char *profile)
{
    get_profiles_dir(buffer, buflen);
    sprintf(buffer, "%s/%s/", buffer, profile);
    return 1;
}

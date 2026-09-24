#ifndef ICYTOWER_DIRECTORIES_H
#define ICYTOWER_DIRECTORIES_H
#include <stddef.h>
int get_profiles_dir(char *buffer, size_t buflen);
int get_custom_characters_dir(char *buffer, size_t buflen);
int get_logfile_path(char *buffer, size_t buflen);
int get_configfile_path(char *buffer, size_t buflen);
int get_character_dir(char *buffer, size_t buflen, const char *charactername);
int get_adcache_dir(char *buffer, size_t buflen);
int get_profile_dir_for_profile(char *buffer, size_t buflen, const char *profile);
#endif

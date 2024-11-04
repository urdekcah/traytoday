#ifndef __TRAYTODAY_FILE_H__
#define __TRAYTODAY_FILE_H__
#include <stdbool.h>
#include "traytoday.h"

#define TRAYTODAY_ROOT_DIR "~/.fishydino/.traytoday"
#define CONFIG_PATH TRAYTODAY_ROOT_DIR "/config.dat"

#define FILE_HEADER "\x22\x11\x24"
#define FILE_FOOTER "\x22\x12\x13"
#define SECTION_SEPARATOR "\x19\x20"
#define VERSION_SEPARATOR "\x21"
#define SETTINGS_SEPARATOR "\x22"
#define DATA_SEPARATOR "\x23"

char* expand_home_directory(const char* path);
int ensure_directory_exist(const char *dp);
int ensure_file_exist(const char *fp);
bool file_exists(const char *fp);

int write_setting_file(const char *filename, struct Version version, struct Settings settings);
int read_setting_file(const char *filename, struct Version *version, struct Settings *settings);
#endif // __TRAYTODAY_FILE_H__
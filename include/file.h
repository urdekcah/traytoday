#ifndef __TRAYTODAY_FILE_H__
#define __TRAYTODAY_FILE_H__
#include "traytoday.h"

#define FILE_HEADER "\x22\x11\x24"
#define FILE_FOOTER "\x22\x12\x13"
#define SECTION_SEPARATOR "\x19\x20"
#define VERSION_SEPARATOR "\x21"
#define SETTINGS_SEPARATOR "\x22"
#define DATA_SEPARATOR "\x23"

int write_setting_file(const char *filename, struct Version version, struct Settings settings);
int read_setting_file(const char *filename, struct Version *version, struct Settings *settings);
#endif // __TRAYTODAY_FILE_H__
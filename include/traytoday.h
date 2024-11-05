#ifndef __TRAYTODAY_TRAYTODAY_H__
#define __TRAYTODAY_TRAYTODAY_H__
#include <stdint.h>

#define API_URL_SCHOOLINFO "https://open.neis.go.kr/hub/schoolInfo"
#define API_URL_MEALSERVICE "https://open.neis.go.kr/hub/mealServiceDietInfo"

struct Version {
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
  uint8_t build;
};

struct Settings {
  char language[3];
  char edu_code[10];
  char school_code[10];
};

#define VERSION (struct Version) { 0, 1, 0, 3 }
#endif // __TRAYTODAY_TRAYTODAY_H__
#include <stdio.h>
#include <time.h>
#include "utils/time.h"

void get_current_time(char *buffer, size_t bufferSize) {
  time_t now = time(NULL);
  struct tm *localTime = localtime(&now);

  snprintf(buffer, bufferSize, "%04d%02d%02d", 
           localTime->tm_year + 1900,
           localTime->tm_mon + 1,
           localTime->tm_mday);
}
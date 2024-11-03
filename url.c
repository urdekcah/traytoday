#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "traytoday.h"
#include "utils/url.h"

char* url_encode(const char* str) {
  if (!str) return NULL;
  
  const char hex[] = "0123456789ABCDEF";
  size_t len = strlen(str);
  char* encoded = malloc(len * URL_ENCODE_FACTOR + 1);
  
  if (!encoded) return NULL;
  
  size_t j = 0;
  for (size_t i = 0; i < len; i++) {
    unsigned char c = (unsigned char)str[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded[j++] = (char)c;
    } else if (c == ' ') {
      encoded[j++] = '+';
    } else {
      encoded[j++] = '%';
      encoded[j++] = hex[c >> 4];
      encoded[j++] = hex[c & 15];
    }
  }
  encoded[j] = '\0';
  return encoded;
}
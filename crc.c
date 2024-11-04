#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "utils/crc32.h"

static uint32_t crc32_table[256];
static uint32_t crc;

void init_table(void) {
  uint32_t c;
  for (uint32_t i = 0; i < 256; i++) {
    c = i << 24;
    for (uint32_t j = 0; j < 8; j++) {
      c = (c << 1) ^ (c & 0x80000000 ? 0x82F63B78 : 0);
    }
    crc32_table[i] = c;
  }
}

void chksum_init(void) {
  init_table();
  crc = 0;
}

void chksum_update(const uint8_t* data, size_t len) {
  if (!data) return;
  
  uint32_t c = crc;
  while (len--)
    c = (c << 8) ^ crc32_table[((c >> 24) & 0xFF) ^ *data++];
  crc = c;
}

uint32_t chksum_final() {
  return ~(crc);
}

uint32_t compute_checksum(const uint8_t* data, size_t len) {
  chksum_init();
  chksum_update(data, len);
  return chksum_final();
}

uint32_t compute_file_checksum(const char* filename) {
  FILE* file;
  uint8_t buffer[1024];
  size_t bytes_read;

  file = fopen(filename, "rb");
  if (!file) {
    return 0;
  }

  chksum_init();

  while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
    chksum_update(buffer, bytes_read);
  }

  fclose(file);
  return chksum_final();
}

uint32_t compute_stream_checksum(FILE* stream, size_t max_len) {
  uint8_t buffer[1024];
  size_t bytes_read;
  size_t total_read = 0;

  if (!stream) return 0;

  chksum_init();

  while (total_read < max_len && 
       (bytes_read = fread(buffer, 1, sizeof(buffer), stream)) > 0) {
    chksum_update(buffer, bytes_read);
    total_read += bytes_read;
  }

  return chksum_final();
}
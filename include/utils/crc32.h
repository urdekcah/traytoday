#ifndef __TRAYTODAY_UTILS_CRC32_H__
#define __TRAYTODAY_UTILS_CRC32_H__
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

void init_table(void);
void chksum_init(void);
void chksum_update(const uint8_t* data, size_t len);
uint32_t chksum_final();
uint32_t compute_checksum(const uint8_t* data, size_t len);
uint32_t compute_file_checksum(const char* filename);
uint32_t compute_stream_checksum(FILE* stream, size_t max_len);
#endif // __TRAYTODAY_UTILS_CRC32_H__
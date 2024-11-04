#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "traytoday.h"
#include "error.h"
#include "utils/crc32.h"
#include "file.h"

static void encode_data(unsigned char *data, size_t len, uint32_t key) {
  for (size_t i = 0; i < len; i++)
    data[i] = data[i] ^ ((key >> (i % 32)) & 0xFF);
}

int write_setting_file(const char *filename, struct Version version, struct Settings settings) {
  if (!filename) return ERRCODE_NULL_POINTER;
  
  FILE *fp;
  fp = fopen(filename, "wb");
  if (!fp) return ERRCODE_FILE_OPEN_FAILED;

  if (fwrite(FILE_HEADER, 1, 3, fp) != 3) {
    fclose(fp);
    return ERRCODE_FILE_WRITE_FAILED;
  }
  
  if (fwrite(SECTION_SEPARATOR, 1, 2, fp) != 2 ||
    fwrite(VERSION_SEPARATOR, 1, 1, fp) != 1) {
    fclose(fp);
    return ERRCODE_FILE_WRITE_FAILED;
  }
  
  unsigned char version_data[4] = {
    version.major, version.minor, version.patch, version.build
  };
  if (fwrite(version_data, 1, 4, fp) != 4) {
    fclose(fp);
    return ERRCODE_FILE_WRITE_FAILED;
  }
  
  if (fwrite(SECTION_SEPARATOR, 1, 2, fp) != 2 ||
    fwrite(SETTINGS_SEPARATOR, 1, 1, fp) != 1) {
    fclose(fp);
    return ERRCODE_FILE_WRITE_FAILED;
  }
  
  unsigned char settings_data[22];
  memcpy(settings_data, settings.language, 2);
  memcpy(settings_data + 2, settings.edu_code, 10);
  memcpy(settings_data + 12, settings.school_code, 10);
  
  uint32_t key = compute_checksum(version_data, 4);
  encode_data(settings_data, sizeof(settings_data), key);
  
  if (fwrite(settings_data, 1, sizeof(settings_data), fp) != sizeof(settings_data)) {
    fclose(fp);
    return ERRCODE_FILE_WRITE_FAILED;
  }
  
  uint32_t crc = compute_checksum(settings_data, sizeof(settings_data));
  if (fwrite(&crc, 1, sizeof(crc), fp) != sizeof(crc)) {
    fclose(fp);
    return ERRCODE_FILE_WRITE_FAILED;
  }
  
  if (fwrite(SECTION_SEPARATOR, 1, 2, fp) != 2 ||
    fwrite(DATA_SEPARATOR, 1, 1, fp) != 1 ||
    fwrite(FILE_FOOTER, 1, 3, fp) != 3) {
    fclose(fp);
    return ERRCODE_FILE_WRITE_FAILED;
  }
  
  fclose(fp);
  return ERRCODE_OK;
}

int read_setting_file(const char *filename, struct Version *version, struct Settings *settings) {
  if (!filename || !version || !settings) return ERRCODE_NULL_POINTER;
  
  FILE *fp;
  fp = fopen(filename, "rb");
  if (!fp) return ERRCODE_FILE_OPEN_FAILED;
  
  char header[3];
  if (fread(header, 1, 3, fp) != 3 || memcmp(header, FILE_HEADER, 3) != 0) {
    fclose(fp);
    return ERRCODE_INVALID_HEADER;
  }
  
  char separator[3];
  if (fread(separator, 1, 3, fp) != 3 || 
    memcmp(separator, SECTION_SEPARATOR VERSION_SEPARATOR, 3) != 0) {
    fclose(fp);
    return ERRCODE_INVALID_VERSION;
  }
  
  unsigned char version_data[4];
  if (fread(version_data, 1, 4, fp) != 4) {
    fclose(fp);
    return ERRCODE_INVALID_VERSION;
  }
  version->major = version_data[0];
  version->minor = version_data[1];
  version->patch = version_data[2];
  version->build = version_data[3];
  
  if (fread(separator, 1, 3, fp) != 3 || 
    memcmp(separator, SECTION_SEPARATOR SETTINGS_SEPARATOR, 3) != 0) {
    fclose(fp);
    return ERRCODE_INVALID_SETTINGS;
  }
  
  unsigned char settings_data[22];
  if (fread(settings_data, 1, sizeof(settings_data), fp) != sizeof(settings_data)) {
    fclose(fp);
    return ERRCODE_INVALID_SETTINGS;
  }
  
  uint32_t stored_crc, calculated_crc;
  if (fread(&stored_crc, 1, sizeof(stored_crc), fp) != sizeof(stored_crc)) {
    fclose(fp);
    return ERRCODE_INVALID_CHECKSUM;
  }
  
  calculated_crc = compute_checksum(settings_data, sizeof(settings_data));
  if (stored_crc != calculated_crc) {
    fclose(fp);
    return ERRCODE_INVALID_CHECKSUM;
  }
  
  uint32_t key = compute_checksum(version_data, 4);
  encode_data(settings_data, sizeof(settings_data), key);
  
  memcpy(settings->language, settings_data, 2);
  memcpy(settings->edu_code, settings_data + 2, 10);
  memcpy(settings->school_code, settings_data + 12, 10);
  
  char footer_separator[3];
  char footer[3];
  if (fread(footer_separator, 1, 3, fp) != 3 || 
    fread(footer, 1, 3, fp) != 3 ||
    memcmp(footer_separator, SECTION_SEPARATOR DATA_SEPARATOR, 3) != 0 ||
    memcmp(footer, FILE_FOOTER, 3) != 0) {
    fclose(fp);
    return ERRCODE_INVALID_FOOTER;
  }
  
  fclose(fp);
  return ERRCODE_OK;
}
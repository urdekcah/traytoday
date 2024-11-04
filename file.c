#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "traytoday.h"
#include "error.h"
#include "utils/crc32.h"
#include "file.h"

char* expand_home_directory(const char* path) {
  if (path[0] == '~') {
    const char* home = getenv("HOME");
    if (home == NULL) {
      fprintf(stderr, "Could not get HOME environment variable.");
      return NULL;
    }

    size_t len = strlen(home) + strlen(path); 
    char* expanded_path = (char*)malloc(len);

    snprintf(expanded_path, len, "%s%s", home, path + 1); 
    return expanded_path;
  }
  return strdup(path);
}

int ensure_directory_exist(const char *dp) {
  struct stat st;
  char* expanded_path = expand_home_directory(dp);

  if (stat(expanded_path, &st) == 0 && S_ISDIR(st.st_mode)) {
    free(expanded_path);
    return 0;
  }

  char *parent_path = strdup(expanded_path);
  char *last_slash = strrchr(parent_path, '/');
  
  if (last_slash != NULL && last_slash != parent_path) {
    *last_slash = '\0';
    ensure_directory_exist(parent_path);
  }

  if (mkdir(expanded_path, 0755) != 0) {
    if (errno != EEXIST) {
      perror("mkdir failed");
      free(parent_path);
      free(expanded_path);
      return -1;
    }
  }

  free(parent_path);
  free(expanded_path);
  return 0;
}

int ensure_file_exist(const char *fp) {
  char* expanded_path = expand_home_directory(fp);
  int fd = open(expanded_path, O_CREAT | O_EXCL, 0644);
  if (fd == -1) {
    if (errno == EEXIST) {
      return 0;
    } else {
      perror("open failed");
      return -1;
    }
  }
  close(fd);
  free(expanded_path);
  return 0;
}

bool file_exists(const char *fp) {
  struct stat st;
  char* expanded_path = expand_home_directory(fp);
  bool exists = stat(expanded_path, &st) == 0;
  free(expanded_path);
  return exists;
}

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
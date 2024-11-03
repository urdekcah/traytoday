#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "traytoday.h"
#include "buffer.h"
#include "client.h"

char* get_json_string(struct json_object* obj, const char* key) {
  struct json_object* str_obj = json_object_object_get(obj, key);
  return str_obj ? strdup(json_object_get_string(str_obj)) : NULL;
}

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
  struct Buffer* buf = (struct Buffer*)userp;
  size_t new_size = buf->size + size * nmemb;
  
  if (buffer_resize(buf, new_size + 1) != 0) {
    return 0;
  }
  
  memcpy(buf->buffer + buf->size, contents, size * nmemb);
  buf->size = new_size;
  buf->buffer[buf->size] = '\0';
  
  return size * nmemb;
}
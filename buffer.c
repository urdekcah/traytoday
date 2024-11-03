#include <stdio.h>
#include <stdlib.h>
#include "traytoday.h"
#include "buffer.h"

struct Buffer* buffer__new(void) {
  struct Buffer* buf = malloc(sizeof(struct Buffer));
  if (!buf) return NULL;
  
  buf->buffer = malloc(INITIAL_BUFFER_SIZE);
  if (!buf->buffer) {
    free(buf);
    return NULL;
  }
  
  buf->buffer[0] = '\0';
  buf->size = 0;
  buf->capacity = INITIAL_BUFFER_SIZE;
  return buf;
}

void buffer_free(struct Buffer* buf) {
  if (buf) {
    free(buf->buffer);
    free(buf);
  }
}

int buffer_resize(struct Buffer* buf, size_t new_size) {
  size_t new_capacity = buf->capacity;
  while (new_capacity < new_size) {
    new_capacity *= 2;
  }
  
  if (new_capacity != buf->capacity) {
    char* new_buffer = realloc(buf->buffer, new_capacity);
    if (!new_buffer) return -1;
    
    buf->buffer = new_buffer;
    buf->capacity = new_capacity;
  }
  return 0;
}
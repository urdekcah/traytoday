#ifndef __TRAYTODAY_BUFFER_H__
#define __TRAYTODAY_BUFFER_H__
#include <stddef.h>
#define INITIAL_BUFFER_SIZE 1024

struct Buffer {
  char* buffer;
  size_t size;
  size_t capacity;
};

struct Buffer* buffer__new(void);
void buffer_free(struct Buffer* buf);
int buffer_resize(struct Buffer* buf, size_t new_size);
#endif // __TRAYTODAY_BUFFER_H__
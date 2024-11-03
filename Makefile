CC := gcc
CFLAGS := -O3 -g0 -fvisibility=hidden -Wl,--strip-all -Wall -Wextra -Wconversion -Wnull-dereference -Wshadow -Wlogical-op -Wuninitialized -fstrict-aliasing -Werror -Iinclude
LDFLAGS := -lcurl -ljson-c -Wl,--strip-all,--warn-common

TARGET := traytoday
SRCS := $(wildcard *.c) $(wildcard builtin/*.c)
SRCS := $(filter-out lex.yy.c, $(SRCS))
OBJS := $(SRCS:.c=.o)

.PHONY: all clean build ochistka

all: build ochistka

build: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

ochistka:
	@echo "Cleaning up object files..."
	@rm -f $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)

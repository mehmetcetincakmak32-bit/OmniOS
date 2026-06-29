CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
AR = ar
ARFLAGS = rcs

SRCS = state_machine.c process_manager.c gesture_engine.c memory.c runtime_loader.c
OBJS = $(SRCS:.c=.o)
TARGET = libomnios_core.a

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c include/omnios_core.h
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	$(CC) $(CFLAGS) test_core.c -L. -lomnios_core -o test_core
	./test_core

clean:
	rm -f $(OBJS) $(TARGET) test_core test_core.exe

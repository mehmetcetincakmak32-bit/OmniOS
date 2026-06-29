CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
AR = ar
ARFLAGS = rcs

SRCS = state_machine.c process_manager.c gesture_engine.c memory.c runtime_loader.c api_translator.c security.c
OBJS = $(SRCS:.c=.o)
TARGET = libomnios_core.a
TEST_SRCS = test_core.c
TEST_TARGET = test_core

.PHONY: all clean test

all: $(TARGET) $(TEST_TARGET)

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(TEST_TARGET): $(TEST_SRCS) $(TARGET)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRCS) -L. -lomnios_core

%.o: %.c include/omnios_core.h
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(TEST_TARGET)

CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude -ffreestanding -nostdlib
LDFLAGS = -ffreestanding -nostdlib -lgcc

SRCS = kernel_main.c scheduler.c syscalls.c timer.c \
       mm/memory.c ipc/ipc.c fs/vfs.c drivers/driver_model.c net/network.c
OBJS = $(SRCS:.c=.o)
TARGET = omnios_kernel.bin

TEST_SRCS = tests/test_kernel.c
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_TARGET = test_kernel

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -T linker.ld -o $@ $^ $(LDFLAGS)

%.o: %.c include/omnios_kernel.h
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_OBJS) $(OBJS)
	$(CC) $(CFLAGS) -I. $(TEST_OBJS) $(OBJS) -o $(TEST_TARGET) -lm
	./$(TEST_TARGET)

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET) $(TEST_TARGET).exe

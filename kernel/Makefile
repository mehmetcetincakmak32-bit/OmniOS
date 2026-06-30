KERNEL_SRC ?= .
KERNEL_OUT ?= $(KERNEL_SRC)/out

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

.PHONY: all clean test mobile_defconfig kernelconfigcheck

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -T linker.ld -o $@ $^ $(LDFLAGS)

%.o: %.c include/omnios_kernel.h
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_OBJS) $(OBJS)
	$(CC) $(CFLAGS) -I. $(TEST_OBJS) $(OBJS) -o $(TEST_TARGET) -lm
	./$(TEST_TARGET)

mobile_defconfig:
	@echo "Creating mobile defconfig..."
	@mkdir -p $(KERNEL_OUT)
	@cp $(KERNEL_SRC)/configs/omnios_mobile_defconfig $(KERNEL_OUT)/.config
	@echo "Config written to $(KERNEL_OUT)/.config"

kernelconfigcheck:
	@if [ -f $(KERNEL_OUT)/.config ]; then \
		$(MAKE) -C $(KERNEL_SRC) O=$(KERNEL_OUT) kconfigcheck; \
	else \
		echo "No .config found. Run 'make mobile_defconfig' first."; \
		exit 1; \
	fi

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET) $(TEST_TARGET).exe
	rm -rf $(KERNEL_OUT)

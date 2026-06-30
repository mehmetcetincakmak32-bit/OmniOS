# OmniOS — kernel/Makefile
# x86_64 SMP kernel build
# SPDX-License-Identifier: MIT

ARCH      ?= x86_64
CC         = gcc
LD         = ld
AS         = as
OBJCOPY    = objcopy

CFLAGS    = -Wall -Wextra -Werror -O2 -m64 -mno-red-zone \
            -ffreestanding -nostdlib -nostartfiles -nodefaultlibs \
            -mcmodel=kernel -mno-sse -mno-mmx -mno-sse2 -mno-3dnow \
            -fno-stack-protector -fomit-frame-pointer \
            -Iinclude -Iarch/$(ARCH) -Imm

LDFLAGS   = -T linker.ld -nostdlib -static -z max-page-size=4096

# Architecture source files
ARCH_SRCS = arch/x86_64/cpu.c arch/x86_64/gdt.c arch/x86_64/idt.c \
            arch/x86_64/apic.c arch/x86_64/paging.c arch/x86_64/smp.c \
            arch/x86_64/interrupts.c arch/x86_64/acpi.c
ARCH_ASM  = arch/x86_64/boot.S

# MM source files
MM_SRCS   = mm/pmm.c mm/vmm.c

# Proc source files
PROC_SRCS = proc/process.c

# Sched source files
SCHED_SRCS = sched/sched.c

# Driver source files
DRV_SRCS  = drivers/pci.c

# Legacy source files (kept for compat)
LEGACY_SRCS = kernel_main.c scheduler.c syscalls.c timer.c \
              mm/memory.c ipc/ipc.c fs/vfs.c drivers/driver_model.c \
              net/network.c hal/mobile_hal.c

OBJS      = $(ARCH_ASM:.S=.o) $(ARCH_SRCS:.c=.o) \
            $(MM_SRCS:.c=.o) $(PROC_SRCS:.c=.o) \
            $(SCHED_SRCS:.c=.o) $(DRV_SRCS:.c=.o) \
            $(LEGACY_SRCS:.c=.o)
TARGET     = omnios-kernel-x86_64.bin

TEST_SRCS  = tests/test_kernel.c
TEST_OBJS  = $(TEST_SRCS:.c=.o)
TEST_TARGET = test_kernel

.PHONY: all clean test mobile_defconfig x86_64_defconfig run_qemu

all: $(TARGET)

$(TARGET): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo "Built: $@"
	@ls -lh $@

%.o: %.S arch/x86_64/cpu.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c include/omnios_kernel.h
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_OBJS) $(OBJS)
	$(CC) $(CFLAGS) -I. $(TEST_OBJS) $(OBJS) -o $(TEST_TARGET) -lm
	./$(TEST_TARGET)

mobile_defconfig:
	@echo "Creating ARM64 mobile defconfig..."
	@mkdir -p out
	@cp configs/omnios_mobile_defconfig out/.config
	@echo "Config written to out/.config"

x86_64_defconfig:
	@echo "Creating x86_64 defconfig..."
	@mkdir -p out
	@cp configs/omnios_x86_64_defconfig out/.config
	@echo "Config written to out/.config"

run_qemu: $(TARGET)
	qemu-system-x86_64 -kernel $(TARGET) -m 2048 -smp 4 \
		-serial stdio -vga virtio -display gtk,gl=on \
		-device virtio-gpu-gl -device virtio-keyboard \
		-device virtio-mouse -device virtio-net,netdev=net0 \
		-netdev user,id=net0 \
		-append "console=ttyS0"

run_qemu_debug: $(TARGET)
	qemu-system-x86_64 -kernel $(TARGET) -m 2048 -smp 4 \
		-serial stdio -vga virtio -display gtk,gl=on \
		-device virtio-gpu-gl -device virtio-keyboard \
		-device virtio-mouse -S -s \
		-append "console=ttyS0"

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET) $(TEST_TARGET).exe
	rm -rf out

# OmniOS — kernel/Makefile
# x86_64 SMP / ARM64 Mobile kernel build
# SPDX-License-Identifier: MIT

ARCH      ?= x86_64
CC         = gcc
LD         = ld
AS         = as
OBJCOPY    = objcopy

ifeq ($(ARCH),arm64)
CFLAGS    = -Wall -Wextra -Werror -O2 -ffreestanding -nostdlib \
            -nostartfiles -nodefaultlibs \
            -fno-stack-protector -fomit-frame-pointer \
            -DCONFIG_ARM64 -Iinclude -Iarch/arm64 -Imm -Ibsp/sm8250 \
            -Ifs -Idrivers -Iproc -Iipc
LDFLAGS   = -T linker_arm64.ld -nostdlib -static
TARGET     = omnios-kernel-arm64.bin
ARCH_SRCS = arch/arm64/arm64.c
ARCH_ASM  = arch/arm64/boot.S
MM_SRCS   = mm/pmm.c mm/vmm.c
PROC_SRCS = proc/process.c
SCHED_SRCS = sched/sched.c
DRV_SRCS  = drivers/pci.c
BSP_SRCS  = bsp/sm8250/sm8250.c \
            bsp/sm8250/drivers/wifi_bt.c \
            bsp/sm8250/drivers/modem_ril.c \
            bsp/sm8250/drivers/sensors.c \
            bsp/sm8250/drivers/power_mgmt.c \
            bsp/sm8250/drivers/usb_otg.c \
            bsp/sm8250/drivers/usb_gadget.c \
            bsp/sm8250/drivers/ufs.c \
            bsp/sm8250/drivers/audio.c \
            bsp/sm8250/drivers/input.c \
            bsp/sm8250/drivers/display_fb.c \
            bsp/sm8250/drivers/rtc.c \
            bsp/sm8250/drivers/nfc.c \
            bsp/sm8250/drivers/fingerprint.c \
            bsp/sm8250/drivers/init.c
FS_SRCS   = fs/elf.c fs/initramfs.c fs/devfs.c fs/procfs.c
TTY_SRCS  = drivers/tty.c
KLOG_SRCS = klog.c
LEGACY_SRCS = kernel_main.c scheduler.c syscall.c timer.c \
              mm/memory.c ipc/ipc.c fs/vfs.c drivers/driver_model.c \
              net/network.c hal/mobile_hal.c
else
CFLAGS    = -Wall -Wextra -Werror -O2 -m64 -mno-red-zone \
            -ffreestanding -nostdlib -nostartfiles -nodefaultlibs \
            -mcmodel=kernel -mno-sse -mno-mmx -mno-sse2 -mno-3dnow \
            -fno-stack-protector -fomit-frame-pointer \
            -DCONFIG_X86_64 -Iinclude -Iarch/x86_64 -Imm -Ifs -Idrivers -Iproc -Iipc
LDFLAGS   = -T linker.ld -nostdlib -static -z max-page-size=4096
TARGET     = omnios-kernel-x86_64.bin
ARCH_SRCS = arch/x86_64/cpu.c arch/x86_64/gdt.c arch/x86_64/idt.c \
            arch/x86_64/apic.c arch/x86_64/paging.c arch/x86_64/smp.c \
            arch/x86_64/interrupts.c arch/x86_64/acpi.c
ARCH_ASM  = arch/x86_64/boot.S
MM_SRCS   = mm/pmm.c mm/vmm.c
PROC_SRCS = proc/process.c
SCHED_SRCS = sched/sched.c
DRV_SRCS  = drivers/pci.c
LEGACY_SRCS = kernel_main.c scheduler.c syscall.c timer.c \
              mm/memory.c ipc/ipc.c fs/vfs.c drivers/driver_model.c \
              net/network.c hal/mobile_hal.c
endif

OBJS      = $(ARCH_ASM:.S=.o) $(ARCH_SRCS:.c=.o) \
            $(MM_SRCS:.c=.o) $(PROC_SRCS:.c=.o) \
            $(SCHED_SRCS:.c=.o) $(DRV_SRCS:.c=.o) \
            $(LEGACY_SRCS:.c=.o)
ifeq ($(ARCH),arm64)
OBJS += $(BSP_SRCS:.c=.o) $(FS_SRCS:.c=.o) $(TTY_SRCS:.c=.o) $(KLOG_SRCS:.c=.o)
endif

TEST_SRCS  = tests/test_kernel.c
TEST_OBJS  = $(TEST_SRCS:.c=.o)
TEST_TARGET = test_kernel

.PHONY: all clean test mobile_defconfig x86_64_defconfig run_qemu run_qemu_arm64

all: initramfs $(TARGET)

initramfs:
ifeq ($(ARCH),arm64)
	@echo "[MAKE] Generating initramfs..."
	@cd ../userspace/init && \
	  aarch64-linux-gnu-gcc -ffreestanding -nostdlib -nostartfiles -nodefaultlibs \
	    -O2 -o init init.c 2>/dev/null; \
	  if [ -f init ]; then \
	    echo "init" | cpio -o -H newc -R 0:0 > ../kernel/initramfs.cpio 2>/dev/null; \
	    echo "[MAKE] initramfs.cpio created"; \
	  else \
	    touch ../kernel/initramfs.cpio; \
	    echo "[MAKE] initramfs.cpio (placeholder)"; \
	  fi
endif

$(TARGET): $(OBJS) $(LINKER_SCRIPT)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo "Built: $@ @ $(ARCH)"
	@ls -lh $@

ifeq ($(ARCH),arm64)
LINKER_SCRIPT = linker_arm64.ld
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
else
LINKER_SCRIPT = linker.ld
%.o: %.S arch/x86_64/cpu.h
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.c include/omnios_kernel.h
	$(CC) $(CFLAGS) -c $< -o $@
endif

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

run_qemu_arm64: $(TARGET)
	qemu-system-aarch64 -machine virt -cpu max -smp 4 -m 2048 \
		-kernel $(TARGET) -serial stdio \
		-display gtk,gl=on -device virtio-gpu-gl \
		-device virtio-keyboard -device virtio-mouse \
		-append "console=ttyAMA0"

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET) $(TEST_TARGET).exe
	rm -rf out

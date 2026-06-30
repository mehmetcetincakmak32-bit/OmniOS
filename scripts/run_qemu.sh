#!/bin/sh
# OmniOS — run_qemu.sh
# Launch OmniOS in QEMU (x86_64 development)
# Faz 1 Foundation
# SPDX-License-Identifier: MIT

set -e

QEMU=qemu-system-x86_64
MEMORY=2048
SMP=2
KERNEL=""
ROOTFS=""
ARCH="x86_64"
HEADLESS=0
DEBUG=0
GDB_PORT=1234

usage() {
    echo "Usage: $0 -k <kernel> -r <rootfs> [-a <arch>] [-m <memory>] [-s <smp>] [-H] [-d] [-g <port>]"
    echo ""
    echo "Options:"
    echo "  -k <kernel>    Path to kernel binary (required)"
    echo "  -r <rootfs>    Path to rootfs directory (required)"
    echo "  -a <arch>      Architecture: x86_64 (default) | aarch64"
    echo "  -m <memory>    RAM in MB (default: 2048)"
    echo "  -s <smp>       CPU count (default: 2)"
    echo "  -H             Headless mode (no display)"
    echo "  -d             Debug mode (wait for GDB on port $GDB_PORT)"
    echo "  -g <port>      GDB port (default: 1234)"
    echo "  -h             Show this help"
    exit 1
}

while getopts "k:r:a:m:s:Hdg:h" opt; do
    case "$opt" in
        k) KERNEL="$OPTARG" ;;
        r) ROOTFS="$OPTARG" ;;
        a) ARCH="$OPTARG" ;;
        m) MEMORY="$OPTARG" ;;
        s) SMP="$OPTARG" ;;
        H) HEADLESS=1 ;;
        d) DEBUG=1 ;;
        g) GDB_PORT="$OPTARG" ;;
        h) usage ;;
        *) usage ;;
    esac
done

if [ -z "$KERNEL" ] || [ -z "$ROOTFS" ]; then
    echo "Error: -k <kernel> and -r <rootfs> are required"
    usage
fi

if [ ! -f "$KERNEL" ]; then
    echo "Error: kernel not found: $KERNEL"
    exit 1
fi

if [ ! -d "$ROOTFS" ]; then
    echo "Error: rootfs directory not found: $ROOTFS"
    exit 1
fi

echo "═══ OmniOS QEMU Launcher ═══"
echo "  Kernel:  $KERNEL"
echo "  Rootfs:  $ROOTFS"
echo "  Arch:    $ARCH"
echo "  Memory:  ${MEMORY}MB"
echo "  CPUs:    $SMP"
echo "  Headless: $([ $HEADLESS -eq 1 ] && echo yes || echo no)"
echo "  Debug:    $([ $DEBUG -eq 1 ] && echo "yes (GDB port $GDB_PORT)" || echo no)"
echo ""

CMD="$QEMU"

# Machine
case "$ARCH" in
    x86_64)
        CMD="$CMD -machine type=q35,accel=kvm:hvf:whpx:tcg"
        CMD="$CMD -cpu host,migratable=off"
        ;;
    aarch64)
        QEMU=qemu-system-aarch64
        CMD="$CMD -machine type=virt,gic-version=3"
        CMD="$CMD -cpu cortex-a72"
        CMD="$CMD -bios /usr/share/qemu-efi-aarch64/QEMU_EFI.fd 2>/dev/null || true"
        ;;
esac

CMD="$CMD -m $MEMORY"
CMD="$CMD -smp $SMP"

# Display
if [ $HEADLESS -eq 1 ]; then
    CMD="$CMD -nographic"
    CMD="$CMD -vga none"
else
    CMD="$CMD -vga virtio"
    CMD="$CMD -display gtk,gl=on"
fi

# VirtIO GPU
CMD="$CMD -device virtio-gpu-gl"

# Storage
CMD="$CMD -drive if=virtio,format=raw,file=$ROOTFS/omnios.img"

# Network
CMD="$CMD -netdev user,id=net0,hostfwd=tcp::2222-:22"
CMD="$CMD -device virtio-net,netdev=net0"

# Serial console
CMD="$CMD -serial mon:stdio"

# Input
CMD="$CMD -device virtio-keyboard"
CMD="$CMD -device virtio-mouse"

# Debug
if [ $DEBUG -eq 1 ]; then
    CMD="$CMD -s -S"
    echo "Waiting for GDB connection on port $GDB_PORT..."
    echo "  Connect with: gdb -ex 'target remote :$GDB_PORT' $KERNEL"
fi

echo "\$ $CMD"
echo ""
eval exec "$CMD"

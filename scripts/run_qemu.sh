#!/bin/sh
# OmniOS — run_qemu.sh
# Launch OmniOS x86_64 SMP kernel in QEMU
# Faz 1 Foundation
# SPDX-License-Identifier: MIT

set -e

QEMU=qemu-system-x86_64
MEMORY=2048
SMP=4
KERNEL=""
ROOTFS=""
ARCH="x86_64"
HEADLESS=0
DEBUG=0
GDB_PORT=1234
KVM=""
UEFI=0

usage() {
    echo "Usage: $0 -k <kernel> [-r <rootfs>] [-m <memory>] [-s <smp>]"
    echo "       [-H] [-d] [-g <port>] [-K] [-U]"
    echo ""
    echo "Options:"
    echo "  -k <kernel>    Path to kernel binary (required)"
    echo "  -r <rootfs>    Path to rootfs directory (optional)"
    echo "  -a <arch>      Architecture: x86_64 (default) | aarch64"
    echo "  -m <memory>    RAM in MB (default: 2048)"
    echo "  -s <smp>       CPU count (default: 4)"
    echo "  -H             Headless mode (no display)"
    echo "  -d             Debug mode (wait for GDB on port $GDB_PORT)"
    echo "  -g <port>      GDB port (default: 1234)"
    echo "  -K             Enable KVM acceleration"
    echo "  -U             Use OVMF UEFI firmware"
    echo "  -h             Show this help"
    exit 1
}

while getopts "k:r:a:m:s:Hg:d:KUh" opt; do
    case "$opt" in
        k) KERNEL="$OPTARG" ;;
        r) ROOTFS="$OPTARG" ;;
        a) ARCH="$OPTARG" ;;
        m) MEMORY="$OPTARG" ;;
        s) SMP="$OPTARG" ;;
        H) HEADLESS=1 ;;
        d) DEBUG=1 ;;
        g) GDB_PORT="$OPTARG" ;;
        K) KVM="-accel kvm" ;;
        U) UEFI=1 ;;
        h) usage ;;
        *) usage ;;
    esac
done

if [ -z "$KERNEL" ]; then
    echo "Error: -k <kernel> is required"
    usage
fi

echo "═══ OmniOS QEMU Launcher (x86_64 SMP) ═══"
echo "  Kernel:  $KERNEL"
echo "  CPUs:    $SMP"
echo "  Memory:  ${MEMORY}MB"
echo "  KVM:     $([ -n "$KVM" ] && echo yes || echo no)"
echo "  UEFI:    $([ $UEFI -eq 1 ] && echo yes || echo no)"
echo ""

CMD="$QEMU $KVM"
CMD="$CMD -machine type=q35,accel=kvm:hvf:whpx:tcg"
CMD="$CMD -cpu host,migratable=off"
CMD="$CMD -m $MEMORY"
CMD="$CMD -smp $SMP"

# UEFI
if [ $UEFI -eq 1 ]; then
    CMD="$CMD -bios /usr/share/ovmf/OVMF.fd 2>/dev/null || \
          $CMD -bios /usr/share/edk2/x64/OVMF.fd 2>/dev/null || true"
fi

# Display
if [ $HEADLESS -eq 1 ]; then
    CMD="$CMD -nographic"
    CMD="$CMD -vga none"
else
    CMD="$CMD -vga virtio"
    CMD="$CMD -display gtk,gl=on"
fi

# Kernel
CMD="$CMD -kernel $KERNEL"
CMD="$CMD -append 'console=ttyS0,115200 root=/dev/vda1'"

# VirtIO devices
CMD="$CMD -device virtio-gpu-gl"
CMD="$CMD -device virtio-keyboard"
CMD="$CMD -device virtio-mouse"
CMD="$CMD -device virtio-net,netdev=net0"
CMD="$CMD -netdev user,id=net0,hostfwd=tcp::2222-:22"

# Storage
if [ -n "$ROOTFS" ]; then
    CMD="$CMD -drive if=virtio,format=raw,file=$ROOTFS/omnios.img"
fi

# Serial
CMD="$CMD -serial stdio"

# Debug
if [ $DEBUG -eq 1 ]; then
    CMD="$CMD -gdb tcp::$GDB_PORT -S"
    echo "Waiting for GDB on port $GDB_PORT..."
fi

# CPU topology
CMD="$CMD -smp $SMP,cores=$((SMP/2)),threads=2,sockets=1"

echo "\$ $CMD"
eval exec "$CMD"

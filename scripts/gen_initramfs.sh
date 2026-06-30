#!/bin/bash
# OmniOS — scripts/gen_initramfs.sh
# Generate cpio initramfs from userspace/init/init binary
# SPDX-License-Identifier: MIT

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
INIT_SRC="${SCRIPT_DIR}/../userspace/init/init.c"
INIT_BIN="${SCRIPT_DIR}/../userspace/init/init"
INITRAMFS="${SCRIPT_DIR}/../kernel/initramfs.cpio"

ARCH="${1:-arm64}"

echo "[GEN_INITRAMFS] Building init binary for ${ARCH}..."

if [ "$ARCH" = "arm64" ]; then
    CROSS="aarch64-linux-gnu-"
    CFLAGS="-ffreestanding -nostdlib -nostartfiles -nodefaultlibs -O2 -mno-red-zone"
elif [ "$ARCH" = "x86_64" ]; then
    CROSS=""
    CFLAGS="-ffreestanding -nostdlib -nostartfiles -nodefaultlibs -O2 -mno-red-zone"
else
    echo "Unknown arch: $ARCH"; exit 1
fi

${CROSS}gcc ${CFLAGS} -o "${INIT_BIN}" "${INIT_SRC}"

if [ ! -f "${INIT_BIN}" ]; then
    echo "[GEN_INITRAMFS] Build failed"; exit 1
fi

echo "[GEN_INITRAMFS] Creating cpio archive..."
cd "${SCRIPT_DIR}/../userspace/init"

# Create cpio archive with init binary
rm -f "${INITRAMFS}"

# Use cpio command to create archive
echo "init" | cpio -o -H newc -R 0:0 > "${INITRAMFS}" 2>/dev/null

if [ -f "${INITRAMFS}" ]; then
    echo "[GEN_INITRAMFS] initramfs: ${INITRAMFS}"
    ls -lh "${INITRAMFS}"
else
    echo "[GEN_INITRAMFS] Failed to create initramfs"
    exit 1
fi

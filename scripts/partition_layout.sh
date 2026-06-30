#!/bin/sh
# OmniOS — partition_layout.sh
# Create disk image with A/B slot partitions for OTA updates
# Faz 2 - OTA A/B Partitioning
# SPDX-License-Identifier: MIT

set -e

usage() {
    echo "Usage: $0 -o <image> -s <size>"
    echo "  -o <image>  Output image path (e.g., omnios.img)"
    echo "  -s <size>   Total size in GB (default: 8)"
    exit 1
}

IMAGE=""
SIZE_GB=8

while getopts "o:s:h" opt; do
    case "$opt" in
        o) IMAGE="$OPTARG" ;;
        s) SIZE_GB="$OPTARG" ;;
        h) usage ;;
        *) usage ;;
    esac
done

if [ -z "$IMAGE" ]; then
    echo "Error: -o <image> is required"
    usage
fi

if [ "$(id -u)" -ne 0 ]; then
    echo "Warning: not running as root, continuing anyway..."
fi

# Calculate partition sizes (in MB)
BOOT_SIZE_MB=64
ROOT_SIZE_MB=2048
DATA_SIZE_MB=$((SIZE_GB * 1024 - BOOT_SIZE_MB * 2 - ROOT_SIZE_MB * 2 - 8))
METADATA_SIZE_MB=8

TOTAL_MB=$((BOOT_SIZE_MB * 2 + ROOT_SIZE_MB * 2 + DATA_SIZE_MB + METADATA_SIZE_MB))

echo "═══ OmniOS Partition Layout ═══"
echo "  Image:      $IMAGE"
echo "  Total:      ${SIZE_GB}G (${TOTAL_MB}M)"
echo ""
echo "  boot_a:     ${BOOT_SIZE_MB}M"
echo "  root_a:     ${ROOT_SIZE_MB}M  (dm-verity)"
echo "  boot_b:     ${BOOT_SIZE_MB}M"
echo "  root_b:     ${ROOT_SIZE_MB}M  (dm-verity)"
echo "  data:       ${DATA_SIZE_MB}M  (F2FS)"
echo "  metadata:   ${METADATA_SIZE_MB}M"
echo ""

# Create sparse image
echo "[*] Creating ${SIZE_GB}G sparse image..."
dd if=/dev/zero of="$IMAGE" bs=1M count=0 seek="$TOTAL_MB" status=progress

# Partition with sgdisk
echo "[*] Writing GPT partition table..."
sgdisk --clear "$IMAGE"

sgdisk --new=1:0:+${BOOT_SIZE_MB}M  --typecode=1:8300 --change-name=1:"boot_a"   "$IMAGE"
sgdisk --new=2:0:+${ROOT_SIZE_MB}M  --typecode=2:8300 --change-name=2:"root_a"   "$IMAGE"
sgdisk --new=3:0:+${BOOT_SIZE_MB}M  --typecode=3:8300 --change-name=3:"boot_b"   "$IMAGE"
sgdisk --new=4:0:+${ROOT_SIZE_MB}M  --typecode=4:8300 --change-name=4:"root_b"   "$IMAGE"
sgdisk --new=5:0:+${DATA_SIZE_MB}M  --typecode=5:8300 --change-name=5:"data"     "$IMAGE"
sgdisk --new=6:0:+${METADATA_SIZE_MB}M --typecode=6:8300 --change-name=6:"metadata" "$IMAGE"

# Make partitions known to the system
echo "[*] Setting up loop device..."
LOOP_DEV=$(losetup -f --show -P "$IMAGE" 2>/dev/null || echo "")
if [ -z "$LOOP_DEV" ]; then
    echo "Warning: losetup failed. Formatting requires loop device."
    echo "After attaching, run:"
    echo "  mkfs.ext4 ${LOOP_DEV}p1  # boot_a"
    echo "  mkfs.ext4 ${LOOP_DEV}p2  # root_a"
    echo "  mkfs.ext4 ${LOOP_DEV}p3  # boot_b"
    echo "  mkfs.ext4 ${LOOP_DEV}p4  # root_b"
    echo "  mkfs.f2fs ${LOOP_DEV}p5  # data"
    echo "  mkfs.ext4 ${LOOP_DEV}p6  # metadata"
    exit 0
fi

# Format partitions
echo "[*] Formatting partitions..."

echo "  boot_a (ext4)..."
mkfs.ext4 -q "${LOOP_DEV}p1" -L boot_a

echo "  root_a (ext4)..."
mkfs.ext4 -q "${LOOP_DEV}p2" -L root_a

echo "  boot_b (ext4)..."
mkfs.ext4 -q "${LOOP_DEV}p3" -L boot_b

echo "  root_b (ext4)..."
mkfs.ext4 -q "${LOOP_DEV}p4" -L root_b

echo "  data (f2fs)..."
mkfs.f2fs -q "${LOOP_DEV}p5" -l data 2>/dev/null || mkfs.ext4 -q "${LOOP_DEV}p5"

echo "  metadata (ext4)..."
mkfs.ext4 -q "${LOOP_DEV}p6" -L metadata

# Write initial slot metadata
echo "[*] Writing initial A/B metadata..."
mkdir -p /tmp/omnios_metadata
mount "${LOOP_DEV}p6" /tmp/omnios_metadata
cat > /tmp/omnios_metadata/slot_info << 'EOF'
active_slot=a
boot_count=0
max_boot_attempts=3
update_available=0
update_slot=b
EOF
umount /tmp/omnios_metadata
rmdir /tmp/omnios_metadata

# Detach loop
losetup -d "$LOOP_DEV"

echo ""
echo "[+] Partition layout complete: $IMAGE"
echo "    Active: slot_a  Standby: slot_b"
echo ""
echo "Next: flash kernel+rootfs to slot_a:"
echo "  dd if=kernel.bin of=$(basename $IMAGE):boot_a"
echo "  dd if=rootfs.ext4 of=$(basename $IMAGE):root_a"

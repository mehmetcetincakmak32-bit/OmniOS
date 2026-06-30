#!/bin/sh
# OmniOS — build_rootfs.sh
# Build Alpine-based root filesystem for OmniOS
# Faz 1 Foundation
# SPDX-License-Identifier: MIT

set -e

usage() {
    echo "Usage: $0 -a <arch> [-o <outdir>] [-m <mirror>] [-r <release>]"
    echo "  -a <arch>    Architecture: aarch64 | x86_64"
    echo "  -o <outdir>  Output directory (default: ./rootfs)"
    echo "  -m <mirror>  Alpine mirror (default: https://dl-cdn.alpinelinux.org/alpine)"
    echo "  -r <release> Alpine release (default: v3.19)"
    exit 1
}

ARCH=""
OUTDIR="$(pwd)/rootfs"
MIRROR="https://dl-cdn.alpinelinux.org/alpine"
RELEASE="v3.19"

while getopts "a:o:m:r:h" opt; do
    case "$opt" in
        a) ARCH="$OPTARG" ;;
        o) OUTDIR="$OPTARG" ;;
        m) MIRROR="$OPTARG" ;;
        r) RELEASE="$OPTARG" ;;
        h) usage ;;
        *) usage ;;
    esac
done

if [ -z "$ARCH" ]; then
    echo "Error: -a <arch> is required"
    usage
fi

case "$ARCH" in
    aarch64) ALPINE_ARCH="aarch64";;
    x86_64)  ALPINE_ARCH="x86_64";;
    *)       echo "Error: unsupported arch '$ARCH'. Use aarch64 or x86_64."; exit 1;;
esac

ROOTFS_URL="${MIRROR}/${RELEASE}/releases/${ALPINE_ARCH}/alpine-minirootfs-${RELEASE}-${ALPINE_ARCH}.tar.gz"

echo "[*] Building OmniOS rootfs"
echo "    Arch:    $ALPINE_ARCH"
echo "    Release: $RELEASE"
echo "    Mirror:  $MIRROR"
echo "    Outdir:  $OUTDIR"

# Fetch minirootfs if not cached
CACHE_DIR="$(pwd)/.cache"
mkdir -p "$CACHE_DIR" "$OUTDIR"

ROOTFS_TGZ="${CACHE_DIR}/alpine-minirootfs-${RELEASE}-${ALPINE_ARCH}.tar.gz"
if [ ! -f "$ROOTFS_TGZ" ]; then
    echo "[*] Downloading Alpine minirootfs..."
    wget -q "$ROOTFS_URL" -O "$ROOTFS_TGZ" || curl -fsSL "$ROOTFS_URL" -o "$ROOTFS_TGZ"
fi

echo "[*] Extracting rootfs..."
tar -xzf "$ROOTFS_TGZ" -C "$OUTDIR"

# Setup DNS for chroot
mkdir -p "$OUTDIR/etc"
echo "nameserver 1.1.1.1" > "$OUTDIR/etc/resolv.conf"
echo "nameserver 8.8.8.8" >> "$OUTDIR/etc/resolv.conf"

# Bind mount for apk
mount --bind /proc "$OUTDIR/proc" 2>/dev/null || true
mount --bind /sys  "$OUTDIR/sys"  2>/dev/null || true
mount --bind /dev  "$OUTDIR/dev"  2>/dev/null || true

# Install packages inside chroot
PACKAGES="
    musl
    musl-dev
    s6
    s6-rc
    s6-linux-init
    wayland
    wayland-protocols
    wlroots
    mesa
    mesa-dri-gallium
    mesa-egl
    mesa-gles
    libxkbcommon
    pixman
    libdrm
    libinput
    libevdev
    seatd
    ncurses
    busybox
    alpine-base
    alpine-conf
    apk-tools
    zlib
    openssl
    ca-certificates
    eudev
    elogind
    polkit
    dbus
    networkmanager
    openssh
    htop
    nano
"

echo "[*] Installing packages (chroot)..."
chroot "$OUTDIR" /bin/sh -c "
    apk update && \
    apk add --no-cache $PACKAGES
"

# Create OmniOS s6-rc service tree
echo "[*] Creating s6-rc service definitions..."

SERVICES_DIR="$OUTDIR/etc/s6-rc"
mkdir -p "$SERVICES_DIR/omnios-compositor"
mkdir -p "$SERVICES_DIR/omnios-flutter"
mkdir -p "$SERVICES_DIR/seatd"
mkdir -p "$SERVICES_DIR/dbus"
mkdir -p "$SERVICES_DIR/networkmanager"

# Compositor service
cat > "$SERVICES_DIR/omnios-compositor/type" << 'EOF'
longrun
EOF

cat > "$SERVICES_DIR/omnios-compositor/run" << 'SEOF'
#!/bin/execlineb -P
s6-envuidgid omni
s6-setuidgid omni
s6-softlimit -o 512
/usr/bin/omnios-compositor
SEOF
chmod +x "$SERVICES_DIR/omnios-compositor/run"

cat > "$SERVICES_DIR/omnios-compositor/dependencies" << 'EOF'
seatd
dbus
EOF

# Flutter System UI service
cat > "$SERVICES_DIR/omnios-flutter/type" << 'EOF'
longrun
EOF

cat > "$SERVICES_DIR/omnios-flutter/run" << 'SEOF'
#!/bin/execlineb -P
s6-envuidgid omni
s6-setuidgid omni
/usr/bin/omnios-flutter-embedder
SEOF
chmod +x "$SERVICES_DIR/omnios-flutter/run"

cat > "$SERVICES_DIR/omnios-flutter/dependencies" << 'EOF'
omnios-compositor
EOF

# seatd
cat > "$SERVICES_DIR/seatd/type" << 'EOF'
longrun
EOF

cat > "$SERVICES_DIR/seatd/run" << 'SEOF'
#!/bin/execlineb -P
s6-setuidgid root
seatd -g video -u seatd
SEOF
chmod +x "$SERVICES_DIR/seatd/run"

# dbus
cat > "$SERVICES_DIR/dbus/type" << 'EOF'
longrun
EOF

cat > "$SERVICES_DIR/dbus/run" << 'SEOF'
#!/bin/execlineb -P
s6-setuidgid root
dbus-daemon --system --nofork
SEOF
chmod +x "$SERVICES_DIR/dbus/run"

# NetworkManager
cat > "$SERVICES_DIR/networkmanager/type" << 'EOF'
longrun
EOF

cat > "$SERVICES_DIR/networkmanager/run" << 'SEOF'
#!/bin/execlineb -P
s6-setuidgid root
NetworkManager --no-daemon
SEOF
chmod +x "$SERVICES_DIR/networkmanager/run"

# Create omni user
chroot "$OUTDIR" /bin/sh -c "
    adduser -D -s /bin/sh omni || true
    addgroup omni video || true
    addgroup omni input || true
    addgroup omni seat || true
"

# Create boot script (s6-linux-init)
mkdir -p "$OUTDIR/etc/s6-linux-init"
cat > "$OUTDIR/etc/s6-linux-init/init" << 'SEOF'
#!/bin/sh
exec /sbin/s6-svscan /etc/s6-rc
SEOF
chmod +x "$OUTDIR/etc/s6-linux-init/init"

# Cleanup
umount "$OUTDIR/proc" 2>/dev/null || true
umount "$OUTDIR/sys"  2>/dev/null || true
umount "$OUTDIR/dev"  2>/dev/null || true

echo "[+] Rootfs built at: $OUTDIR"
echo "    Size: $(du -sh "$OUTDIR" | cut -f1)"
echo ""
echo "Next steps:"
echo "  1. Build kernel:  make -C kernel mobile_defconfig && make -C kernel"
echo "  2. Run QEMU:      bash scripts/run_qemu.sh -k kernel/omnios_kernel.bin -r $OUTDIR -a $ARCH"

#!/bin/sh
# OmniOS — ota_update.sh
# Over-the-air A/B slot update system
# Faz 2 - OTA A/B Partitioning
# SPDX-License-Identifier: MIT

set -e

SLOT_INFO_PATH="/metadata/slot_info"
UPDATE_DIR="/tmp/omnios_update"

usage() {
    echo "Usage: $0 <command> [args]"
    echo ""
    echo "Commands:"
    echo "  status              Show current slot status"
    echo "  prepare <url>       Download and prepare update"
    echo "  apply               Apply prepared update to standby slot"
    echo "  switch              Switch to standby slot (reboot required)"
    echo "  commit              Mark current slot as successful"
    echo "  rollback            Revert to previous slot"
    echo "  verify              Verify dm-verity hash of current root"
    exit 1
}

if [ "$(id -u)" -ne 0 ]; then
    echo "Error: must be run as root"
    exit 1
}

load_slot_info() {
    if [ ! -f "$SLOT_INFO_PATH" ]; then
        echo "Error: slot info not found at $SLOT_INFO_PATH"
        exit 1
    fi
    . "$SLOT_INFO_PATH"
}

save_slot_info() {
    cat > "$SLOT_INFO_PATH" << EOF
active_slot=$active_slot
boot_count=$boot_count
max_boot_attempts=$max_boot_attempts
update_available=$update_available
update_slot=$update_slot
EOF
    sync
}

get_slot_devices() {
    case "$active_slot" in
        a) BOOT_DEV="/dev/disk/by-partlabel/boot_a"
           ROOT_DEV="/dev/disk/by-partlabel/root_a"
           STANDBY_BOOT="/dev/disk/by-partlabel/boot_b"
           STANDBY_ROOT="/dev/disk/by-partlabel/root_b" ;;
        b) BOOT_DEV="/dev/disk/by-partlabel/boot_b"
           ROOT_DEV="/dev/disk/by-partlabel/root_b"
           STANDBY_BOOT="/dev/disk/by-partlabel/boot_a"
           STANDBY_ROOT="/dev/disk/by-partlabel/root_a" ;;
        *) echo "Error: unknown slot '$active_slot'"; exit 1 ;;
    esac
}

cmd_status() {
    load_slot_info
    get_slot_devices

    echo "═══ OmniOS A/B Slot Status ═══"
    echo "  Active slot:    $active_slot"
    echo "  Boot count:     $boot_count / $max_boot_attempts"
    echo "  Update avail:   $update_available"
    echo "  Update slot:    $update_slot"
    echo ""
    echo "  Boot device:    $BOOT_DEV"
    echo "  Root device:    $ROOT_DEV"
    echo "  Standby boot:   $STANDBY_BOOT"
    echo "  Standby root:   $STANDBY_ROOT"
    echo ""

    if [ -f "$BOOT_DEV" ]; then
        echo "  Active partition: present"
    else
        echo "  Active partition: NOT FOUND"
    fi

    # Check dm-verity
    VERITY_DEV=$(dmsetup info -c omnios-root 2>/dev/null | tail -1 | awk '{print $1}')
    if [ -n "$VERITY_DEV" ]; then
        echo "  dm-verity: active ($VERITY_DEV)"
    else
        echo "  dm-verity: not active (running without verification)"
    fi
}

cmd_prepare() {
    local URL="$1"
    if [ -z "$URL" ]; then
        echo "Error: URL required"
        usage
    fi

    load_slot_info

    echo "[*] Preparing OTA update from $URL"
    mkdir -p "$UPDATE_DIR"

    echo "  Downloading update package..."
    wget -q "$URL" -O "$UPDATE_DIR/update.zip" || curl -fsSL "$URL" -o "$UPDATE_DIR/update.zip"

    echo "  Verifying signature..."
    # TODO: verify with public key
    echo "  (signature verification placeholder)"

    echo "  Extracting..."
    unzip -q -o "$UPDATE_DIR/update.zip" -d "$UPDATE_DIR"

    echo "  Preparing dm-verity hash tree..."
    if [ -f "$UPDATE_DIR/root.img" ]; then
        veritysetup format "$UPDATE_DIR/root.img" "$UPDATE_DIR/root.verity" \
            --hash=sha256 --data-block-size=4096 --hash-block-size=4096 \
            > "$UPDATE_DIR/verity_params"
        cat "$UPDATE_DIR/verity_params"
    fi

    update_available=1
    save_slot_info

    echo "[+] Update prepared for slot $update_slot"
    echo "    Run '$(basename $0) apply' to flash"
}

cmd_apply() {
    load_slot_info

    if [ "$update_available" != "1" ]; then
        echo "Error: no update prepared. Run '$(basename $0) prepare' first."
        exit 1
    fi

    get_slot_devices

    echo "[*] Flashing update to standby slot ($update_slot)..."

    # Flash boot
    if [ -f "$UPDATE_DIR/boot.img" ]; then
        echo "  Flashing boot..."
        dd if="$UPDATE_DIR/boot.img" of="$STANDBY_BOOT" bs=1M status=progress
    fi

    # Flash root
    if [ -f "$UPDATE_DIR/root.img" ]; then
        echo "  Flashing root..."
        dd if="$UPDATE_DIR/root.img" of="$STANDBY_ROOT" bs=1M status=progress

        # Setup dm-verity on standby
        if [ -f "$UPDATE_DIR/root.verity" ] && [ -f "$UPDATE_DIR/verity_params" ]; then
            echo "  Setting up dm-verity for standby..."
            HASH=$(grep "Root hash:" "$UPDATE_DIR/verity_params" | awk '{print $3}')
            if [ -n "$HASH" ]; then
                # Store verity params for next boot
                mkdir -p /metadata/verity
                cp "$UPDATE_DIR/root.verity" "/metadata/verity/root_${update_slot}.verity"
                echo "$HASH" > "/metadata/verity/root_${update_slot}.hash"
                echo "  Root hash: $HASH"
            fi
        fi
    fi

    # Flash dtb/firmware if present
    if [ -f "$UPDATE_DIR/dtb.img" ]; then
        echo "  Flashing device tree..."
        dd if="$UPDATE_DIR/dtb.img" of="${STANDBY_BOOT%boot*}dtb" bs=1M status=progress 2>/dev/null || true
    fi

    update_slot=$([ "$active_slot" = "a" ] && echo "b" || echo "a")
    echo "[+] Update flashed to slot $update_slot"
    echo "    Run '$(basename $0) switch' and reboot to activate"
}

cmd_switch() {
    load_slot_info
    get_slot_devices

    local new_slot=$([ "$active_slot" = "a" ] && echo "b" || echo "a")
    echo "[*] Switching from slot $active_slot to $new_slot"

    # Update bootloader variable or metadata
    active_slot=$new_slot
    boot_count=0
    update_available=0
    save_slot_info

    echo "[+] Switched to slot $new_slot"
    echo "    Reboot to apply: shutdown -r now"
}

cmd_commit() {
    load_slot_info

    echo "[*] Committing slot $active_slot as successful"
    boot_count=0
    save_slot_info
    echo "[+] Slot $active_slot committed"
}

cmd_rollback() {
    load_slot_info
    get_slot_devices

    local prev_slot=$([ "$active_slot" = "a" ] && echo "b" || echo "a")

    if [ "$boot_count" -ge "$max_boot_attempts" ]; then
        echo "Warning: slot $active_slot failed $boot_count times"
        echo "Rolling back to slot $prev_slot..."

        active_slot=$prev_slot
        boot_count=0
        update_available=0
        save_slot_info

        echo "[+] Rolled back to slot $prev_slot"
        echo "    Reboot to apply"
    else
        echo "Slot $active_slot still has attempts remaining ($boot_count/$max_boot_attempts)"
    fi
}

cmd_verify() {
    load_slot_info
    get_slot_devices

    echo "[*] Verifying dm-verity for slot $active_slot..."
    local hash_file="/metadata/verity/root_${active_slot}.hash"
    local verity_file="/metadata/verity/root_${active_slot}.verity"

    if [ -f "$hash_file" ] && [ -f "$verity_file" ]; then
        local HASH=$(cat "$hash_file")
        echo "  Expected root hash: $HASH"

        # Try to verify
        veritysetup verify "$ROOT_DEV" "$verity_file" --root-hash="$HASH" \
            && echo "[+] dm-verity: VERIFIED" \
            || echo "[!] dm-verity: MISMATCH - possible corruption"
    else
        echo "  No dm-verity data for slot $active_slot"
        echo "  Running filesystem check instead..."
        fsck.ext4 -n "$ROOT_DEV" 2>/dev/null || echo "  Warning: fsck failed"
    fi
}

# ── Main dispatch ──────────────────────────────────────────────────

case "${1:-help}" in
    status)   cmd_status ;;
    prepare)  cmd_prepare "$2" ;;
    apply)    cmd_apply ;;
    switch)   cmd_switch ;;
    commit)   cmd_commit ;;
    rollback) cmd_rollback ;;
    verify)   cmd_verify ;;
    help|*)   usage ;;
esac

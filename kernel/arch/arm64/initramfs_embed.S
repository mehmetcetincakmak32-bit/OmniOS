/* OmniOS — kernel/arch/arm64/initramfs_embed.S */
/* Embed initramfs cpio archive into kernel binary via .incbin */
/* SPDX-License-Identifier: MIT */

.section .initramfs, "a", @progbits
.global _binary_initramfs_cpio_start
.global _binary_initramfs_cpio_end

_binary_initramfs_cpio_start:
    .incbin "initramfs.cpio"
_binary_initramfs_cpio_end:

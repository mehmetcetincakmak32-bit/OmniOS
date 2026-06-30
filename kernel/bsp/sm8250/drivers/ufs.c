/* OmniOS — kernel/bsp/sm8250/drivers/ufs.c */
/* UFS 3.0 flash storage for SM8250 */
/* SPDX-License-Identifier: MIT */

#include "../sm8250.h"
#include <stdio.h>
#include <string.h>

#define UFS_HCI_BASE       SM8250_UFS_BASE
#define UFS_HC_CTRL        0x00
#define UFS_HC_STATUS      0x04
#define UFS_DEV_ID         0x08
#define UFS_INT_STS        0x10
#define UFS_INT_EN         0x14
#define UFS_UTP_TASK       0x30
#define UFS_UTP_TRANSFER   0x40
#define UFS_UTP_RESP       0x50
#define UFS_DATA_BASE      0x10000
#define UFS_DATA_SIZE      0x100000

#define UFS_BLOCK_SIZE     4096

static int ufs_ready = 0;
static uint64_t ufs_total_blocks = 0;

static int ufs_wait_status(uint32_t mask, uint32_t val, int timeout_us) {
    for (int i = 0; i < timeout_us; i++) {
        if ((read_reg(UFS_HCI_BASE, UFS_HC_STATUS) & mask) == val) return 0;
    }
    return -1;
}

int ufs_init(void) {
    printf("[UFS] UFS 3.0 flash init\n");

    /* Reset HCI */
    write_reg(UFS_HCI_BASE, UFS_HC_CTRL, 0x02);
    if (ufs_wait_status(0x02, 0, 100000) < 0) {
        printf("[UFS] Reset timeout\n");
        return -1;
    }

    /* Enable interrupts */
    write_reg(UFS_HCI_BASE, UFS_INT_EN, 0x1FF);

    /* Start controller */
    write_reg(UFS_HCI_BASE, UFS_HC_CTRL, 0x01);
    if (ufs_wait_status(0x01, 0x01, 100000) < 0) {
        printf("[UFS] Start timeout\n");
        return -1;
    }

    uint32_t dev_id = read_reg(UFS_HCI_BASE, UFS_DEV_ID);
    ufs_total_blocks = (dev_id & 0xFFFF) * 1024ULL;  /* Simulated: ID=capacity */
    if (ufs_total_blocks == 0) ufs_total_blocks = 122880;  /* ~480 MB default */

    ufs_ready = 1;
    printf("[UFS] Ready: %llu blocks (%llu MB)\n",
        (unsigned long long)ufs_total_blocks,
        (unsigned long long)(ufs_total_blocks * UFS_BLOCK_SIZE / 1024 / 1024));
    return 0;
}

int ufs_read(uint64_t lba, uint8_t *buf, uint32_t count) {
    if (!ufs_ready) return -1;
    if (lba + count > ufs_total_blocks) return -1;

    uint32_t size = count * UFS_BLOCK_SIZE;
    memcpy(buf, (const void *)(UFS_HCI_BASE + UFS_DATA_BASE + lba * UFS_BLOCK_SIZE), size);
    return (int)size;
}

int ufs_write(uint64_t lba, const uint8_t *buf, uint32_t count) {
    if (!ufs_ready) return -1;
    if (lba + count > ufs_total_blocks) return -1;

    uint32_t size = count * UFS_BLOCK_SIZE;
    memcpy((void *)(UFS_HCI_BASE + UFS_DATA_BASE + lba * UFS_BLOCK_SIZE), buf, size);
    return (int)size;
}

int ufs_trim(uint64_t lba, uint32_t count) {
    if (!ufs_ready) return -1;
    printf("[UFS] Trim: lba=%llu count=%u\n", (unsigned long long)lba, count);
    return 0;
}

uint64_t ufs_get_capacity(void) {
    return ufs_total_blocks * UFS_BLOCK_SIZE;
}

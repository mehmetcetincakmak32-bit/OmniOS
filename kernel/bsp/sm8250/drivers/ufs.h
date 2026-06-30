/* OmniOS — kernel/bsp/sm8250/drivers/ufs.h */
/* UFS 3.0 flash storage interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_UFS_H
#define OMNIOS_UFS_H

#include <stdint.h>

int      ufs_init(void);
int      ufs_read(uint64_t lba, uint8_t *buf, uint32_t count);
int      ufs_write(uint64_t lba, const uint8_t *buf, uint32_t count);
int      ufs_trim(uint64_t lba, uint32_t count);
uint64_t ufs_get_capacity(void);

#endif

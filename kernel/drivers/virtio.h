/* OmniOS — kernel/drivers/virtio.h */
/* VirtIO transport + device framework */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_VIRTIO_H
#define OMNIOS_VIRTIO_H

#include <stdint.h>
#include <stdbool.h>

/* VirtIO PCI vendor/device IDs */
#define VIRTIO_VENDOR  0x1AF4
#define VIRTIO_NET     0x1000
#define VIRTIO_BLOCK   0x1001
#define VIRTIO_CONSOLE 0x1003
#define VIRTIO_GPU     0x1050
#define VIRTIO_INPUT   0x1052

/* MMIO magic */
#define VIRTIO_MMIO_MAGIC  0x74726976

/* PCI capability offsets (modern virtio) */
#define VIRTIO_PCI_CAP_COMMON_CFG  1
#define VIRTIO_PCI_CAP_NOTIFY_CFG  2
#define VIRTIO_PCI_CAP_ISR_CFG     3
#define VIRTIO_PCI_CAP_DEVICE_CFG  4
#define VIRTIO_PCI_CAP_PCI_CFG     5

/* Queue descriptor flags */
#define VIRTQ_DESC_F_NEXT     1
#define VIRTQ_DESC_F_WRITE    2
#define VIRTQ_DESC_F_INDIRECT 4

/* Queue avail flags */
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1

/* Queue used flags */
#define VIRTQ_USED_F_NO_NOTIFY 1

/* Device status */
#define VIRTIO_STATUS_ACK      1
#define VIRTIO_STATUS_DRIVER   2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_STATUS_FEATURES_OK 8
#define VIRTIO_STATUS_DEVICE_NEEDS_RESET 64
#define VIRTIO_STATUS_FAILED   128

typedef struct {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed)) virtq_desc_t;

typedef struct {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[];
} __attribute__((packed)) virtq_avail_t;

typedef struct {
    uint16_t id;
    uint32_t len;
} __attribute__((packed)) virtq_used_elem_t;

typedef struct {
    uint16_t flags;
    uint16_t idx;
    virtq_used_elem_t ring[];
} __attribute__((packed)) virtq_used_t;

typedef struct {
    virtq_desc_t  *desc;
    virtq_avail_t *avail;
    virtq_used_t  *used;
    uint16_t       size;
    uint16_t       free_head;
    uint16_t       free_count;
} virtio_queue_t;

typedef struct {
    void    *common_cfg;
    void    *notify_base;
    uint32_t notify_off_multiplier;
    void    *isr;
    void    *device_cfg;
    uint32_t device_id;
    uint32_t vendor_id;
    uint64_t guest_features;
    uint64_t host_features;
    int      irq;
    uint8_t  status;

    virtio_queue_t queues[8];
    int queue_count;

    void *driver_data;
} virtio_device_t;

/* Common config structure (modern) */
typedef struct {
    uint32_t device_feature_select;
    uint32_t device_feature;
    uint32_t driver_feature_select;
    uint32_t driver_feature;
    uint16_t msix_config;
    uint16_t num_queues;
    uint8_t  device_status;
    uint8_t  config_gen;
    uint16_t queue_select;
    uint16_t queue_size;
    uint16_t queue_msix_vector;
    uint16_t queue_enable;
    uint16_t queue_notify_off;
    uint64_t queue_desc;
    uint64_t queue_driver;
    uint64_t queue_device;
} __attribute__((packed)) virtio_pci_common_cfg_t;

int virtio_init(void);
virtio_device_t *virtio_probe_pci(void);
int virtio_setup_queue(virtio_device_t *dev, int qid, int size);
void virtio_notify(virtio_device_t *dev, int qid);
int virtio_send_desc(virtio_device_t *dev, int qid,
    uint64_t addr, uint32_t len, uint16_t flags, uint16_t next);
int virtio_recv_used(virtio_device_t *dev, int qid, uint32_t *len);

#endif /* OMNIOS_VIRTIO_H */

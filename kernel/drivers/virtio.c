/* OmniOS — kernel/drivers/virtio.c */
/* VirtIO transport (modern PCI) + virtqueue operations */
/* SPDX-License-Identifier: MIT */

#include "virtio.h"
#include "pci.h"
#include "../arch/x86_64/io.h"
#include "../mm/pmm.h"
#include <stdio.h>
#include <string.h>

static virtio_device_t devices[8];
static int device_count = 0;

static uint8_t virtio_find_cap(pci_device_t *pdev, uint8_t cap_type) {
    uint8_t cap = 0x41; /* PCI_CAP_LIST (first capability pointer) */
    for (int i = 0; i < 48; i++) {
        uint32_t cap_hdr = pci_read_config(pdev->bus, pdev->slot, pdev->func, cap);
        if ((cap_hdr & 0xFF) == 0) break;
        if (((cap_hdr >> 8) & 0xFF) == cap_type) return cap;
        cap = (cap_hdr >> 20) & 0xFF;
        if (cap < 0x40) break;
    }
    return 0;
}

virtio_device_t *virtio_probe_pci(void) {
    for (int i = 0; i < pci_device_count(); i++) {
        pci_device_t *pdev = pci_get_device(i);
        if (pdev->vendor_id != VIRTIO_VENDOR) continue;
        if (device_count >= 8) break;

        virtio_device_t *dev = &devices[device_count++];
        memset(dev, 0, sizeof(*dev));
        dev->device_id = pdev->device_id;
        dev->vendor_id = pdev->vendor_id;
        dev->irq = pdev->irq;

        printf("  VirtIO device: %04x:%04x irq=%d bars=[0x%llx 0x%llx]\n",
            pdev->vendor_id, pdev->device_id, pdev->irq,
            (unsigned long long)pdev->bars[0],
            (unsigned long long)pdev->bars[1]);

        /* Map common config via capability */
        uint8_t cap_off = virtio_find_cap(pdev, VIRTIO_PCI_CAP_COMMON_CFG);
        if (cap_off && pdev->bars[0]) {
            dev->common_cfg = (void *)(uintptr_t)(pdev->bars[0] + pdev->bars[0]);
        }

        dev->status = VIRTIO_STATUS_ACK | VIRTIO_STATUS_DRIVER;
        return dev;
    }
    return NULL;
}

int virtio_setup_queue(virtio_device_t *dev, int qid, int size) {
    if (qid >= 8) return -1;
    virtio_queue_t *q = &dev->queues[qid];
    q->size = size;
    q->free_count = size;
    q->free_head = 0;

    /* Allocate descriptor, avail, used rings (physically contiguous) */
    size_t desc_size = size * sizeof(virtq_desc_t);
    size_t avail_size = sizeof(virtq_avail_t) + size * sizeof(uint16_t);
    size_t used_size = sizeof(virtq_used_t) + size * sizeof(virtq_used_elem_t);
    size_t total = desc_size + avail_size + used_size;
    total = (total + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    uintptr_t phys = pmm_alloc_pages(total / PAGE_SIZE);
    if (!phys) return -1;

    uintptr_t virt = phys + 0xFFFFFFFF80000000ULL;
    q->desc = (virtq_desc_t *)virt;
    q->avail = (virtq_avail_t *)(virt + desc_size);
    q->used = (virtq_used_t *)(virt + desc_size + avail_size);

    /* Link descriptors into free list */
    for (int i = 0; i < size - 1; i++) q->desc[i].next = i + 1;
    q->desc[size - 1].next = 0xFFFF;

    /* VirtIO PCI common cfg: queue_select = qid, queue_size, queue_desc, etc. */
    virtio_pci_common_cfg_t *cfg = (virtio_pci_common_cfg_t *)dev->common_cfg;
    if (cfg) {
        cfg->queue_select = qid;
        cfg->queue_size = size;
        cfg->queue_desc = phys;
        cfg->queue_driver = phys + desc_size;
        cfg->queue_device = phys + desc_size + avail_size;
        cfg->queue_enable = 1;
    }

    dev->queue_count++;
    return 0;
}

void virtio_notify(virtio_device_t *dev, int qid) {
    virtio_pci_common_cfg_t *cfg = (virtio_pci_common_cfg_t *)dev->common_cfg;
    if (cfg) {
        uint32_t off = cfg->queue_notify_off;
        mmio_write32(dev->notify_base + off * dev->notify_off_multiplier, qid);
    }
}

int virtio_send_desc(virtio_device_t *dev, int qid,
        uint64_t addr, uint32_t len, uint16_t flags, uint16_t next) {
    (void)next;
    virtio_queue_t *q = &dev->queues[qid];
    if (q->free_count == 0) return -1;

    uint16_t idx = q->free_head;
    q->desc[idx].addr = addr;
    q->desc[idx].len = len;
    q->desc[idx].flags = flags;
    q->free_head = q->desc[idx].next;
    q->free_count--;

    uint16_t avail_idx = q->avail->idx;
    q->avail->ring[avail_idx % q->size] = idx;
    __sync_synchronize();
    q->avail->idx = avail_idx + 1;
    return idx;
}

int virtio_recv_used(virtio_device_t *dev, int qid, uint32_t *len) {
    virtio_queue_t *q = &dev->queues[qid];
    __sync_synchronize();
    uint16_t used_idx = q->used->idx;
    uint16_t last = 0;

    if (last == used_idx) return -1;
    uint16_t id = q->used->ring[last % q->size].id;
    if (len) *len = q->used->ring[last % q->size].len;
    q->free_head = id;
    q->free_count++;
    last++;
    return id;
}

int virtio_init(void) {
    printf("[VIRTIO] Transport initialized\n");
    return 0;
}

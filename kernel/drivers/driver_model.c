/*
 * OmniOS Device Driver Framework
 * Register/unregister devices, find by name/ID
 */

#include "../include/omnios_kernel.h"
#include <string.h>
#include <stdio.h>

static device_t* _device_list = NULL;
static uint32_t _device_count = 0;
static uint32_t _next_dev_id = 1;

status_t dev_init(void) {
    _device_list = NULL;
    _device_count = 0;
    _next_dev_id = 1;
    printf("[DEV] Aygit surucusu modeli hazir\n");
    return STATUS_SUCCESS;
}

status_t dev_register(device_t* device) {
    if (!device) return STATUS_INVALID;

    device_t* new_dev = (device_t*)kmalloc(sizeof(device_t));
    if (!new_dev) return STATUS_NO_MEMORY;

    memcpy(new_dev, device, sizeof(device_t));
    new_dev->dev_id = _next_dev_id++;
    new_dev->registered = true;
    new_dev->next = _device_list;
    _device_list = new_dev;
    _device_count++;

    printf("[DEV] %s kaydedildi (id=%d, tip=%d)\n",
           device->name, new_dev->dev_id, device->type);

    /* Call init if available */
    if (new_dev->ops && new_dev->ops->init) {
        new_dev->ops->init();
    }

    return STATUS_SUCCESS;
}

status_t dev_unregister(uint32_t dev_id) {
    device_t* prev = NULL;
    device_t* curr = _device_list;

    while (curr) {
        if (curr->dev_id == dev_id) {
            if (curr->ops && curr->ops->deinit)
                curr->ops->deinit();

            if (prev)
                prev->next = curr->next;
            else
                _device_list = curr->next;

            printf("[DEV] %s kaldirildi (id=%d)\n", curr->name, dev_id);
            kfree(curr);
            _device_count--;
            return STATUS_SUCCESS;
        }
        prev = curr;
        curr = curr->next;
    }
    return STATUS_NOT_FOUND;
}

device_t* dev_find(const char* name) {
    device_t* curr = _device_list;
    while (curr) {
        if (strcmp(curr->name, name) == 0)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

device_t* dev_find_by_id(uint32_t dev_id) {
    device_t* curr = _device_list;
    while (curr) {
        if (curr->dev_id == dev_id)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

uint32_t dev_get_count(void) {
    return _device_count;
}

/* OmniOS — kernel/bsp/sm8250/drivers/fingerprint.c */
/* Fingerprint sensor (e.g. Synaptics FS9500 / Goodix) */
/* SPDX-License-Identifier: MIT */

#include "../sm8250.h"
#include <stdio.h>
#include <string.h>

#define FP_I2C_BASE       (SM8250_QUPV3_BASE + 0x30000)
#define FP_IRQ            122
#define FP_RST_GPIO       18
#define FP_INT_GPIO       19

#define FP_MAX_TEMPLATES  5
#define FP_TEMPLATE_SIZE  2048

typedef struct {
    int   id;
    char  name[64];
    uint8_t template[FP_TEMPLATE_SIZE];
    int   enrolled;
} fp_template_t;

static fp_template_t _templates[FP_MAX_TEMPLATES];
static int _fp_ready = 0;
static int _fp_count = 0;

int fingerprint_init(void) {
    printf("[FP] Fingerprint sensor init\n");

    /* Configure GPIOs */
    write_reg(SM8250_TLMM_BASE, FP_RST_GPIO * 4, 0x03);
    write_reg(SM8250_TLMM_BASE, FP_INT_GPIO * 4, 0x00);

    /* Reset sensor */
    write_reg(SM8250_TLMM_BASE, FP_RST_GPIO * 4, 0);
    for (int i = 0; i < 50000; i++) __asm__("nop");
    write_reg(SM8250_TLMM_BASE, FP_RST_GPIO * 4, 0x03);

    memset(_templates, 0, sizeof(_templates));
    _fp_ready = 1;
    printf("[FP] Ready\n");
    return 0;
}

int fingerprint_enroll(const char *name) {
    if (!_fp_ready) return -1;
    if (_fp_count >= FP_MAX_TEMPLATES) return -1;

    _templates[_fp_count].id = _fp_count + 1;
    strncpy(_templates[_fp_count].name, name, sizeof(_templates[_fp_count].name) - 1);
    _templates[_fp_count].enrolled = 1;
    _fp_count++;

    printf("[FP] Enrolled: %s (id=%d)\n", name, _templates[_fp_count - 1].id);
    return _templates[_fp_count - 1].id;
}

int fingerprint_authenticate(void) {
    if (!_fp_ready) return -1;
    if (_fp_count == 0) return -1;

    /* Simulate scan + match */
    int match_id = _templates[0].id;
    printf("[FP] Match: id=%d (%s)\n", match_id, _templates[0].name);
    return match_id;
}

int fingerprint_delete(int id) {
    if (!_fp_ready) return -1;
    for (int i = 0; i < _fp_count; i++) {
        if (_templates[i].id == id) {
            _templates[i].enrolled = 0;
            printf("[FP] Deleted id=%d (%s)\n", id, _templates[i].name);
            return 0;
        }
    }
    return -1;
}

int fingerprint_count(void) {
    return _fp_count;
}

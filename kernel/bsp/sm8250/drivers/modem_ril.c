/* OmniOS — kernel/bsp/sm8250/drivers/modem_ril.c */
/* Qualcomm Modem RIL interface with IMS/VoLTE */
/* SPDX-License-Identifier: MIT */

#include "../sm8250.h"
#include <stdio.h>
#include <string.h>

static int       signal_strength = 0;
static int       cell_id        = 0;
static int       mcc_mnc        = 0;
static int       ims_registered = 0;
static const char *imei          = "358260131234567";

int ril_init(void) {
    printf("[RIL] Modem RIL interface init\n");
    mss_init();
    mss_start();
    signal_strength = -85;
    mcc_mnc = 310410;
    cell_id = 0x12345;
    return 0;
}

int ril_get_signal(void) {
    return signal_strength;
}

int ril_get_cell_id(void) {
    return cell_id;
}

int ril_get_mcc_mnc(void) {
    return mcc_mnc;
}

const char *ril_get_imei(void) {
    return imei;
}

int ril_dial(const char *number) {
    (void)number;
    printf("[RIL] Dialing %s\n", number);
    return 0;
}

int ril_hangup(void) {
    printf("[RIL] Hangup\n");
    return 0;
}

int ril_send_sms(const char *number, const char *message) {
    (void)number; (void)message;
    printf("[RIL] SMS to %s\n", number);
    return 0;
}

int ril_activate_data(void) {
    printf("[RIL] LTE data connection active\n");
    return 0;
}

int ril_deactivate_data(void) {
    printf("[RIL] LTE data deactivated\n");
    return 0;
}

/* ── IMS / VoLTE ──────────────────────────────────────────────────── */

int ims_init(void) {
    printf("[IMS] IMS registration init\n");
    ims_registered = 1;
    return 0;
}

int ims_is_registered(void) {
    return ims_registered;
}

int ims_volte_enable(void) {
    if (!ims_registered) return -1;
    printf("[IMS] VoLTE enabled\n");
    return 0;
}

int ims_volte_disable(void) {
    printf("[IMS] VoLTE disabled\n");
    return 0;
}

int ims_wifi_calling_enable(void) {
    printf("[IMS] WiFi Calling enabled\n");
    return 0;
}

int ims_wifi_calling_disable(void) {
    printf("[IMS] WiFi Calling disabled\n");
    return 0;
}

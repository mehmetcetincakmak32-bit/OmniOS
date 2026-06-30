/* OmniOS — kernel/bsp/sm8250/drivers/modem_ril.h */
/* Mobile RIL + IMS interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_MODEM_RIL_H
#define OMNIOS_MODEM_RIL_H

int ril_init(void);
int ril_get_signal(void);
int ril_get_cell_id(void);
int ril_get_mcc_mnc(void);
const char *ril_get_imei(void);
int ril_dial(const char *number);
int ril_hangup(void);
int ril_send_sms(const char *number, const char *message);
int ril_activate_data(void);
int ril_deactivate_data(void);

int ims_init(void);
int ims_is_registered(void);
int ims_volte_enable(void);
int ims_volte_disable(void);
int ims_wifi_calling_enable(void);
int ims_wifi_calling_disable(void);

#endif

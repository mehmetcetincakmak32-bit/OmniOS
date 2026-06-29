#include "power_hal.h"
#include <stdio.h>
#include <stdint.h>

static battery_info_t _battery_info;
static bool _charging_enabled = false;

status_t power_hal_init(void) {
    printf("[Power HAL] Güç yönetimi başlatılıyor...\n");
    _battery_info.level = 85;
    _battery_info.voltage_mv = 3850;
    _battery_info.capacity_mah = 4500;
    _battery_info.charging = false;
    _battery_info.plugged = true;
    _battery_info.usb_present = false;
    _battery_info.wireless_present = true;
    _battery_info.temperature_c = 32;
    _battery_info.health = 1;
    _battery_info.timestamp = 0;
    printf("[Power HAL] Batarya: %d%%, %dmV (%dmAh), %d°C\n",
           _battery_info.level, _battery_info.voltage_mv, _battery_info.capacity_mah,
           _battery_info.temperature_c);
    return STATUS_SUCCESS;
}

void power_hal_shutdown(void) {
    printf("[Power HAL] Güç yönetimi kapatılıyor...\n");
}

battery_info_t* power_hal_get_battery_info(void) {
    static battery_info_t info = {0};
    info = _battery_info;
    info.timestamp = info.timestamp + 1;
    return &info;
}

status_t power_hal_set_charge_enable(bool enable) {
    _charging_enabled = enable;
    _battery_info.charging = enable;
    printf("[Power HAL] Şarj %s\n", enable ? "aktif" : "pasif");
    return STATUS_SUCCESS;
}

bool power_hal_is_charging(void) {
    return _battery_info.charging;
}

uint32_t power_hal_get_level(void) {
    return _battery_info.level;
}

power_stats_t* power_hal_get_stats(void) {
    static power_stats_t stats = {0};
    memset(&stats, 0, sizeof(power_stats_t));
    stats.level = _battery_info.level;
    stats.temperature_c = _battery_info.temperature_c;
    stats.cpu_freq = 2300;
    stats.screen_time_ms = 3600;
    return &stats;
}

status_t power_hal_enter_suspend(void) {
    printf("[Power HAL] Sistem askıya alınıyor...\n");
    return STATUS_SUCCESS;
}

status_t power_hal_exit_suspend(void) {
    printf("[Power HAL] Sistem uyandırılıyor...\n");
    return STATUS_SUCCESS;
}
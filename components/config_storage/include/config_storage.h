// config_storage.h
#pragma once

#include "esp_err.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "nvs.h"

#define DEFAULT_AP_PASSWORD "12345678"

// ---------------------------------------------------------------------------
// Multi-device support
// ---------------------------------------------------------------------------

// Number of independently monitored Victron devices.
#define VICTRON_MAX_DEVICES 3
#define VICTRON_DEVICE_NAME_MAX 20

// Per-device configuration. Each slot is one monitored Victron device and is
// rendered as its own tab in the UI.
typedef struct {
    uint8_t enabled;                 // 0 = disabled, 1 = enabled
    char    name[VICTRON_DEVICE_NAME_MAX]; // user label (tab name)
    uint8_t record_type;             // victron_record_type_t value (0x00 = auto)
    uint8_t mac[6];                  // BLE MAC (all-zero = not yet bound)
    uint8_t aes_key[16];             // 128-bit AES key for this device
} victron_device_config_t;

// Load all device configs from NVS (returns ESP_OK and fills `configs` with
// up to `max_devices` entries, defaulting/migrating as needed).
esp_err_t load_device_configs(victron_device_config_t *configs, size_t max_devices);

// Persist all device configs to NVS.
esp_err_t save_device_configs(const victron_device_config_t *configs, size_t count);

// brightness settings
esp_err_t load_brightness(uint8_t *brightness_out);
esp_err_t save_brightness(uint8_t brightness);

// AES key handling (legacy single-key path, kept for migration + web fallback)
esp_err_t load_aes_key(uint8_t key_out[16]);
esp_err_t save_aes_key(const uint8_t key_in[16]);

// Screensaver settings
esp_err_t load_screensaver_settings(bool *enabled, uint8_t *brightness, uint16_t *timeout);
esp_err_t save_screensaver_settings(bool enabled, uint8_t brightness, uint16_t timeout);

// Wi-Fi AP settings handling (NVS namespace: "wifi")
// ssid_out and pass_out must have space for ssid_len and pass_len, respectively.
// On return, *ssid_len and *pass_len are set to the actual string lengths (including null).
esp_err_t load_wifi_config(char *ssid_out, size_t *ssid_len,
                           char *pass_out, size_t *pass_len,
                           uint8_t *enabled_out);

// Save AP settings; ssid and pass should be null-terminated strings.
esp_err_t save_wifi_config(const char *ssid,
                           const char *pass,
                           uint8_t enabled_out);

// Relay tab configuration persistence
esp_err_t load_relay_config(bool *enabled_out,
                            uint8_t *count_out,
                            uint8_t *pins_out,
                            char (*labels_out)[20],
                            size_t max_pins);

esp_err_t save_relay_config(bool enabled,
                            const uint8_t *pins,
                            const char (*labels)[20],
                            uint8_t count);

// Victron BLE debug flag persistence (NVS namespace: "debug")
esp_err_t load_victron_debug(bool *enabled_out);
esp_err_t save_victron_debug(bool enabled);

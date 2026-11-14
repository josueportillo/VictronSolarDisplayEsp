// victron_ble.h
#ifndef VICTRON_BLE_H
#define VICTRON_BLE_H

#include <stdint.h>
#include <stdbool.h>
#include "victron_records.h"
#include "config_storage.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Unified Victron BLE data container
// -----------------------------------------------------------------------------

typedef struct {
    victron_record_type_t type;  // record type (e.g., SOLAR_CHARGER, BATTERY_MONITOR)
    uint16_t              product_id; // Victron product identifier
    uint8_t               slot;   // device slot index (0..VICTRON_MAX_DEVICES-1)
    uint8_t               mac[6]; // source BLE MAC address (little-endian)
    victron_record_t      record; // parsed record data (union of all device types)
} victron_data_t;

// -----------------------------------------------------------------------------
// Public interface
// -----------------------------------------------------------------------------

// Callback for receiving new Victron data frames
typedef void (*victron_data_cb_t)(const victron_data_t *data);

// Initialize BLE scanning and decryption for Victron Smart devices
void victron_ble_init(void);

// Register a callback to receive decoded Victron BLE data
void victron_ble_register_callback(victron_data_cb_t cb);

// Enable or disable verbose/debug logging
void victron_ble_set_debug(bool enabled);

// Apply device configuration at runtime (called when Settings saves configs).
void victron_ble_apply_device_configs(const victron_device_config_t *configs, size_t count);

// Retrieve the current in-memory device configs (includes auto-learned MACs).
void victron_ble_get_device_configs(victron_device_config_t *configs, size_t max_devices);

#ifdef __cplusplus
}
#endif

#endif // VICTRON_BLE_H

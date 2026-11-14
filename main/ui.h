/* ui.h */
#ifndef UI_H
#define UI_H

#include <stdint.h>
#include <lvgl.h>
#include "victron_ble.h"
#include "ui/ui_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize all LVGL UI elements, including the device tabs, Settings, and Relay tabs.
 */
void ui_init(void);

/**
 * BLE data callback to update the UI with new panel data.
 * @param d Pointer to the victron_data_t structure containing sensor readings.
 */
void ui_on_panel_data(const victron_data_t *d);

/* Rename a device tab after its config label changed. */
void ui_rename_device_tab(ui_state_t *ui, size_t slot, const char *name);

/* Notify the UI that the user performed an activity (e.g. touch).
 * This will reset the screensaver timer and restore brightness if active.
 */
void ui_notify_user_activity(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_H */

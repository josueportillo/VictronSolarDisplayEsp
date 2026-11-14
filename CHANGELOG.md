# Changelog

## v2.0.0 — Multi-device monitoring fork

This repository is a fork of [wytr/VictronSolarDisplayEsp](https://github.com/wytr/VictronSolarDisplayEsp) with multi-device monitoring support. All changes are documented below so the diff from upstream is easy to review.

### Added

- **3 independent device slots** (`VICTRON_MAX_DEVICES = 3` in `components/config_storage/include/config_storage.h`), each with:
  - an enable flag,
  - a user label (used as the tab name),
  - a device type (Battery Monitor / DC-DC Converter / Solar Charger / Orion XS / Inverter / Smart Lithium / Auto),
  - a BLE MAC address (auto-learned, or manually set),
  - its own 16-byte AES key.
- **Per-device dashboard tabs** — replaces the original single "Live" tab with three device tabs (plus the existing Settings and Relay tabs), each updated independently.
- **MAC-based BLE routing with auto-learn** — packets are matched to a slot by MAC; a slot with a key but no MAC yet auto-binds on the first packet whose key[0] and record type both match.
- **Settings → Devices** on-device configuration page (enable / name / type / MAC / AES key per device + "Save Devices").
- **3-device SoftAP web portal** (`files/index.html`) with name + type + key per device.
- **Legacy single-key migration** — the previous `victron/aes_key` NVS entry is migrated into slot 0 on first boot.

### Changed (vs upstream)

| File | Change |
|------|--------|
| `components/config_storage/config_storage.c/.h` | Added `victron_device_config_t`, `load_device_configs()`, `save_device_configs()`, legacy-key migration |
| `components/victron_ble/victron_ble.c/.h` | Single global AES key → per-device keys + MAC routing; `victron_data_t` gains `slot` + `mac`; added `victron_ble_apply_device_configs()` |
| `main/ui.c` / `main/ui.h` / `main/ui/ui_state.h` | Single "Live" tab → 3 per-device tabs with per-slot views and routing |
| `main/ui/settings_panel.c/.h` | New "Devices" settings page; removed single MAC/AES-key fields |
| `main/ui/view_registry.c/.h` | Removed single-view `ensure` cache (per-slot views managed in `ui.c`) |
| `main/config_server.c` | `POST /save` now parses 3 devices (`nameN`/`typeN`/`keyN`) |
| `files/index.html` | 3-device configuration form |
| `README.md` | Added "Multi-Device Monitoring" section |

### Preserved (unchanged)

- Relay control — Relay tab + GPIO handling (pins 5, 6, 7, 15, 16, 46, 9, 14) untouched.
- Wi-Fi SoftAP, screensaver, display brightness, Victron BLE debug settings.

### Build

- Target: ESP32-S3 (Guition JC3248W535 3.5" 320×480 touch display)
- Toolchain: ESP-IDF v5.4.1

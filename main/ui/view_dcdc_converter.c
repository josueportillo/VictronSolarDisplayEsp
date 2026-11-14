#include "view_dcdc_converter.h"

#include <stdlib.h>
#include <string.h>

#include "ui_format.h"

typedef enum {
    DCDC_LABEL_INPUT_V = 0,
    DCDC_LABEL_OUTPUT_V,
    DCDC_LABEL_STATE,
    DCDC_LABEL_COUNT
} dcdc_label_t;

typedef struct {
    ui_device_view_t base;
    lv_obj_t *row_primary;
    lv_obj_t *value_labels[DCDC_LABEL_COUNT];
    lv_obj_t *error_label;
} ui_dcdc_view_t;

static void dcdc_view_update(ui_device_view_t *view, const victron_data_t *data);
static void dcdc_view_show(ui_device_view_t *view);
static void dcdc_view_hide(ui_device_view_t *view);
static void dcdc_view_destroy(ui_device_view_t *view);

static void format_input_voltage(lv_obj_t *label, const victron_data_t *data);
static void format_output_voltage(lv_obj_t *label, const victron_data_t *data);
static void format_state(lv_obj_t *label, const victron_data_t *data);
static const char *dcdc_state_string(uint8_t state);
static const char *dcdc_error_string(uint8_t code);

static const ui_label_descriptor_t dcdc_descriptors[DCDC_LABEL_COUNT] = {
    { "input_voltage",  "V IN",   format_input_voltage },
    { "output_voltage", "V OUT",  format_output_voltage },
    { "state",          "STATE",  format_state },
};

static lv_obj_t *create_label_box(ui_state_t *ui, lv_obj_t *parent,
                                  const ui_label_descriptor_t *desc)
{
    lv_obj_t *box = lv_obj_create(parent);
    lv_obj_set_size(box, lv_pct(30), 100);
    lv_obj_set_style_pad_all(box, 8, 0);
    lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(box, 0, 0);
    lv_obj_set_style_outline_width(box, 0, 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *header = lv_label_create(box);
    lv_label_set_text(header, desc->title ? desc->title : "");
    lv_obj_add_style(header, &ui->styles.medium, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 5);

    lv_obj_t *value = lv_label_create(box);
    lv_label_set_text(value, "--");
    lv_obj_add_style(value, &ui->styles.value, 0);
    lv_obj_align(value, LV_ALIGN_BOTTOM_MID, 0, -5);

    return value;
}

ui_device_view_t *ui_dcdc_converter_view_create(ui_state_t *ui, lv_obj_t *parent)
{
    if (ui == NULL || parent == NULL) {
        return NULL;
    }

    ui_dcdc_view_t *view = calloc(1, sizeof(*view));
    if (view == NULL) {
        return NULL;
    }

    view->base.ui = ui;
    view->base.root = lv_obj_create(parent);
    lv_obj_set_size(view->base.root, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(view->base.root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(view->base.root, 0, 0);
    lv_obj_set_style_outline_width(view->base.root, 0, 0);
    lv_obj_set_style_pad_all(view->base.root, 0, 0);
    lv_obj_clear_flag(view->base.root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(view->base.root, LV_OBJ_FLAG_HIDDEN);

    view->row_primary = lv_obj_create(view->base.root);
    lv_obj_set_size(view->row_primary, lv_pct(100), 100);
    lv_obj_set_flex_flow(view->row_primary, LV_STYLE_PAD_ROW);
    lv_obj_set_flex_align(view->row_primary,
                          LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(view->row_primary, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(view->row_primary, 0, 0);
    lv_obj_set_style_outline_width(view->row_primary, 0, 0);

    for (size_t i = 0; i < DCDC_LABEL_COUNT; ++i) {
        view->value_labels[i] = create_label_box(ui, view->row_primary,
                                                 &dcdc_descriptors[i]);
    }

    view->error_label = lv_label_create(view->base.root);
    lv_obj_add_style(view->error_label, &ui->styles.small, 0);
    lv_label_set_text(view->error_label, "OK");
    lv_obj_align(view->error_label, LV_ALIGN_BOTTOM_MID, 0, -12);

    view->base.update = dcdc_view_update;
    view->base.show = dcdc_view_show;
    view->base.hide = dcdc_view_hide;
    view->base.destroy = dcdc_view_destroy;

    return &view->base;
}

static ui_dcdc_view_t *dcdc_view_from_base(ui_device_view_t *base)
{
    return (ui_dcdc_view_t *)base;
}

static void dcdc_view_update(ui_device_view_t *view, const victron_data_t *data)
{
    ui_dcdc_view_t *dcdc = dcdc_view_from_base(view);
    if (dcdc == NULL || data == NULL || data->type != VICTRON_BLE_RECORD_DCDC_CONVERTER) {
        return;
    }

    for (size_t i = 0; i < DCDC_LABEL_COUNT; ++i) {
        if (dcdc->value_labels[i] != NULL) {
            dcdc_descriptors[i].formatter(dcdc->value_labels[i], data);
        }
    }

    const victron_record_dcdc_converter_t *d = &data->record.dcdc;
    if (dcdc->error_label) {
        lv_label_set_text(dcdc->error_label, dcdc_error_string(d->charger_error));
    }

    if (view->ui && view->ui->lbl_error) {
        lv_label_set_text(view->ui->lbl_error, dcdc_error_string(d->charger_error));
    }
}

static void dcdc_view_show(ui_device_view_t *view)
{
    if (view && view->root) {
        lv_obj_clear_flag(view->root, LV_OBJ_FLAG_HIDDEN);
    }
}

static void dcdc_view_hide(ui_device_view_t *view)
{
    if (view && view->root) {
        lv_obj_add_flag(view->root, LV_OBJ_FLAG_HIDDEN);
    }
}

static void dcdc_view_destroy(ui_device_view_t *view)
{
    if (view == NULL) {
        return;
    }
    if (view->root) {
        lv_obj_del(view->root);
        view->root = NULL;
    }
    free(view);
}

static void format_input_voltage(lv_obj_t *label, const victron_data_t *data)
{
    if (label == NULL || data == NULL || data->type != VICTRON_BLE_RECORD_DCDC_CONVERTER) {
        return;
    }
    const victron_record_dcdc_converter_t *d = &data->record.dcdc;
    ui_label_set_unsigned_fixed(label, d->input_voltage_centi, 100, 2, " V");
}

static void format_output_voltage(lv_obj_t *label, const victron_data_t *data)
{
    if (label == NULL || data == NULL || data->type != VICTRON_BLE_RECORD_DCDC_CONVERTER) {
        return;
    }
    const victron_record_dcdc_converter_t *d = &data->record.dcdc;
    ui_label_set_unsigned_fixed(label, d->output_voltage_centi, 100, 2, " V");
}

static void format_state(lv_obj_t *label, const victron_data_t *data)
{
    if (label == NULL || data == NULL || data->type != VICTRON_BLE_RECORD_DCDC_CONVERTER) {
        return;
    }
    const victron_record_dcdc_converter_t *d = &data->record.dcdc;
    lv_label_set_text(label, dcdc_state_string(d->device_state));
}

static const char *dcdc_state_string(uint8_t state)
{
    switch (state) {
    case 0:  return "Off";
    case 1:  return "LowPwr";
    case 2:  return "Fault";
    case 3:  return "Bulk";
    case 4:  return "Absorb";
    case 5:  return "Float";
    case 6:  return "Storage";
    case 7:  return "Equaliz";
    case 8:  return "Equaliz";
    case 9:  return "Invert";
    case 10: return "PwrSup";
    case 11: return "Start";
    default: return "Unknown";
    }
}

static const char *dcdc_error_string(uint8_t code)
{
    switch (code) {
    case 0:   return "OK";
    case 1:   return "Battery temp high";
    case 2:   return "Battery volt high";
    case 3:
    case 4:   return "Remote temp sensor";
    case 5:   return "Remote temp lost";
    case 6:
    case 7:   return "Remote volt sense";
    case 8:   return "Remote volt lost";
    case 11:  return "High ripple";
    case 14:  return "Temp too low";
    case 17:  return "Charger temp";
    case 18:  return "Over current";
    case 19:  return "Polarity";
    case 26:  return "Overheated";
    case 27:  return "Short circuit";
    case 33:  return "Input volt high";
    case 34:  return "Input curr high";
    case 38:  return "Input shutdown";
    case 114: return "CPU temp";
    case 116: return "Cal lost";
    default:  return "Unknown error";
    }
}

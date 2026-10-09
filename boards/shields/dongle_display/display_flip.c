/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 *
 * &dflip: rotate the SH1106/SSD1306 display 180 degrees at runtime.
 *
 * The Zephyr driver only applies segment-remap / com-invdir at init and has no
 * orientation API, so this sends the two scan-direction commands over I2C
 * itself and inverts whatever the devicetree set. Segment remap only affects
 * data written afterwards, so the whole screen is invalidated to redraw it.
 * The state is saved under "dongle_display/flipped".
 */

#define DT_DRV_COMPAT zmk_behavior_display_flip

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

#include <drivers/behavior.h>
#include <lvgl.h>

#include <zmk/behavior.h>
#include <zmk/display.h>

#include "display_flip.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define DISPLAY_NODE DT_CHOSEN(zephyr_display)

#if DT_HAS_CHOSEN(zephyr_display) && DT_ON_BUS(DISPLAY_NODE, i2c)

#define CONTROL_ALL_BYTES_CMD 0x00
#define SET_SEGMENT_MAP_NORMAL 0xa0
#define SET_SEGMENT_MAP_REMAPPED 0xa1
#define SET_COM_OUTPUT_SCAN_NORMAL 0xc0
#define SET_COM_OUTPUT_SCAN_FLIPPED 0xc8

static const struct i2c_dt_spec display_bus = I2C_DT_SPEC_GET(DISPLAY_NODE);

static bool flipped;

static int send_orientation(void) {
    bool segment_remap = DT_PROP(DISPLAY_NODE, segment_remap) ^ flipped;
    bool com_invdir = DT_PROP(DISPLAY_NODE, com_invdir) ^ flipped;
    uint8_t cmds[] = {
        segment_remap ? SET_SEGMENT_MAP_REMAPPED : SET_SEGMENT_MAP_NORMAL,
        com_invdir ? SET_COM_OUTPUT_SCAN_FLIPPED : SET_COM_OUTPUT_SCAN_NORMAL,
    };

    return i2c_burst_write_dt(&display_bus, CONTROL_ALL_BYTES_CMD, cmds, sizeof(cmds));
}

void zmk_dongle_display_flip_apply(void) {
    if (!flipped) {
        return;
    }

    int err = send_orientation();
    if (err) {
        LOG_ERR("Failed to apply display flip (%d)", err);
    }
}

static void toggle_flip_cb(struct k_work *work) {
    flipped = !flipped;

    if (zmk_display_is_initialized()) {
        int err = send_orientation();
        if (err) {
            LOG_ERR("Failed to flip display (%d)", err);
        }
        lv_obj_invalidate(lv_screen_active());
    }

#if IS_ENABLED(CONFIG_SETTINGS)
    settings_save_one("dongle_display/flipped", &flipped, sizeof(flipped));
#endif
}

static K_WORK_DEFINE(toggle_flip_work, toggle_flip_cb);

#if IS_ENABLED(CONFIG_SETTINGS)
static int flip_settings_set(const char *name, size_t len, settings_read_cb read_cb,
                             void *cb_arg) {
    if (!settings_name_steq(name, "flipped", NULL)) {
        return -ENOENT;
    }
    if (len != sizeof(flipped)) {
        return -EINVAL;
    }

    int rc = read_cb(cb_arg, &flipped, sizeof(flipped));
    return rc < 0 ? rc : 0;
}

SETTINGS_STATIC_HANDLER_DEFINE(dongle_display_flip, "dongle_display", NULL, flip_settings_set,
                               NULL, NULL);
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int on_flip_pressed(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event) {
    k_work_submit_to_queue(zmk_display_work_q(), &toggle_flip_work);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_flip_released(struct zmk_behavior_binding *binding,
                            struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_display_flip_driver_api = {
    .binding_pressed = on_flip_pressed,
    .binding_released = on_flip_released,
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define DFLIP_INST(n)                                                                              \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                   \
                            &behavior_display_flip_driver_api);

DT_INST_FOREACH_STATUS_OKAY(DFLIP_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */

#else

void zmk_dongle_display_flip_apply(void) {}

#endif

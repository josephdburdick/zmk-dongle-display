/*
 * Stage: shows the resting eight-ball when idle, typing stats while keys are
 * pressed, and the rolling eight-ball while the pointer moves on the scroll layer.
 *
 * SPDX-License-Identifier: MIT
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/keymap.h>

#include "stage.h"

#define TICK_PERIOD_MS 250
#define STAGE_W 84
#define STAGE_H 44

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

static void show(struct zmk_widget_stage *widget, enum stage_state state) {
    if (state == widget->shown) {
        return;
    }
    bool typing = state == STAGE_TYPING;
#if IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_TYPING_STATS)
    if (typing && widget->shown == STAGE_IDLE) {
        zmk_widget_typing_stats_reset(&widget->stats);
    }
    zmk_widget_typing_stats_set_active(&widget->stats, typing);
    if (typing) {
        lv_obj_remove_flag(zmk_widget_typing_stats_obj(&widget->stats), LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(zmk_widget_eight_ball_obj(&widget->ball), LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(zmk_widget_typing_stats_obj(&widget->stats), LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(zmk_widget_eight_ball_obj(&widget->ball), LV_OBJ_FLAG_HIDDEN);
    }
#else
    ARG_UNUSED(typing);
#endif
    widget->shown = state;
}

/* ---- tick timer (display thread) ---- */

static void tick_cb(lv_timer_t *timer) {
    struct zmk_widget_stage *widget = lv_timer_get_user_data(timer);
    show(widget, stage_fsm_tick(&widget->fsm, k_uptime_get()));
}

/* ---- key presses ---- */

struct stage_key_state {
    bool pressed;
    int64_t timestamp;
};

static void stage_key_update_cb(struct stage_key_state state) {
    if (!state.pressed) {
        return;
    }
    struct zmk_widget_stage *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        show(widget, stage_fsm_key_press(&widget->fsm, state.timestamp));
    }
}

static struct stage_key_state stage_key_get_state(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);
    if (ev == NULL) {
        return (struct stage_key_state){.pressed = false, .timestamp = 0};
    }
    return (struct stage_key_state){.pressed = ev->state, .timestamp = ev->timestamp};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_stage_keys, struct stage_key_state, stage_key_update_cb,
                            stage_key_get_state)
ZMK_SUBSCRIPTION(widget_stage_keys, zmk_position_state_changed);

/* ---- pointer motion (input thread -> display thread) ---- */

#if DT_HAS_CHOSEN(zmk_dongle_display_pointer)

static atomic_t pending_delta = ATOMIC_INIT(0);

static void scroll_work_cb(struct k_work *work) {
    ARG_UNUSED(work);
    int32_t delta = (int32_t)atomic_set(&pending_delta, 0);
    if (delta == 0) {
        return;
    }
    struct zmk_widget_stage *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        show(widget, stage_fsm_scroll_motion(&widget->fsm, k_uptime_get()));
        zmk_widget_eight_ball_roll(&widget->ball, delta);
    }
}

K_WORK_DEFINE(scroll_work, scroll_work_cb);

static void pointer_cb(struct input_event *evt, void *user_data) {
    ARG_UNUSED(user_data);
#if IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_EIGHT_BALL_AXIS_X)
    const uint16_t axis = INPUT_REL_X;
#else
    const uint16_t axis = INPUT_REL_Y;
#endif
    if (evt->type != INPUT_EV_REL || evt->code != axis || evt->value == 0) {
        return;
    }
    if (!zmk_keymap_layer_active(CONFIG_ZMK_DONGLE_DISPLAY_SCROLL_LAYER)) {
        return;
    }
    atomic_add(&pending_delta, evt->value);
    if (zmk_display_is_initialized()) {
        k_work_submit_to_queue(zmk_display_work_q(), &scroll_work);
    }
}

INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_CHOSEN(zmk_dongle_display_pointer)), pointer_cb, NULL);

#endif /* DT_HAS_CHOSEN(zmk_dongle_display_pointer) */

/* ---- init ---- */

int zmk_widget_stage_init(struct zmk_widget_stage *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, STAGE_W, STAGE_H);
    lv_obj_set_style_pad_all(widget->obj, 0, 0);
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    lv_obj_remove_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);

    zmk_widget_eight_ball_init(&widget->ball, widget->obj);
    lv_obj_center(zmk_widget_eight_ball_obj(&widget->ball));

#if IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_TYPING_STATS)
    zmk_widget_typing_stats_init(&widget->stats, widget->obj);
    lv_obj_align(zmk_widget_typing_stats_obj(&widget->stats), LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_flag(zmk_widget_typing_stats_obj(&widget->stats), LV_OBJ_FLAG_HIDDEN);
#endif

    stage_fsm_init(&widget->fsm, CONFIG_ZMK_DONGLE_DISPLAY_TYPING_TIMEOUT_MS,
                   CONFIG_ZMK_DONGLE_DISPLAY_SCROLL_TIMEOUT_MS);
    widget->shown = STAGE_IDLE;
    widget->tick = lv_timer_create(tick_cb, TICK_PERIOD_MS, widget);

    sys_slist_append(&widgets, &widget->node);
    widget_stage_keys_init();
    return 0;
}

lv_obj_t *zmk_widget_stage_obj(struct zmk_widget_stage *widget) { return widget->obj; }

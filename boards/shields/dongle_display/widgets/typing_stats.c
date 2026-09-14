/*
 * SPDX-License-Identifier: MIT
 */
#include <string.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/wpm.h>

#include "typing_stats.h"

#define SAMPLE_PERIOD_MS 500
#define MIN_SCALE_WPM 40

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);
static lv_style_t line_style;

struct typing_stats_state {
    uint8_t wpm;
};

static void set_label(struct zmk_widget_typing_stats *widget, uint8_t wpm) {
    char text[5];
    snprintf(text, sizeof(text), "%u", wpm);
    lv_label_set_text(widget->wpm_label, text);
}

static void redraw_line(struct zmk_widget_typing_stats *widget) {
    uint8_t scale = widget->peak > MIN_SCALE_WPM ? widget->peak : MIN_SCALE_WPM;
    for (int i = 0; i < TYPING_STATS_SAMPLES; i++) {
        int32_t y = TYPING_STATS_LINE_H - ((int32_t)widget->samples[i] * TYPING_STATS_LINE_H) / scale;
        widget->points[i].x = (i * TYPING_STATS_LINE_W) / (TYPING_STATS_SAMPLES - 1);
        widget->points[i].y = y;
    }
    lv_line_set_points(widget->line, widget->points, TYPING_STATS_SAMPLES);
}

static void push_sample(struct zmk_widget_typing_stats *widget, uint8_t wpm) {
    memmove(widget->samples, widget->samples + 1, TYPING_STATS_SAMPLES - 1);
    widget->samples[TYPING_STATS_SAMPLES - 1] = wpm;
    if (wpm > widget->peak) {
        widget->peak = wpm;
    }
    redraw_line(widget);
}

static void sample_timer_cb(lv_timer_t *timer) {
    struct zmk_widget_typing_stats *widget = lv_timer_get_user_data(timer);
    int wpm = zmk_wpm_get_state();
    push_sample(widget, wpm > 255 ? 255 : (uint8_t)wpm);
}

static void typing_stats_update_cb(struct typing_stats_state state) {
    struct zmk_widget_typing_stats *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_label(widget, state.wpm); }
}

static struct typing_stats_state typing_stats_get_state(const zmk_event_t *eh) {
    const struct zmk_wpm_state_changed *ev = as_zmk_wpm_state_changed(eh);
    int wpm = ev ? ev->state : zmk_wpm_get_state();
    return (struct typing_stats_state){.wpm = wpm > 255 ? 255 : (uint8_t)wpm};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_typing_stats, struct typing_stats_state, typing_stats_update_cb,
                            typing_stats_get_state)
ZMK_SUBSCRIPTION(widget_typing_stats, zmk_wpm_state_changed);

int zmk_widget_typing_stats_init(struct zmk_widget_typing_stats *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 84, 44);
    lv_obj_set_style_pad_all(widget->obj, 0, 0);
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_remove_flag(widget->obj, LV_OBJ_FLAG_SCROLLABLE);

    /* Fixed positions: the number grows to three 8 px glyphs, the unit sits past that. */
    widget->wpm_label = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->wpm_label, &lv_font_unscii_16, 0);
    lv_obj_set_width(widget->wpm_label, 26);
    lv_obj_set_pos(widget->wpm_label, 2, 1);
    lv_label_set_text(widget->wpm_label, "0");

    widget->unit_label = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->unit_label, &lv_font_unscii_8, 0);
    lv_label_set_text(widget->unit_label, "wpm");
    lv_obj_set_pos(widget->unit_label, 32, 8);

    lv_style_init(&line_style);
    lv_style_set_line_width(&line_style, 1);
    lv_style_set_line_color(&line_style, lv_color_black());
    lv_style_set_line_rounded(&line_style, false);

    widget->line = lv_line_create(widget->obj);
    lv_obj_add_style(widget->line, &line_style, 0);
    lv_obj_set_pos(widget->line, TYPING_STATS_LINE_X, TYPING_STATS_LINE_Y);
    lv_obj_set_size(widget->line, TYPING_STATS_LINE_W + 1, TYPING_STATS_LINE_H + 1);

    memset(widget->samples, 0, sizeof(widget->samples));
    widget->peak = 0;
    redraw_line(widget);

    widget->timer = lv_timer_create(sample_timer_cb, SAMPLE_PERIOD_MS, widget);
    lv_timer_pause(widget->timer);

    sys_slist_append(&widgets, &widget->node);
    widget_typing_stats_init();
    return 0;
}

lv_obj_t *zmk_widget_typing_stats_obj(struct zmk_widget_typing_stats *widget) { return widget->obj; }

void zmk_widget_typing_stats_reset(struct zmk_widget_typing_stats *widget) {
    memset(widget->samples, 0, sizeof(widget->samples));
    widget->peak = 0;
    redraw_line(widget);
}

void zmk_widget_typing_stats_set_active(struct zmk_widget_typing_stats *widget, bool active) {
    if (active) {
        lv_timer_resume(widget->timer);
        lv_timer_reset(widget->timer);
    } else {
        lv_timer_pause(widget->timer);
    }
}

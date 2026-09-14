/*
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

#define TYPING_STATS_SAMPLES 40
#define TYPING_STATS_LINE_W 78
#define TYPING_STATS_LINE_H 20
#define TYPING_STATS_LINE_X 2
#define TYPING_STATS_LINE_Y 22

struct zmk_widget_typing_stats {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *wpm_label;
    lv_obj_t *unit_label;
    lv_obj_t *line;
    lv_point_precise_t points[TYPING_STATS_SAMPLES];
    uint8_t samples[TYPING_STATS_SAMPLES];
    uint8_t peak;
    lv_timer_t *timer;
};

int zmk_widget_typing_stats_init(struct zmk_widget_typing_stats *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_typing_stats_obj(struct zmk_widget_typing_stats *widget);
void zmk_widget_typing_stats_reset(struct zmk_widget_typing_stats *widget);
void zmk_widget_typing_stats_set_active(struct zmk_widget_typing_stats *widget, bool active);

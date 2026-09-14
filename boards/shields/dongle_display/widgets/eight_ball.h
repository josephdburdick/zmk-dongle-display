/*
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <lvgl.h>
#include <stdint.h>

#define EIGHT_BALL_FRAMES 12

struct zmk_widget_eight_ball {
    lv_obj_t *obj;
    int32_t accum;
    uint8_t frame;
};

int zmk_widget_eight_ball_init(struct zmk_widget_eight_ball *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_eight_ball_obj(struct zmk_widget_eight_ball *widget);
/* Advance the animation by a pointer delta. Must be called from the display work queue. */
void zmk_widget_eight_ball_roll(struct zmk_widget_eight_ball *widget, int32_t delta);

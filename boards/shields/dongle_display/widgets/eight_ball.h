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
    int64_t last_frame_ms;
};

int zmk_widget_eight_ball_init(struct zmk_widget_eight_ball *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_eight_ball_obj(struct zmk_widget_eight_ball *widget);
/* Feed a pointer delta; advances at most one frame per MIN_FRAME_MS. Display work queue only. */
void zmk_widget_eight_ball_roll(struct zmk_widget_eight_ball *widget, int32_t delta, int64_t now_ms);
/* Drop any leftover motion so the ball stops cleanly. */
void zmk_widget_eight_ball_settle(struct zmk_widget_eight_ball *widget);

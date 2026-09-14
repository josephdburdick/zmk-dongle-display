/*
 * SPDX-License-Identifier: MIT
 */
#include "eight_ball.h"

#ifndef CONFIG_ZMK_DONGLE_DISPLAY_EIGHT_BALL_STEP_DIV
#define CONFIG_ZMK_DONGLE_DISPLAY_EIGHT_BALL_STEP_DIV 8
#endif

LV_IMG_DECLARE(eight_ball_00);
LV_IMG_DECLARE(eight_ball_01);
LV_IMG_DECLARE(eight_ball_02);
LV_IMG_DECLARE(eight_ball_03);
LV_IMG_DECLARE(eight_ball_04);
LV_IMG_DECLARE(eight_ball_05);
LV_IMG_DECLARE(eight_ball_06);
LV_IMG_DECLARE(eight_ball_07);
LV_IMG_DECLARE(eight_ball_08);
LV_IMG_DECLARE(eight_ball_09);
LV_IMG_DECLARE(eight_ball_10);
LV_IMG_DECLARE(eight_ball_11);

static const lv_img_dsc_t *frames[EIGHT_BALL_FRAMES] = {
    &eight_ball_00, &eight_ball_01, &eight_ball_02, &eight_ball_03,
    &eight_ball_04, &eight_ball_05, &eight_ball_06, &eight_ball_07,
    &eight_ball_08, &eight_ball_09, &eight_ball_10, &eight_ball_11,
};

int zmk_widget_eight_ball_init(struct zmk_widget_eight_ball *widget, lv_obj_t *parent) {
    widget->obj = lv_img_create(parent);
    widget->accum = 0;
    widget->frame = 0;
    lv_img_set_src(widget->obj, frames[0]);
    return 0;
}

lv_obj_t *zmk_widget_eight_ball_obj(struct zmk_widget_eight_ball *widget) { return widget->obj; }

void zmk_widget_eight_ball_roll(struct zmk_widget_eight_ball *widget, int32_t delta) {
    const int32_t step = CONFIG_ZMK_DONGLE_DISPLAY_EIGHT_BALL_STEP_DIV;
    uint8_t before = widget->frame;
    widget->accum += delta;
    while (widget->accum >= step) {
        widget->accum -= step;
        widget->frame = (widget->frame + 1) % EIGHT_BALL_FRAMES;
    }
    while (widget->accum <= -step) {
        widget->accum += step;
        widget->frame = (widget->frame + EIGHT_BALL_FRAMES - 1) % EIGHT_BALL_FRAMES;
    }
    if (widget->frame != before) {
        lv_img_set_src(widget->obj, frames[widget->frame]);
    }
}

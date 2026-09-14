/*
 * SPDX-License-Identifier: MIT
 */
#include "eight_ball.h"

#ifndef CONFIG_ZMK_DONGLE_DISPLAY_EIGHT_BALL_STEP_DIV
#define CONFIG_ZMK_DONGLE_DISPLAY_EIGHT_BALL_STEP_DIV 24
#endif
#ifndef CONFIG_ZMK_DONGLE_DISPLAY_EIGHT_BALL_MIN_FRAME_MS
#define CONFIG_ZMK_DONGLE_DISPLAY_EIGHT_BALL_MIN_FRAME_MS 60
#endif
#ifndef CONFIG_ZMK_DONGLE_DISPLAY_EIGHT_BALL_MAX_FRAMES_PER_UPDATE
#define CONFIG_ZMK_DONGLE_DISPLAY_EIGHT_BALL_MAX_FRAMES_PER_UPDATE 3
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

static const lv_img_dsc_t *frames_src[EIGHT_BALL_FRAMES] = {
    &eight_ball_00, &eight_ball_01, &eight_ball_02, &eight_ball_03,
    &eight_ball_04, &eight_ball_05, &eight_ball_06, &eight_ball_07,
    &eight_ball_08, &eight_ball_09, &eight_ball_10, &eight_ball_11,
};

int zmk_widget_eight_ball_init(struct zmk_widget_eight_ball *widget, lv_obj_t *parent) {
    widget->obj = lv_img_create(parent);
    widget->accum = 0;
    widget->frame = 0;
    widget->last_frame_ms = 0;
    lv_img_set_src(widget->obj, frames_src[0]);
    return 0;
}

lv_obj_t *zmk_widget_eight_ball_obj(struct zmk_widget_eight_ball *widget) { return widget->obj; }

void zmk_widget_eight_ball_roll(struct zmk_widget_eight_ball *widget, int32_t delta, int64_t now_ms) {
    const int32_t step = CONFIG_ZMK_DONGLE_DISPLAY_EIGHT_BALL_STEP_DIV;
    const int32_t max_frames = CONFIG_ZMK_DONGLE_DISPLAY_EIGHT_BALL_MAX_FRAMES_PER_UPDATE;
    const int32_t limit = max_frames * step;
    widget->accum += delta;
    if (widget->accum > limit) {
        widget->accum = limit;
    } else if (widget->accum < -limit) {
        widget->accum = -limit;
    }
    if (now_ms - widget->last_frame_ms < CONFIG_ZMK_DONGLE_DISPLAY_EIGHT_BALL_MIN_FRAME_MS) {
        return;
    }
    int32_t frames = widget->accum / step; /* truncates toward zero */
    if (frames == 0) {
        return;
    }
    widget->accum -= frames * step;
    widget->frame = (uint8_t)(((int32_t)widget->frame + frames + EIGHT_BALL_FRAMES * 4) % EIGHT_BALL_FRAMES);
    widget->last_frame_ms = now_ms;
    lv_img_set_src(widget->obj, frames_src[widget->frame]);
}

void zmk_widget_eight_ball_settle(struct zmk_widget_eight_ball *widget) { widget->accum = 0; }

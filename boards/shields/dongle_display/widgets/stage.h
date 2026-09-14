/*
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

#include "stage_fsm.h"
#include "eight_ball.h"
#include "typing_stats.h"

struct zmk_widget_stage {
    sys_snode_t node;
    lv_obj_t *obj;
    struct zmk_widget_eight_ball ball;
    struct zmk_widget_typing_stats stats;
    struct stage_fsm fsm;
    lv_timer_t *tick;
    enum stage_state shown;
};

int zmk_widget_stage_init(struct zmk_widget_stage *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_stage_obj(struct zmk_widget_stage *widget);

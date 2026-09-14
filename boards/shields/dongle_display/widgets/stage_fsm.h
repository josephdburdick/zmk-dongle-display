/*
 * Stage state machine for the dongle display: decides whether the stage
 * shows the resting 8-ball (IDLE), typing stats (TYPING), or the rolling
 * 8-ball (SCROLLING). Pure C: no Zephyr, no LVGL, host-testable.
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdint.h>

enum stage_state {
    STAGE_IDLE = 0,
    STAGE_TYPING = 1,
    STAGE_SCROLLING = 2,
};

struct stage_fsm {
    enum stage_state state;
    int64_t last_key_ms;
    int64_t last_scroll_ms;
    int64_t typing_timeout_ms;
    int64_t scroll_timeout_ms;
};

void stage_fsm_init(struct stage_fsm *f, int64_t typing_timeout_ms, int64_t scroll_timeout_ms);
enum stage_state stage_fsm_key_press(struct stage_fsm *f, int64_t now_ms);
enum stage_state stage_fsm_scroll_motion(struct stage_fsm *f, int64_t now_ms);
enum stage_state stage_fsm_tick(struct stage_fsm *f, int64_t now_ms);

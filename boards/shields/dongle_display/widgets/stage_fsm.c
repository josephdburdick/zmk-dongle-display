/*
 * SPDX-License-Identifier: MIT
 */
#include "stage_fsm.h"

void stage_fsm_init(struct stage_fsm *f, int64_t typing_timeout_ms, int64_t scroll_timeout_ms) {
    f->state = STAGE_IDLE;
    f->last_key_ms = INT64_MIN / 2;
    f->last_scroll_ms = INT64_MIN / 2;
    f->typing_timeout_ms = typing_timeout_ms;
    f->scroll_timeout_ms = scroll_timeout_ms;
}

enum stage_state stage_fsm_key_press(struct stage_fsm *f, int64_t now_ms) {
    f->last_key_ms = now_ms;
    if (f->state != STAGE_SCROLLING) {
        f->state = STAGE_TYPING;
    }
    return f->state;
}

enum stage_state stage_fsm_scroll_motion(struct stage_fsm *f, int64_t now_ms) {
    f->last_scroll_ms = now_ms;
    f->state = STAGE_SCROLLING;
    return f->state;
}

enum stage_state stage_fsm_tick(struct stage_fsm *f, int64_t now_ms) {
    switch (f->state) {
    case STAGE_SCROLLING:
        if (now_ms - f->last_scroll_ms >= f->scroll_timeout_ms) {
            f->state = STAGE_IDLE;
        }
        break;
    case STAGE_TYPING:
        if (now_ms - f->last_key_ms >= f->typing_timeout_ms) {
            f->state = STAGE_IDLE;
        }
        break;
    case STAGE_IDLE:
        break;
    }
    return f->state;
}

#include <assert.h>
#include <stdio.h>
#include "../../boards/shields/dongle_display/widgets/stage_fsm.h"

static struct stage_fsm f;

static void reset(void) { stage_fsm_init(&f, 4000, 1000); }

static void test_boot_is_idle(void) {
    reset();
    assert(f.state == STAGE_IDLE);
}

static void test_key_press_enters_typing(void) {
    reset();
    assert(stage_fsm_key_press(&f, 100) == STAGE_TYPING);
}

static void test_typing_times_out_to_idle(void) {
    reset();
    stage_fsm_key_press(&f, 100);
    assert(stage_fsm_tick(&f, 100 + 3999) == STAGE_TYPING);
    assert(stage_fsm_tick(&f, 100 + 4000) == STAGE_IDLE);
}

static void test_key_press_extends_typing(void) {
    reset();
    stage_fsm_key_press(&f, 0);
    stage_fsm_key_press(&f, 3000);
    assert(stage_fsm_tick(&f, 6999) == STAGE_TYPING);
    assert(stage_fsm_tick(&f, 7000) == STAGE_IDLE);
}

static void test_scroll_enters_scrolling_from_idle_and_typing(void) {
    reset();
    assert(stage_fsm_scroll_motion(&f, 10) == STAGE_SCROLLING);
    reset();
    stage_fsm_key_press(&f, 10);
    assert(stage_fsm_scroll_motion(&f, 20) == STAGE_SCROLLING);
}

static void test_scroll_wins_over_key_press(void) {
    reset();
    stage_fsm_scroll_motion(&f, 10);
    assert(stage_fsm_key_press(&f, 20) == STAGE_SCROLLING);
}

static void test_scroll_times_out_to_idle(void) {
    reset();
    stage_fsm_scroll_motion(&f, 10);
    assert(stage_fsm_tick(&f, 10 + 999) == STAGE_SCROLLING);
    assert(stage_fsm_tick(&f, 10 + 1000) == STAGE_IDLE);
}

static void test_scroll_motion_extends_scrolling(void) {
    reset();
    stage_fsm_scroll_motion(&f, 0);
    stage_fsm_scroll_motion(&f, 900);
    assert(stage_fsm_tick(&f, 1899) == STAGE_SCROLLING);
    assert(stage_fsm_tick(&f, 1900) == STAGE_IDLE);
}

int main(void) {
    test_boot_is_idle();
    test_key_press_enters_typing();
    test_typing_times_out_to_idle();
    test_key_press_extends_typing();
    test_scroll_enters_scrolling_from_idle_and_typing();
    test_scroll_wins_over_key_press();
    test_scroll_times_out_to_idle();
    test_scroll_motion_extends_scrolling();
    printf("stage_fsm: 8 tests passed\n");
    return 0;
}

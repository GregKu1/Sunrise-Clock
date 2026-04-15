#pragma once

#include <Arduino.h>
#include "common.h"

void ui_init();

void ui_draw_test();

void ui_draw_display_mode(struct tm *time_now);

void ui_draw_edit_mode(alarm_t *week);

bool ui_handle_click(alarm_t *week);

void ui_handle_long_press(alarm_t *week);

void ui_handle_encoder_increment(alarm_t *week);

void ui_handle_encoder_decrement(alarm_t *week);

bool ui_is_edit_mode();


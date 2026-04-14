#pragma once

#include <Arduino.h>
#include "types.h"

void ui_init();

void ui_draw_display_mode(struct tm *time_now);

void ui_draw_edit_mode(alarm_t *week, int selected_day);

void ui_handle_click();

void ui_handle_long_press();

void ui_handle_encoder_increment(alarm_t *week);

void ui_handle_encoder_decrement(alarm_t *week);

bool ui_is_edit_mode();


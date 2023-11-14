// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "qp.h"
#    include "sipo_pins.h"
#    if defined(INIT_EE_HANDS_LEFT)
#        include "qp_surface.h"
#        include "qp_eink_panel.h"
extern painter_device_t il91874;
#    else
extern painter_device_t ili9163;
extern painter_device_t ili9341;
#    endif // defined(INIT_EE_HANDS_LEFT)
#endif // defined(QUANTUM_PAINTER_ENABLE)

#if defined(RGB_MATRIX_ENABLE) && defined(ONE_HAND_ENABLE)
typedef enum one_hand_movement_t{
    DIRECTION_NONE,
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} one_hand_movement_t;
extern uint8_t one_hand_col;
extern uint8_t one_hand_row;
extern one_hand_movement_t one_hand_movement;
#endif // RGB_MATRIX_ENABLE && ONE_HAND_ENABLE

#if defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)
#    include "touch_driver.h"
extern touch_device_t ili9341_touch;
extern bool ili9341_pressed;
#    if defined(ONE_HAND_ENABLE)
void screen_one_hand(touch_report_t touch_report);
#    endif // ONE_HAND_ENABLE
#endif // TOUCH_SCREEN_ENABLE

// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include "logging.h"

#if defined(ONE_HAND_ENABLE)
uint8_t one_hand_col;
uint8_t one_hand_row;
one_hand_movement_t one_hand_movement;
#endif // ONE_HAND_ENABLE

#if defined(QUANTUM_PAINTER_ENABLE)
// eInk is on left side, dont allocate framebuffer on right
#    if defined(INIT_EE_HANDS_LEFT)
configure_sipo_pins(
    __PADDING__,
    IL91874_RST_PIN,
    IL91874_SRAM_CS_PIN,
    SCREENS_DC_PIN,
    IL91874_CS_PIN
);
painter_device_t il91874;
uint8_t il91874_buffer[EINK_BYTES_REQD(IL91874_WIDTH, IL91874_HEIGHT)];
#    else
configure_sipo_pins(
    ILI9341_TOUCH_CS_PIN,
    SCREENS_DC_PIN,
    ILI9163_RST_PIN,
    ILI9163_CS_PIN,
    ILI9341_RST_PIN,
    ILI9341_CS_PIN
);
painter_device_t ili9163;
painter_device_t ili9341;
#    endif // defined(INIT_EE_HANDS_LEFT)
#endif // defined(QUANTUM_PAINTER_ENABLE)

#if defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)
bool ili9341_pressed = false;

// Calibration isn't very precise
static const touch_driver_t ili9341_touch_driver = {
        .width = _ILI9341_WIDTH,
        .height = _ILI9341_HEIGHT,
        .scale_x = 0.07,
        .scale_y = -0.09,
        .offset_x = -26,
        .offset_y = 345,
        .rotation = (ILI9341_ROTATION + 2) % 4,
        .upside_down = false,
        .spi_config = {
            .chip_select_pin = ILI9341_TOUCH_CS_PIN,
            .divisor = TOUCH_SPI_DIV,
            .lsb_first = false,
            .mode = TOUCH_SPI_MODE,
            .irq_pin = NO_PIN,
            .x_cmd = 0xD0,
            .y_cmd = 0x90
        },
    };
touch_device_t ili9341_touch = &ili9341_touch_driver;
#endif // defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)

uint32_t deferred_init(uint32_t trigger_time, void *cb_arg) {
    logging(UNKNOWN, LOG_TRACE, "-- kb init --");

    __attribute__((unused)) bool ret = true;

#if defined (QUANTUM_PAINTER_ENABLE)
    // =======
    // SIPO
    setPinOutput(SIPO_CS_PIN);
    writePinHigh(SIPO_CS_PIN);

    // =======
    // QP
    wait_ms(150); //Let screens draw some power

#    if defined(INIT_EE_HANDS_LEFT)
    il91874 = qp_il91874_make_spi_device(_IL91874_WIDTH, _IL91874_HEIGHT, IL91874_CS_PIN, SCREENS_DC_PIN, IL91874_RST_PIN, SCREENS_SPI_DIV, SCREENS_SPI_MODE, (void *)il91874_buffer);
    ret &= qp_init(il91874, IL91874_ROTATION);
    ret &= qp_power(il91874, true);
#    else // --------------------  Right half --------------------
    ili9163 = qp_ili9163_make_spi_device(_ILI9163_WIDTH, _ILI9163_HEIGHT, ILI9163_CS_PIN, SCREENS_DC_PIN, ILI9163_RST_PIN, SCREENS_SPI_DIV, SCREENS_SPI_MODE);
    ret &= qp_init(ili9163, ILI9163_ROTATION);
    ret &= qp_power(ili9163, true);

    ili9341 = qp_ili9341_make_spi_device(_ILI9341_WIDTH, _ILI9341_HEIGHT, ILI9341_CS_PIN, SCREENS_DC_PIN, ILI9341_RST_PIN, SCREENS_SPI_DIV, SCREENS_SPI_MODE);
    ret &= qp_init(ili9341, ILI9341_ROTATION);
    ret &= qp_power(ili9341, true);

    qp_rect(ili9163, 0, 0, ILI9163_WIDTH, ILI9163_HEIGHT, HSV_BLACK, true);
    qp_rect(ili9341, 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, HSV_BLACK, true);
#    endif
    if (ret) {
        logging(QP, LOG_TRACE, "QP Setup: OK");
    } else {
        logging(QP, LOG_ERROR, "QP Setup: Error");
    }
#endif // defined (QUANTUM_PAINTER_ENABLE)


#if defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)
    ret = touch_spi_init(ili9341_touch);

    if (ret) {
        logging(TOUCH, LOG_TRACE, "Touch Setup: OK");
    } else {
        logging(TOUCH, LOG_ERROR, "Touch Setup: Error");
    }
#endif // defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)

    // =======
    // Call user code
    logging(UNKNOWN, LOG_TRACE, "-- user code --");
    keyboard_post_init_user();

    return 0;
}

void keyboard_post_init_kb(void) {
    defer_exec(INIT_DELAY, deferred_init, NULL);
}

#if defined(ONE_HAND_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)
void screen_one_hand(touch_report_t touch_report) {
    int16_t x = touch_report.x - ILI9341_WIDTH / 2;
    int16_t y = touch_report.y - ILI9341_HEIGHT / 2;

    if (x > 30) {
        one_hand_movement = y > 0 ? DIRECTION_RIGHT : DIRECTION_LEFT;
    } else if (x < -30){
        one_hand_movement = y > 0 ? DIRECTION_UP : DIRECTION_DOWN;
    }
}
#endif // ONE_HAND_ENABLE && TOUCH_SCREEN_ENABLE && INIT_EE_HANDS_RIGHT

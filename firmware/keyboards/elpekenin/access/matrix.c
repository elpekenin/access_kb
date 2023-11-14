// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/* Notes:
 * - Each register has 8 inputs, aka each row has 8 columns
 * - As such, we can use the number of registers as the size of the variable
 *  !! this would not hold for other registers with a different amount of inputs
 */

#include "quantum.h"
#include "custom_spi_master.h"

#if defined(TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)
#    include "touch_driver.h"
#    include "access.h"
#endif // TOUCH_SCREEN_ENABLE && INIT_EE_HANDS_RIGHT

extern uint8_t thisHand, thatHand;

void matrix_init_custom(void) {
    setPinOutput(PISO_CS_PIN);
    writePinHigh(PISO_CS_PIN);
    custom_spi_init(REGISTERS_SPI_DRIVER_ID);
}

static inline bool check_changes(matrix_row_t *current_matrix, matrix_row_t *temp_matrix) {
    thisHand = is_keyboard_left() ? 0 : ROWS_PER_HAND;
    thatHand = ROWS_PER_HAND - thisHand;

    bool changed = memcmp(current_matrix, temp_matrix, N_PISO_REGISTERS) != 0;
    if (changed) {
        memcpy(current_matrix, temp_matrix, N_PISO_REGISTERS);
    }
    return changed;
}

bool matrix_scan_custom(matrix_row_t *current_matrix) {
    matrix_row_t temp_matrix[N_PISO_REGISTERS];

    // Read matrix over SPI
    custom_spi_start(PISO_CS_PIN, false, REGISTERS_SPI_MODE, PISO_SPI_DIV, REGISTERS_SPI_DRIVER_ID);
    custom_spi_receive((uint8_t *)temp_matrix, N_PISO_REGISTERS, REGISTERS_SPI_DRIVER_ID);
    custom_spi_stop(REGISTERS_SPI_DRIVER_ID);

#if defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)
    // IRQ pin is connected to the 1st input of the last shift register
    // invert its value so it reflects whether the screen is pressed
    temp_matrix[4] ^= (1 << 0);

#    if defined(ONE_HAND_MODE_ENABLE)
    // Do nothing until sensor initialised or when screen isn't pressed
    if (ili9341_touch == NULL || ili9341_pressed == false) {
        // Set the selected key's position in the matrix to 0 as screen isn't pressed
        current_matrix[one_hand_row] &= ~(1 << one_hand_col);

        return check_changes(current_matrix, temp_matrix);
    }

    touch_report_t touch_report = get_spi_touch_report(ili9341_touch, false);

    // Convert left-based to center-based coord
    int16_t x = touch_report.x - ILI9341_WIDTH / 2;

    // If screen pressed on the "trigger" area, virtually press selected key
    if (-30 < x && x < 30) {
        current_matrix[one_hand_row] |= 1 << one_hand_col;
    }
#    endif // ONE_HAND_MODE_ENABLE
#endif // defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)

    return check_changes(current_matrix, temp_matrix);
}

#if defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)
void matrix_scan_kb(void) {
    // check here so debounce is already applied
    ili9341_pressed = matrix_is_on(9, 0);
}
#endif // defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)

// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/* Notes:
 * - Each register has 8 inputs, aka each row has 8 columns
 * - As such, we can use the number of registers as the size of the variable
 *  !! this would not hold for other registers with a different amount of inputs
 */

#include <quantum/quantum.h>

#include "elpekenin/logging.h"
#include "elpekenin/spi_custom.h"

#if defined(TOUCH_SCREEN_ENABLE) && IS_RIGHT_HAND
#    include "elpekenin/touch.h"
#endif // TOUCH_SCREEN_ENABLE && INIT_EE_HANDS_RIGHT

void matrix_init_custom(void) {
    gpio_set_pin_output(PISO_CS_PIN);
    gpio_write_pin_high(PISO_CS_PIN);
    spi_custom_init(REGISTERS_SPI_DRIVER_ID);
}


bool matrix_scan_custom(matrix_row_t *current_matrix) {
    matrix_row_t temp_matrix[ROWS_PER_HAND];

    // Read matrix over SPI
    if (!spi_custom_start(PISO_CS_PIN, false, REGISTERS_SPI_MODE, PISO_SPI_DIV, REGISTERS_SPI_DRIVER_ID)) {
        return false;
    }
    spi_custom_receive((uint8_t *)temp_matrix, ROWS_PER_HAND, REGISTERS_SPI_DRIVER_ID);
    spi_custom_stop(REGISTERS_SPI_DRIVER_ID);

    if (!is_keyboard_left()) {
        // IRQ pin is connected to the 1st input of the last shift register
        // invert its value so it reflects whether the screen is pressed
        temp_matrix[4] ^= (1 << 0); // column 0
    }

    bool changed = memcmp(current_matrix, temp_matrix, ROWS_PER_HAND) != 0;
    if (changed) {
        memcpy(current_matrix, temp_matrix, ROWS_PER_HAND);
    }

    return changed;
}

#if defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && IS_RIGHT_HAND
bool is_ili9341_pressed(void) {
    return matrix_is_on(9, 0);
}
#endif

// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/* Notes:
 * - Each register has 8 inputs, aka each row has 8 columns
 * - As such, we can use the number of registers as the size of the variable
 *  !! this would not hold for other registers with a different amount of inputs
 */

#include "quantum.h"

#include "elpekenin/spi_custom.h"

#if defined(TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)
#    include "elpekenin/touch.h"
#endif // TOUCH_SCREEN_ENABLE && INIT_EE_HANDS_RIGHT

extern uint8_t thisHand, thatHand;

void matrix_init_custom(void) {
    setPinOutput(PISO_CS_PIN);
    writePinHigh(PISO_CS_PIN);
    spi_custom_init(REGISTERS_SPI_DRIVER_ID);
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
    spi_custom_start(PISO_CS_PIN, false, REGISTERS_SPI_MODE, PISO_SPI_DIV, REGISTERS_SPI_DRIVER_ID);
    spi_custom_receive((uint8_t *)temp_matrix, N_PISO_REGISTERS, REGISTERS_SPI_DRIVER_ID);
    spi_custom_stop(REGISTERS_SPI_DRIVER_ID);

#if defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)
    // IRQ pin is connected to the 1st input of the last shift register
    // invert its value so it reflects whether the screen is pressed
    temp_matrix[4] ^= (1 << 0);
#endif // defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)

    return check_changes(current_matrix, temp_matrix);
}

#if defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)
void matrix_scan_kb(void) {
    // check here so debounce is already applied
    ili9341_pressed = matrix_is_on(9, 0);
}
#endif // defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)

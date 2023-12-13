// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#if defined(ONE_HAND_ENABLE)
RGB_MATRIX_EFFECT(onehand_mode)

#    if defined(RGB_MATRIX_CUSTOM_EFFECT_IMPLS)
#        include "access.h"
#        define ONE_HAND_LED_INDEX (g_led_config.matrix_co[one_hand_row][one_hand_col])
#        define ONE_HAND_BG_COLOR     0, 0, 0
#        define ONE_HAND_MARKER_COLOR 0, 0, RGB_MATRIX_MAXIMUM_BRIGHTNESS

static void onehand_mode_init(effect_params_t *param) {
    one_hand_col = 0;
    one_hand_row = 0;
    one_hand_movement = DIRECTION_NONE;
    rgb_matrix_set_color_all(ONE_HAND_BG_COLOR);
    rgb_matrix_set_color(ONE_HAND_LED_INDEX, ONE_HAND_MARKER_COLOR);
}

static bool onehand_mode(effect_params_t *params) {
    if (params->init) {
        onehand_mode_init(params);
    }

    // Remove previous marker
    rgb_matrix_set_color(ONE_HAND_LED_INDEX, ONE_HAND_BG_COLOR);

    do {
        switch (one_hand_movement) {
            case DIRECTION_UP:
#if defined(SPLIT_KEYBOARD)
                if (one_hand_row == 0) {
                    one_hand_row = MATRIX_ROWS/2 - 1;
                } else if (one_hand_row == MATRIX_ROWS/2) {
                    one_hand_row = MATRIX_ROWS - 1;
                }
#else
                if (one_hand_row == 0) {
                    one_hand_row = MATRIX_ROWS - 1;
                }
#endif // SPLIT_KEYBOARD
                else {
                    one_hand_row--;
                }
                break;

            case DIRECTION_DOWN:
#if defined(SPLIT_KEYBOARD)
                if (one_hand_row == MATRIX_ROWS/2 - 1) {
                    one_hand_row = 0;
                } else if (one_hand_row == MATRIX_ROWS - 1) {
                    one_hand_row = MATRIX_ROWS/2;
                }
#else
                if (one_hand_row == MATRIX_ROWS - 1) {
                     one_hand_row = 0;
                }
#endif // SPLIT_KEYBOARD
                else {
                    one_hand_row++;
                }
                break;

            case DIRECTION_LEFT:
#if defined(SPLIT_KEYBOARD)
                if (one_hand_col == 0) {
                    one_hand_col = MATRIX_COLS - 1;
                    if (one_hand_row < MATRIX_COLS/2) {
                        one_hand_row += MATRIX_ROWS/2;
                    } else {
                        one_hand_row -= MATRIX_ROWS/2;
                    }
                } else {
                    one_hand_col--;
                }
#else
                if (one_hand_col == 0) {
                    one_hand_col = MATRIX_COLS - 1;
                } else {
                    one_hand_col--;
                }
#endif // SPLIT_KEYBOARD
                break;

            case DIRECTION_RIGHT:
#if defined(SPLIT_KEYBOARD)
                if (one_hand_col == MATRIX_COLS - 1) {
                    one_hand_col = 0;
                    if (one_hand_row < MATRIX_COLS/2) {
                        one_hand_row += MATRIX_ROWS/2;
                    } else {
                        one_hand_row -= MATRIX_ROWS/2;
                    }
                } else {
                    one_hand_col++;
                }
#else
                if (one_hand_col == MATRIX_COLS - 1) {
                    one_hand_col = 0;
                } else {
                    one_hand_col++;
                }
#endif // SPLIT_KEYBOARD
                break;

            case DIRECTION_NONE:
                break;
        }
    // looping ensures we always mark a key as NO_LED should only happen on un-populated points of the matrix
    // most of the times we will only make an iteration, so it shouldn't be slow
    } while (ONE_HAND_LED_INDEX == NO_LED);

    // Clear direction and draw new marker
    one_hand_movement = DIRECTION_NONE;
    rgb_matrix_set_color(ONE_HAND_LED_INDEX, ONE_HAND_MARKER_COLOR);

    return false;
}
#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif // ONE_HAND_ENABLE

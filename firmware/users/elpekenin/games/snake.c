// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "rng.h"
#include "graphics.h"
#include "logging.h"

#include "games/snake.h"

static inline position_t random_position(void) {
    return (position_t) {
        .x = rng_min_max(0, GAME_COLS),
        .y = rng_min_max(0, GAME_ROWS),
    };
}

static inline bool check_collision(position_t pos1, position_t pos2) {
    return pos1.x == pos2.x && pos1.y == pos2.y;
}

static inline void generate_food(game_state_t *state) {
    while (true) {
        bool valid = true;
        position_t new_food = random_position();

        for (uint8_t i = 0; i < state->snake_len; ++i) {
            if (check_collision(new_food, state->snake[i])) {
                valid = false;
            }
        }

        if (valid) {
            state->food = new_food;
            return;
        }
    }
}

static const position_t board_start = {.x = 10, .y = 100};

static inline position_t to_pixels(position_t tile_pos) {
    return (position_t) {
        .x = board_start.x + tile_pos.x * TILE_SIZE,
        .y = board_start.y + tile_pos.y * TILE_SIZE,
    };
}

static inline void draw_game(game_state_t *state) {
    if (!is_keyboard_master()) {
        return;
    }

    // clear canvas
    position_t board_end = to_pixels(
        (position_t) {
            .x = GAME_COLS,
            .y = GAME_ROWS
        }
    );
    qp_rect(
        state->display,
        board_start.x,
        board_start.y,
        board_end.x,
        board_end.y,
        HSV_BLACK,
        true
    );
    // smal outline
        qp_rect(
        state->display,
        board_start.x - 1,
        board_start.y - 1,
        board_end.x + 1,
        board_end.y + 1,
        HSV_WHITE,
        false
    );

    if (!state->playing) {
        const char *text = "Ded, git gut.";
        painter_font_handle_t font = qp_fonts[1];

        int16_t text_w = qp_textwidth(font, text);
        uint16_t board_w = board_end.x - board_start.x;
        uint16_t board_h = board_end.y - board_start.y;
        
        qp_drawtext(
            state->display,
            board_start.x + (board_w - text_w) / 2,
            board_start.y + (board_h - font->line_height) / 2,
            font,
            text
        );
        return;
    }

    // draw snake
    for (uint8_t i = 0; i < state->snake_len; ++i) {
        position_t snake_coords = to_pixels(state->snake[i]);
        qp_rect(
            state->display,
            snake_coords.x,
            snake_coords.y,
            snake_coords.x + TILE_SIZE,
            snake_coords.y + TILE_SIZE,
            HSV_GREEN,
            true
        );
    }

    // draw food
    position_t food_coords = to_pixels(state->food);
    qp_rect(
        state->display,
        food_coords.x,
        food_coords.y,
        food_coords.x + TILE_SIZE,
        food_coords.y + TILE_SIZE,
        HSV_RED,
        true
    );
}

// return value: "snake still alive"
void advance_snake_game(game_state_t *state) {
    if (!state || !state->playing) {
        return;
    }

    // on start, draw game but dont move
    if (state->direction == NO_MOVEMENT) {
        draw_game(state);
        return;
    }

    // compute new position of the head from current position
    position_t new_head = state->snake[0];
    switch (state->direction) {
        case LEFT:
            new_head.x--;
            break;
        
        case RIGHT:
            new_head.x++;
            break;
        
        case TOP:
            new_head.y--;
            break;
        
        case BOTTOM:
            new_head.y++;
            break;

        case NO_MOVEMENT:
            logging(UNKNOWN, LOG_ERROR, "%s unreachable.", __func__);
            return;
    }

    // check for wall
    if (state->death_walls) {
        // works for both directions
        // as we use **u**16, we get overflow on (0--)
        if (new_head.x >= GAME_ROWS || new_head.y >= GAME_COLS) {
            state->playing = false;
            draw_game(state);
            return;
        }
    } else {
        logging(UNKNOWN, LOG_ERROR, "%s not implemented", __func__);
        state->playing = false;
        return;
    }

    // check for running into itself
    // -1 as we "collision" with tail is ok, it would move on next frame
    for (uint8_t i = 0; i < state->snake_len - 1; ++i) {
        if (check_collision(new_head, state->snake[i])) {
            state->playing = false;
            draw_game(state);
            return;
        }
    }

    // shift body parts one position to the right (make room for head)
    memmove(state->snake + 1, state->snake, state->snake_len * sizeof(position_t));
    
    // first item is the new head location
    state->snake[0] = new_head;

    // eat -> len increases
    if (check_collision(new_head, state->food)) {
        state->snake_len++;
        generate_food(state);
    }

    draw_game(state);
}

game_state_t new_snake_game(painter_device_t display) {
    game_state_t state = {
        .display = display,
        .playing = true,
        .direction = NO_MOVEMENT,
        .death_walls = true,
        .snake = {random_position()},
        .snake_len = 1
    };
    generate_food(&state);
    return state;
}

// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "game.h"

// TODO: Support for different games
#include "snake.h"

static game_state_t game;

void set_game_movement(movement_t direction) {
    game.direction = direction;
}

void game_init(painter_device_t display) {
    game = new_snake_game(display);
}

// reset is just init + re-use display
void game_reset(void) {
    game_init(game.display);
}

void game_frame(void) {
    advance_snake_game(&game);
}

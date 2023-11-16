// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "graphics.h"
#include "qp_logging.h"
#include "user_logging.h"

static char                 *qp_log_pointers[LOG_N_LINES];
static deferred_token        qp_log_tokens[LOG_N_LINES];
static bool                  qp_log_redraw;

static char           qp_log[LOG_N_LINES][LOG_N_CHARS + 1];
static log_level_t    qp_log_levels[LOG_N_LINES];
static uint8_t        qp_log_current_col;

void sendchar_qp_hook(uint8_t c) {
    // Setup the arrays on the 1st go
    static bool initialized = false;
    if (!initialized) {
        memset(qp_log, 0, sizeof(qp_log));
        for (uint8_t i = 0; i < LOG_N_LINES; ++i) {
            qp_log_pointers[i] = qp_log[i];
            qp_log_tokens[i]   = INVALID_DEFERRED_TOKEN;
        }
        qp_log_redraw = false;
        initialized = true;
    }

    if (c == '\n') {
        // Add null pointer to current line
        qp_log_pointers[LOG_N_LINES - 1][qp_log_current_col] = 0;

        // Move everything 1 line upwards
        char *temp = qp_log_pointers[0];
        for (uint8_t i = 0; i < LOG_N_LINES - 1; ++i) {
            qp_log_pointers[i] = qp_log_pointers[i + 1];
            qp_log_levels[i]   = qp_log_levels[i + 1];
        }
        qp_log_pointers[LOG_N_LINES - 1] = temp;
        qp_log_levels[LOG_N_LINES - 1]   = NONE;

        // Reset stuff
        qp_log_current_col                                   = 0;
        qp_log_pointers[LOG_N_LINES - 1][qp_log_current_col] = 0;
        qp_log_redraw                                        = true;
    } else if (qp_log_current_col >= LOG_N_CHARS) {
        return;
    } else {
        qp_log_pointers[LOG_N_LINES - 1][qp_log_current_col++] = c;
        qp_log_pointers[LOG_N_LINES - 1][qp_log_current_col]   = 0;
        qp_log_levels[LOG_N_LINES - 1]                         = get_message_level();
        qp_log_redraw                                          = true;
    }
}

static const HSV log_colors[] = {
    [NONE]  = {HSV_WHITE},
    [TRACE] = {0, 0, 100},
    [DEBUG] = {HSV_YELLOW},
    [INFO]  = {HSV_BLUE},
    [WARN]  = {HSV_ORANGE},
    [ERROR] = {HSV_RED},
};
ASSERT_LEVELS(log_colors);

void qp_logging_render(qp_logging_render_args_t args) {
    if (!qp_log_redraw || !args.device) {
        return;
    }

    qp_log_redraw = false;

    // Clear space
    qp_rect(args.device, args.x, args.y, args.screen_w, args.y + LOG_N_LINES * args.font->line_height, HSV_BLACK, true);

    uint16_t y = args.y;
    for (uint8_t i = 0; i < LOG_N_LINES; ++i) {
        bool text_fits = qp_textwidth(args.font, (const char *)qp_log_pointers[i]) < (args.screen_w - args.x);

        if (qp_log_tokens[i] != INVALID_DEFERRED_TOKEN) {
            stop_scrolling_text(qp_log_tokens[i]);
            qp_log_tokens[i] = INVALID_DEFERRED_TOKEN;
        }

        y += args.font->line_height;

        HSV bg = {HSV_BLACK};
        HSV fg = log_colors[qp_log_levels[i]];

        if (text_fits) {
            qp_drawtext_recolor(
                args.device,
                args.x,
                y,
                args.font,
                (const char *)qp_log_pointers[i],
                fg.h, fg.s, fg.v,
                bg.h, bg.s, bg.v
            );
        } else {
            qp_log_tokens[i] = draw_scrolling_text_recolor(
                args.device,
                args.x,
                y,
                args.font,
                (const char *)qp_log_pointers[i],
                args.n_chars,
                args.delay,
                fg.h, fg.s, fg.v,
                bg.h, bg.s, bg.v
            );
        }
    }
}

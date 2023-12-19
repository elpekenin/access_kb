// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "qp.h"
#include "qp_internal.h"

#include "elpekenin/logging.h"
#include "elpekenin/utils/hash_map.h"

#include "generated/qp_resources.h"

#if !defined(QUANTUM_PAINTER_NUM_DISPLAYS)
#    define QUANTUM_PAINTER_NUM_DISPLAYS 1
#endif // QUANTUM_PAINTER_NUM_DISPLAYS

void elpekenin_qp_init(void); // qp_init used by QMK (:


// *** Asset handling ***

void _load_display(painter_device_t display, const char *name);
void _load_font(const uint8_t *font, const char *name);
void _load_image(const uint8_t *img, const char *name);
#define load_display(x) _load_display(x, #x)
#define load_font(x) _load_font(x, #x)
#define load_image(x) _load_image(x, #x)

uint8_t qp_get_num_displays(void);
uint8_t qp_get_num_imgs(void);
uint8_t qp_get_num_fonts(void);

painter_device_t       qp_get_device_by_index(uint8_t index);
painter_font_handle_t  qp_get_font_by_index(uint8_t index);
painter_image_handle_t qp_get_img_by_index(uint8_t index);

painter_device_t       qp_get_device_by_name(const char *name);
painter_font_handle_t  qp_get_font_by_name(const char *name);
painter_image_handle_t qp_get_img_by_name(const char *name);


// *** Show build info ***

void draw_commit(painter_device_t device);
void draw_features(painter_device_t device);


// *** Scrolling text ***

typedef struct scrolling_text_state_t {
    painter_device_t      device;
    uint16_t              x;
    uint16_t              y;
    painter_font_handle_t font;
    char *                str;
    uint8_t               n_chars; // Chars being drawn each time
    uint32_t              delay;
    uint8_t               char_number; // Current positon
    uint8_t               spaces;      // Whitespaces between string repetition
    int16_t               width;       // of the currently-shown string, to remove before rendering
    qp_pixel_t            fg;
    qp_pixel_t            bg;
    deferred_token        defer_token;
} scrolling_text_state_t;

deferred_token draw_scrolling_text(painter_device_t device, uint16_t x, uint16_t y, painter_font_handle_t font, const char *str, uint8_t n_chars, uint32_t delay);
deferred_token draw_scrolling_text_recolor(painter_device_t device, uint16_t x, uint16_t y, painter_font_handle_t font, const char *str, uint8_t n_chars, uint32_t delay, uint8_t hue_fg, uint8_t sat_fg, uint8_t val_fg, uint8_t hue_bg, uint8_t sat_bg, uint8_t val_bg);
void stop_scrolling_text(deferred_token scrolling_token);
void extend_scrolling_text(deferred_token scrolling_token, const char *str);


// *** Callbacks ***

void set_qp_logging_device(painter_device_t device);
void set_uptime_device(painter_device_t device);

#if defined(KEYLOG_ENABLE)
void set_keylog_device(painter_device_t device);
#endif

// exposed for logging/backend/qp.c
typedef struct PACKED {
    uint8_t  n_chars;
    uint32_t delay;
} scrolling_args_t;

typedef struct PACKED {
    painter_device_t device;
    painter_font_handle_t font;
    scrolling_args_t scrolling_args;
    uint16_t x;
    uint16_t y;
} qp_callback_args_t;

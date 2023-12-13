// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "qp.h"
#include "qp_internal.h"

#include "generated/qp_resources.h"

#include "elpekenin/logging.h"
#include "elpekenin/utils/hash_map.h"

#if !defined(QUANTUM_PAINTER_NUM_DISPLAYS)
#    define QUANTUM_PAINTER_NUM_DISPLAYS 1
#endif // QUANTUM_PAINTER_NUM_DISPLAYS

// =======
// Storage
extern painter_device_t qp_devices_pekenin[QUANTUM_PAINTER_NUM_DISPLAYS];
extern painter_font_handle_t qp_fonts[QUANTUM_PAINTER_NUM_FONTS];
extern painter_image_handle_t qp_images[QUANTUM_PAINTER_NUM_IMAGES];

// =======
// Loading
void _load_display(painter_device_t display, const char *name);
void _load_font(const uint8_t *font, const char *name);
void _load_image(const uint8_t *img, const char *name);
#define load_display(x) _load_display(x, #x)
#define load_font(x) _load_font(x, #x)
#define load_image(x) _load_image(x, #x)
void load_qp_resources(void);

uint8_t num_displays(void);
uint8_t num_imgs(void);
uint8_t num_fonts(void);

extern hash_map_t display_map;
extern hash_map_t font_map;
extern hash_map_t img_map;
painter_device_t       get_device(const char *name);
painter_font_handle_t  get_font(const char *name);
painter_image_handle_t get_img(const char *name);

// ======
// States
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

// =======
// Drawing
void draw_commit(painter_device_t device);
void draw_features(painter_device_t device);

deferred_token draw_scrolling_text(painter_device_t device, uint16_t x, uint16_t y, painter_font_handle_t font, const char *str, uint8_t n_chars, uint32_t delay);
deferred_token draw_scrolling_text_recolor(painter_device_t device, uint16_t x, uint16_t y, painter_font_handle_t font, const char *str, uint8_t n_chars, uint32_t delay, uint8_t hue_fg, uint8_t sat_fg, uint8_t val_fg, uint8_t hue_bg, uint8_t sat_bg, uint8_t val_bg);
void stop_scrolling_text(deferred_token scrolling_token);
void extend_scrolling_text(deferred_token scrolling_token, const char *str);

// Target device
extern painter_device_t qp_log_target_device;
void housekeeping_qp(uint32_t now);

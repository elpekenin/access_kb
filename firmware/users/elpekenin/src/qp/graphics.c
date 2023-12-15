// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#if defined(WPM_ENABLE)
#    include "wpm.h"
#endif // defined(WPM_ENABLE)

#include "elpekenin/build_info.h"
#include "elpekenin/logging/backends/qp.h"
#include "elpekenin/qp/graphics.h"
#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/time.h"

#if defined(KEYLOG_ENABLE)
#    include "elpekenin/keylog.h"
#endif // defined(KEYLOG_ENABLE)


// *** Internal variables ***

static painter_device_t       qp_devices_pekenin[QUANTUM_PAINTER_NUM_DISPLAYS] = {0}; // Has to be filled by the user
static painter_font_handle_t  qp_fonts[QUANTUM_PAINTER_NUM_FONTS]   = {0};
static painter_image_handle_t qp_images[QUANTUM_PAINTER_NUM_IMAGES] = {0};

static uint8_t display_counter = 0;
static uint8_t font_counter    = 0;
static uint8_t img_counter     = 0;

static hash_map_t display_map = {0};
static hash_map_t font_map    = {0};
static hash_map_t img_map     = {0};

static deferred_executor_t    scrolling_text_executors[QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS] = {0};
static scrolling_text_state_t scrolling_text_states[QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS] = {0};


static deferred_token     logging_token = INVALID_DEFERRED_TOKEN;
static qp_callback_args_t logging_args  = {
    .device = NULL,
    .font   = NULL,
    // hard-coded for ILI9341 on access
    .x = 160,
    .y = 100,
    .scrolling_args.n_chars = 18,
    .scrolling_args.delay = 500,
};

static deferred_token     uptime_token = INVALID_DEFERRED_TOKEN;
static qp_callback_args_t uptime_args  = {
    .device = NULL,
    .font   = NULL,
    // hard-coded for ILI9341 on access
    .x = 50,
    .y = 55,
};

#if defined(KEYLOG_ENABLE)
static deferred_token     keylog_token = INVALID_DEFERRED_TOKEN;
static qp_callback_args_t keylog_args  = {
    .device = NULL,
    .font   = NULL,
};
#endif


// *** Asset handling ***

void _load_display(painter_device_t display, const char *name) {
    if (display_counter >= QUANTUM_PAINTER_NUM_DISPLAYS) {
        logging(QP, LOG_ERROR, "Loading '%s' failed. OOB", name);
        return;
    }

    qp_devices_pekenin[display_counter] = display;
    display_map.add(&display_map, name, &qp_devices_pekenin[display_counter]);

    logging(QP, LOG_DEBUG, "Loaded '%s' at [%d]", name, display_counter++);
}

void _load_font(const uint8_t *font, const char *name) {
    if (font_counter >= QUANTUM_PAINTER_NUM_FONTS) {
        logging(QP, LOG_ERROR, "Loading '%s' failed. OOB", name);
        return;
    }

    painter_font_handle_t dummy = qp_load_font_mem(font);
    qp_fonts[font_counter] = dummy;
    font_map.add(&font_map, name, &qp_fonts[font_counter]);

    logging(QP, LOG_DEBUG, "Loaded '%s' at [%d]", name, font_counter++);
}

void _load_image(const uint8_t *img, const char *name) {
    if (img_counter >= QUANTUM_PAINTER_NUM_IMAGES) {
        logging(QP, LOG_ERROR, "Loading '%s' failed. OOB", name);
        return;
    }

    painter_image_handle_t dummy = qp_load_image_mem(img);
    qp_images[img_counter] = dummy;
    img_map.add(&img_map, name, &qp_images[img_counter]);

    logging(QP, LOG_DEBUG, "Loaded '%s' at [%d]", name, img_counter++);
}

// getters
uint8_t qp_get_num_displays(void) {
    return display_counter;
}

painter_device_t qp_get_device_by_index(uint8_t index) {
    return qp_devices_pekenin[index];
}

painter_device_t qp_get_device_by_name(const char *name) {
    painter_device_t *p_dev;
    return display_map.get(&display_map, name, (void **)&p_dev) ? *p_dev : NULL;
}


uint8_t qp_get_num_fonts(void) {
    return font_counter;
}

painter_font_handle_t qp_get_font_by_index(uint8_t index) {
    return qp_fonts[index];
}

painter_font_handle_t qp_get_font_by_name(const char *name) {
    painter_font_handle_t *p_font;
    return font_map.get(&font_map, name, (void **)&p_font) ? *p_font : NULL;
}


uint8_t qp_get_num_imgs(void) {
    return img_counter;
}

painter_image_handle_t qp_get_img_by_index(uint8_t index) {
    return qp_images[index];
}

painter_image_handle_t qp_get_img_by_name(const char *name) {
    painter_image_handle_t *p_img;
    return img_map.get(&img_map, name, (void **)&p_img) ? *p_img : NULL;
}


// *** Build info ***

void draw_commit(painter_device_t device) {
    painter_driver_t *    driver     = (painter_driver_t *)device;
    painter_font_handle_t font       = qp_get_font_by_name("font_fira_code");
    uint16_t              width      = driver->panel_width;
    uint16_t              height     = driver->panel_height;
    int16_t               hash_width = qp_textwidth(font, build_info.commit);

    uint16_t real_width  = width;
    uint16_t real_height = height;

    if (driver->rotation == QP_ROTATION_90 || driver->rotation == QP_ROTATION_270) {
        real_width  = height;
        real_height = width;
    }

    uint16_t x = real_width  - hash_width;
    uint16_t y = real_height - font->line_height;

    qp_drawtext_recolor(device, x, y, font, build_info.commit, HSV_RED, HSV_WHITE);
}


// *** Scrolling text ***

static bool render_scrolling_text_state(scrolling_text_state_t *state) {
    logging(SCROLL_TXT, LOG_TRACE, "%s: entry (char #%d)", __func__, (int)state->char_number);

    // prepare string slice
    char *slice = alloca(state->n_chars + 1); // +1 for null terminator
    if (slice == NULL) {
        logging(SCROLL_TXT, LOG_ERROR, "%s: could not allocate", __func__);
        return false;
    }
    memset(slice, 0, state->n_chars + 1);

    uint8_t len = strlen(state->str);
    for (uint8_t i = 0; i < state->n_chars; ++i) {
        uint8_t index = (state->char_number + i);

        // wrap to string length (plus spaces)
        uint8_t wrapped = index % (len + state->spaces);

        // copy a char or add separator whitespaces
        if (wrapped < len) {
            slice[i] = state->str[wrapped];
        } else {
            slice[i] = ' ';
        }
    }


    int16_t width = qp_textwidth(state->font, (const char *)slice);
    // clear previous rendering if needed
    if (state->width > 0) {
        qp_rect(state->device, state->x, state->y, state->x + state->width, state->y + state->font->line_height, HSV_BLACK, true);
    }
    state->width = width;


    // draw it
    bool ret = qp_drawtext_recolor(state->device, state->x, state->y, state->font, (const char *)slice, state->fg.hsv888.h, state->fg.hsv888.s, state->fg.hsv888.v, state->bg.hsv888.h, state->bg.hsv888.s, state->bg.hsv888.v);

    // update position
    if (ret) {
        ++state->char_number;
        if (state->char_number == len + state->spaces) {
            state->char_number = 0;
        }
    }

    if (ret) {
        logging(SCROLL_TXT, LOG_TRACE, "%s: ok", __func__);
    } else {
        logging(SCROLL_TXT, LOG_ERROR, "%s: fail", __func__);
    }
    return ret;
}

static uint32_t scrolling_text_callback(uint32_t trigger_time, void *cb_arg) {
    scrolling_text_state_t *state = (scrolling_text_state_t *)cb_arg;
    bool                    ret = render_scrolling_text_state(state);

    if (!ret) {
        // Setting the device to NULL clears the scrolling slot
        state->device = NULL;
    }
    // If we're successful, keep scrolling -- returning 0 cancels the deferred execution
    return ret ? state->delay : 0;
}

deferred_token draw_scrolling_text(painter_device_t device, uint16_t x, uint16_t y, painter_font_handle_t font, const char *str, uint8_t n_chars, uint32_t delay) {
    return draw_scrolling_text_recolor(device, x, y, font, str, n_chars, delay, 0, 0, 255, 0, 0, 0);
}

deferred_token draw_scrolling_text_recolor(painter_device_t device, uint16_t x, uint16_t y, painter_font_handle_t font, const char *str, uint8_t n_chars, uint32_t delay, uint8_t hue_fg, uint8_t sat_fg, uint8_t val_fg, uint8_t hue_bg, uint8_t sat_bg, uint8_t val_bg) {
    logging(SCROLL_TXT, LOG_TRACE, "%s: entry", __func__);

    scrolling_text_state_t *scrolling_state = NULL;
    for (int i = 0; i < QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS; ++i) {
        if (scrolling_text_states[i].device == NULL) {
            scrolling_state = &scrolling_text_states[i];
            break;
        }
    }

    if (scrolling_state == NULL) {
        logging(SCROLL_TXT, LOG_ERROR, "%s: fail (no free slot)", __func__);
        return INVALID_DEFERRED_TOKEN;
    }

    // make a copy of the string, to prevent issues if the original variable is removed
    // note: input is expected to end in null terminator
    uint8_t len          = strlen(str) + 1; // add one to also allocate/copy the terminator
    scrolling_state->str = malloc(len);
    if (scrolling_state->str == NULL) {
        logging(SCROLL_TXT, LOG_ERROR, "%s: fail (allocation)", __func__);
        return false;
    }
    strcpy(scrolling_state->str, str);

    // Prepare the scrolling state
    scrolling_state->device      = device;
    scrolling_state->x           = x;
    scrolling_state->y           = y;
    scrolling_state->font        = font;
    scrolling_state->n_chars     = n_chars;
    scrolling_state->delay       = delay;
    scrolling_state->char_number = 0;
    scrolling_state->spaces      = 5; // TODO: Receive as argument?
    scrolling_state->fg          = (qp_pixel_t){.hsv888 = {.h = hue_fg, .s = sat_fg, .v = val_fg}};
    scrolling_state->bg          = (qp_pixel_t){.hsv888 = {.h = hue_bg, .s = sat_bg, .v = val_bg}};

    // Draw the first string
    if (!render_scrolling_text_state(scrolling_state)) {
        scrolling_state->device = NULL; // disregard the allocated scroling slot
        logging(SCROLL_TXT, LOG_ERROR, "%s: fail (render 1st step)", __func__);
        return INVALID_DEFERRED_TOKEN;
    }

    // Set up the timer
    scrolling_state->defer_token = defer_exec_advanced(scrolling_text_executors, QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS, delay, scrolling_text_callback, scrolling_state);
    if (scrolling_state->defer_token == INVALID_DEFERRED_TOKEN) {
        scrolling_state->device = NULL; // disregard the allocated scrolling slot
        logging(SCROLL_TXT, LOG_ERROR, "%s: fail (setup executor)", __func__);
        return INVALID_DEFERRED_TOKEN;
    }

    logging(SCROLL_TXT, LOG_TRACE, "%s: ok (deferred token = %d)", __func__, (int)scrolling_state->defer_token);
    return scrolling_state->defer_token;
}

void extend_scrolling_text(deferred_token scrolling_token, const char *str) {
    for (int i = 0; i < QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS; ++i) {
        if (scrolling_text_states[i].defer_token == scrolling_token) {
            uint8_t cur_len = strlen(scrolling_text_states[i].str);
            uint8_t new_len = strlen(str);
            char *  new_pos = realloc(scrolling_text_states[i].str, cur_len + new_len);

            if (new_pos == NULL) {
                logging(SCROLL_TXT, LOG_ERROR, "%s: fail (realloc)", __func__);
                return;
            }
            scrolling_text_states[i].str = new_pos;

            strcat(scrolling_text_states[i].str, str);

            return;
        }
    }
}

void stop_scrolling_text(deferred_token scrolling_token) {
    for (int i = 0; i < QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS; ++i) {
        if (scrolling_text_states[i].defer_token == scrolling_token) {
            cancel_deferred_exec_advanced(scrolling_text_executors, QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS, scrolling_token);
            scrolling_text_state_t *state = &scrolling_text_states[i];

            // Clear screen and de-allocate
            qp_rect(state->device, state->x, state->y, state->x + state->width, state->y + state->font->line_height, HSV_BLACK, true);
            free(scrolling_text_states[i].str);

            // Cleanup the state
            scrolling_text_states[i].device      = NULL;
            scrolling_text_states[i].defer_token = INVALID_DEFERRED_TOKEN;

            return;
        }
    }
}

void scrolling_text_tick(void) {
    static uint32_t last_scrolling_text_exec = 0;
    deferred_exec_advanced_task(scrolling_text_executors, QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS, &last_scrolling_text_exec);
}


// *** Callbacks ***

static uint32_t qp_logging_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;
    qp_logging_backend_render(args); // no-op if nothing to draw
    return 100;
}

static uint32_t qp_uptime_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    uint32_t interval = 1000;  // once per second
 
    if (args->device == NULL) {
        return interval;
    }

    div_t result = div(trigger_time, MS_IN_A_DAY);
    // uint8_t days = result.quot;

    result = div(result.rem, MS_IN_AN_HOUR);
    uint8_t hours = result.quot;

    result = div(result.rem, MS_IN_A_MIN);
    uint8_t minutes = result.quot;

    uint8_t seconds = result.rem / MS_IN_A_SEC;

    char uptime_str[16];
    snprintf(uptime_str, sizeof(uptime_str), "Up|%2dh%2dm%2ds", hours, minutes, seconds);

    qp_drawtext(args->device, args->x, args->y, args->font, uptime_str);

    return interval;
}

#if defined(KEYLOG_ENABLE)
static uint32_t qp_keylog_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;

    uint32_t interval = 100;
    if (args->device == NULL || !is_keylog_dirty()) {
        return interval;
    }

    const char *keylog = get_keylog();
    uint16_t    width  = qp_get_width(args->device);
    uint16_t    height = qp_get_height(args->device);

    int16_t textwidth = qp_textwidth(args->font, keylog);
    qp_rect(
        args->device,
        0,
        height - args->font->line_height,
        width - textwidth,
        height,
        HSV_BLACK,
        true
    );

    // default to white, change it based on WPM (if enabled)
    HSV color = {HSV_WHITE};

#    if defined(WPM_ENABLE)
    uint8_t wpm = get_current_wpm();

    if (wpm > 10) {
        color = (HSV){HSV_RED};
    }
    if (wpm > 30) {
        color = (HSV){HSV_YELLOW};
    }
    if (wpm > 50) {
        color = (HSV){HSV_GREEN};
    }
#    endif // defined(WPM_ENABLE)

    qp_drawtext_recolor(
        args->device,
        width - textwidth,
        height - args->font->line_height,
        args->font,
        keylog,
        color.h, color.s, color.v,
        HSV_BLACK
    );

    return interval;
}
#endif // defined(KEYLOG_ENABLE)


// *** API ***

void elpekenin_qp_init(void) {
    display_map = new_hash_map();
    font_map = new_hash_map();
    img_map = new_hash_map();

    // has to be after the maps, as it uses them
    load_qp_resources();

    painter_font_handle_t fira_code = qp_get_font_by_name("font_fira_code");

    logging_args.font = fira_code;
    uptime_args.font = fira_code;
#if defined(KEYLOG_ENABLE)
    keylog_args.font = fira_code;
#endif
}

void set_qp_logging_device(painter_device_t device) {
    logging_args.device = device;

    // dont want multiple "workers" at the same time
    cancel_deferred_exec(logging_token);
    logging_token = defer_exec(10, qp_logging_callback, &logging_args);
}

void set_uptime_device(painter_device_t device) {
    logging_args.device = device;
    
    // dont want multiple "workers" at the same time
    cancel_deferred_exec(uptime_token);
    uptime_token = defer_exec(10, qp_uptime_callback, &uptime_args);
}

#if defined(KEYLOG_ENABLE)
void set_keylog_device(painter_device_t device) {
    keylog_args.device = device;
    
    // dont want multiple "workers" at the same time
    cancel_deferred_exec(keylog_token);
    keylog_token = defer_exec(10, qp_keylog_callback, &keylog_args);
}
#endif

// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "color.h"

#if defined(WPM_ENABLE)
#    include "wpm.h"
#endif

#include "elpekenin/build_info.h"
#include "elpekenin/rng.h"
#include "elpekenin/logging/backends/qp.h"
#include "elpekenin/qp/graphics.h"
#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/dyn_array.h"
#include "elpekenin/utils/map.h"
#include "elpekenin/utils/memory.h"
#include "elpekenin/utils/string.h"
#include "elpekenin/utils/time.h"

#if defined(KEYLOG_ENABLE)
#    include "elpekenin/keylog.h"
#endif


// *** Macro magic for defer_exec handling ***

#define TASK(__name) \
    static deferred_token     __name##_token = INVALID_DEFERRED_TOKEN; \
    static qp_callback_args_t __name##_args  = {0}; \
    static uint32_t __name##_task_callback(uint32_t trigger_time, void *cb_arg); \
    void set_##__name##_device(painter_device_t device) { \
        __name##_args.device = device; \
        cancel_deferred_exec(__name##_token); \
        __name##_token = defer_exec(10, __name##_task_callback, &__name##_args); \
    } \


// *** Internal variables ***

static new_map(painter_device_t,       device_map);
static new_map(painter_font_handle_t,  font_map);
static new_map(painter_image_handle_t, image_map);

static deferred_executor_t    scrolling_text_executors[QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS] = {0};
static scrolling_text_state_t scrolling_text_states[QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS] = {0};

TASK(logging)
TASK(uptime)
TASK(heap_stats)
#if defined(KEYLOG_ENABLE)
TASK(keylog)
#endif

typedef enum {
    FILLING,
    COPYING,
    DONE,
} anim_t;

typedef struct {
    size_t   used;
    bool     running;
    uint64_t mask;
    anim_t   state;
    char *   curr;
    char *   dest;
} heap_stats_extra_t;
static heap_stats_extra_t heap_stats_extra = {0};


// *** Asset handling ***

void _load_display(painter_device_t display, const char *name) {
    logging(QP, LOG_DEBUG, "'%s' at [%d]", name, array_len(device_map.values));
    map_set(device_map, name, display);
}

void _load_font(const uint8_t *font, const char *name) {
    logging(QP, LOG_DEBUG, "'%s' at [%d]", name, array_len(font_map.values));
    map_set(font_map, name, qp_load_font_mem(font));
}

void _load_image(const uint8_t *img, const char *name) {
    logging(QP, LOG_DEBUG, "'%s' at [%d]", name, array_len(image_map.values));
    map_set(image_map, name, qp_load_image_mem(img));
}

// getters
uint8_t qp_get_num_displays(void) {
    return array_len(device_map.values);
}

painter_device_t qp_get_device_by_index(uint8_t index) {
    if (index >= qp_get_num_displays()) {
        return NULL;
    }

    return device_map.values[index];
}

painter_device_t qp_get_device_by_name(const char *name) {
    bool found;

    painter_device_t ret = map_get(device_map, name, found);
    if (!found) {
        return NULL;
    }

    return ret;
}


uint8_t qp_get_num_fonts(void) {
    return array_len(font_map.values);
}

painter_font_handle_t qp_get_font_by_index(uint8_t index) {
    if (index >= qp_get_num_fonts()) {
        return NULL;
    }

    return font_map.values[index];
}

painter_font_handle_t qp_get_font_by_name(const char *name) {
    bool found;

    painter_font_handle_t ret = map_get(font_map, name, found);
    if (!found) {
        return NULL;
    }

    return ret;
}


uint8_t qp_get_num_imgs(void) {
    return array_len(image_map.values);
}

painter_image_handle_t qp_get_img_by_index(uint8_t index) {
    if (index >= qp_get_num_imgs()) {
        return NULL;
    }

    return image_map.values[index];
}

painter_image_handle_t qp_get_img_by_name(const char *name) {
    bool found;

    painter_image_handle_t ret = map_get(image_map, name, found);
    if (!found) {
        return NULL;
    }

    return ret;
}


// *** Build info ***

void draw_commit(painter_device_t device) {
    painter_driver_t *    driver     = (painter_driver_t *)device;
    painter_font_handle_t font       = qp_get_font_by_name("font_fira_code");
    uint16_t              width      = driver->panel_width;
    uint16_t              height     = driver->panel_height;
    int16_t               hash_width = qp_textwidth(font, get_build_info().commit);

    uint16_t real_width  = width;
    uint16_t real_height = height;

    if (driver->rotation == QP_ROTATION_90 || driver->rotation == QP_ROTATION_270) {
        real_width  = height;
        real_height = width;
    }

    uint16_t x = real_width  - hash_width;
    uint16_t y = real_height - font->line_height;

    qp_drawtext_recolor(device, x, y, font, get_build_info().commit, HSV_RED, HSV_WHITE);
}


// *** Scrolling text ***

static bool render_scrolling_text_state(scrolling_text_state_t *state) {
    logging(SCROLL, LOG_TRACE, "%s: entry (char #%d)", __func__, (int)state->char_number);

    // prepare string slice
    char *slice = alloca(state->n_chars + 1); // +1 for null terminator
    if (slice == NULL) {
        logging(SCROLL, LOG_ERROR, "%s: could not allocate", __func__);
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
        logging(SCROLL, LOG_TRACE, "%s: ok", __func__);
    } else {
        logging(SCROLL, LOG_ERROR, "%s: fail", __func__);
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
    logging(SCROLL, LOG_TRACE, "%s: entry", __func__);

    scrolling_text_state_t *scrolling_state = NULL;
    for (
        scrolling_text_state_t *state = scrolling_text_states;
        state < &scrolling_text_states[QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS];
        ++state
    ) {
        if (state->device == NULL) {
            scrolling_state = state;
            break;
        }
    }

    if (scrolling_state == NULL) {
        logging(SCROLL, LOG_ERROR, "%s: fail (no free slot)", __func__);
        return INVALID_DEFERRED_TOKEN;
    }

    // make a copy of the string, to prevent issues if the original variable is removed
    // note: input is expected to end in null terminator
    uint8_t len          = strlen(str) + 1; // add one to also allocate/copy the terminator
    scrolling_state->str = malloc(len);
    if (scrolling_state->str == NULL) {
        logging(SCROLL, LOG_ERROR, "%s: fail (allocation)", __func__);
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
        logging(SCROLL, LOG_ERROR, "%s: fail (render 1st step)", __func__);
        return INVALID_DEFERRED_TOKEN;
    }

    // Set up the timer
    scrolling_state->defer_token = defer_exec_advanced(scrolling_text_executors, QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS, delay, scrolling_text_callback, scrolling_state);
    if (scrolling_state->defer_token == INVALID_DEFERRED_TOKEN) {
        scrolling_state->device = NULL; // disregard the allocated scrolling slot
        logging(SCROLL, LOG_ERROR, "%s: fail (setup executor)", __func__);
        return INVALID_DEFERRED_TOKEN;
    }

    logging(SCROLL, LOG_TRACE, "%s: ok (deferred token = %d)", __func__, (int)scrolling_state->defer_token);
    return scrolling_state->defer_token;
}

void extend_scrolling_text(deferred_token scrolling_token, const char *str) {
    for (
        scrolling_text_state_t *state = scrolling_text_states;
        state < &scrolling_text_states[QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS];
        ++state
    ) {
        if (state->defer_token == scrolling_token) {
            uint8_t cur_len = strlen(state->str);
            uint8_t new_len = strlen(str);
            char *  new_pos = realloc(state->str, cur_len + new_len);

            if (new_pos == NULL) {
                logging(SCROLL, LOG_ERROR, "%s: fail (realloc)", __func__);
                return;
            }
            state->str = new_pos;

            strcat(state->str, str);

            return;
        }
    }
}

void stop_scrolling_text(deferred_token scrolling_token) {
    for (
        scrolling_text_state_t *state = scrolling_text_states;
        state < &scrolling_text_states[QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS];
        ++state
    ) {
        if (state->defer_token == scrolling_token) {
            cancel_deferred_exec_advanced(scrolling_text_executors, QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS, scrolling_token);

            // Clear screen and de-allocate
            qp_rect(state->device, state->x, state->y, state->x + state->width, state->y + state->font->line_height, HSV_BLACK, true);
            free(state->str);

            // Cleanup the state
            state->device      = NULL;
            state->defer_token = INVALID_DEFERRED_TOKEN;

            return;
        }
    }
}


// *** Callbacks ***

static uint32_t scrolling_text_tick_callback(uint32_t trigger_time, void *cb_arg) {
    static uint32_t last_scrolling_text_exec = 0;
    deferred_exec_advanced_task(scrolling_text_executors, QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS, &last_scrolling_text_exec);
    return 100; // 100ms sounds fast enough for me, text moving at +10 frames/second doesnt sound too readable for me
}

static uint32_t logging_task_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;
    qp_logging_backend_render(args); // no-op if nothing to draw
    return 100;
}

static uint32_t uptime_task_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args = (qp_callback_args_t *)cb_arg;
 
    if (args->device == NULL) {
        return 1000;
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

    return 1000;
}

static uint16_t heap_random(uint16_t max, uint64_t *mask) {
    uint16_t random;

    do { // dont mess already-done char
        random = rng_min_max(0, max);
    } while ((*mask & (1 << random)));

    *mask |= (1 << random);
    return random;
}

static void draw_heap(qp_callback_args_t *args) {
    heap_stats_extra_t *extra = (heap_stats_extra_t *)args->extra;
    qp_drawtext(args->device, args->x, args->y, args->font, extra->curr);
}

static uint32_t heap_animation_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args  = (qp_callback_args_t *)cb_arg;
    heap_stats_extra_t *extra = (heap_stats_extra_t *)args->extra;

    if (
        extra->curr == NULL // first time, just draw it and remember the string
        || extra->state == DONE // strings converged, we are done
    ) {
        extra->running = false;

        WITHOUT_LOGGING(ALLOC, free(extra->curr););
        extra->curr = extra->dest;
        extra->dest = NULL;

        draw_heap(args);
        return 0;
    }

    // actual logic
    size_t len = strlen(extra->dest);

    char chr = '\0';
    do { // dont want a terminator mid-string
        chr = rng_min_max('!', '~');
    } while (chr == '\0');

    // all bits that should be set are set, change state
    uint64_t mask = (1 << (len - 1)) - 1;
    if ((extra->mask & mask) == mask) {
        extra->mask = 0;
        switch (extra->state) {
            case FILLING:
                extra->state = COPYING;
                break;

            case COPYING:
                extra->state = DONE;
                break;

            case DONE:
                break;
        }
    }

    // this is an index, -1 prevents out of bouds str[len]
    uint16_t pos = heap_random(len - 1, &extra->mask);

    switch (extra->state) {
        case FILLING:
            extra->curr[pos] = chr;
            break;

        case COPYING:
            extra->curr[pos] = extra->dest[pos];
            break;

        case DONE:
            break;
    }

    draw_heap(args);
    return 50;
}

static uint32_t heap_stats_task_callback(uint32_t trigger_time, void *cb_arg) {
    qp_callback_args_t *args  = (qp_callback_args_t *)cb_arg;
    heap_stats_extra_t *extra = (heap_stats_extra_t *)args->extra;

    size_t used  = get_used_heap();
    size_t total = get_heap_size();

    // args->extra contains the heap used on last iteration to avoid extra redraws
    // args->extra->running avoids redrawing caused by the dynamic memorry allocations on the animation code
    if (args->device == NULL || extra->used == used || extra->running) {
        return 1000;
    }

    extra->used    = used;
    extra->running = true;

    // generate "used/total" string
    pretty_bytes(used, g_scratch, ARRAY_SIZE(g_scratch));
    strcat(g_scratch, "/");
    size_t len = strlen(g_scratch);
    pretty_bytes(total, g_scratch + len, ARRAY_SIZE(g_scratch) - len);

    // start the animation
    size_t dest_size = strlen(g_scratch);
    if (extra->curr != NULL) {
        extra->curr = realloc(extra->curr, dest_size);
        memset(extra->curr, ' ', dest_size);
    }
    extra->dest  = malloc(dest_size);
    extra->mask  = 0;
    extra->state = FILLING;
    strcpy(extra->dest, g_scratch);

    defer_exec(10, heap_animation_callback, args);

    return 1000;
}

#if defined(KEYLOG_ENABLE)
static uint32_t keylog_task_callback(uint32_t trigger_time, void *cb_arg) {
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
#    endif

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
#endif


// *** API ***

void elpekenin_qp_init(void) {
    map_init(device_map,  2, NULL);
    map_init(  font_map,  2, NULL);
    map_init( image_map, 10, NULL);

    // has to be after the maps, as it uses them
    load_qp_resources();

    painter_font_handle_t fira_code = qp_get_font_by_name("font_fira_code");

    // positions are hard-coded for ILI9341 on access

    logging_args.font = fira_code;
    logging_args.x = 160;
    logging_args.y = 100;
    logging_args.scrolling_args.n_chars = 18;
    logging_args.scrolling_args.delay = 500;

    uptime_args.font = fira_code;
    uptime_args.x = 50;
    uptime_args.y = 55;

    heap_stats_args.font = fira_code;
    heap_stats_args.x = 50;
    heap_stats_args.y = uptime_args.y + fira_code->line_height + 2;
    heap_stats_args.extra = &heap_stats_extra;

#if defined(KEYLOG_ENABLE)
    keylog_args.font = fira_code;
#endif

    defer_exec(10, scrolling_text_tick_callback, NULL);
}

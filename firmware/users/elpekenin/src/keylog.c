// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdlib.h>
#include <string.h>

#include "action_layer.h" // get_highest_layer
#include "action_util.h" // get_mods
#include "host.h" // host_keyboard_led_state

#include "elpekenin/keylog.h"
#include "elpekenin/logging.h"
#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/hash_map.h"

#include "generated/keycode_str.h"


// *** Internal variables ***

#ifndef KEYLOG_SIZE
#    define KEYLOG_SIZE (40)
#endif

char keylog[KEYLOG_SIZE + 1] = " "; // extra space for terminator


// *** Replacements implementation ***

typedef enum {
    NO_MODS,
    SHIFT,
    AL_GR,
    // TODO: implement more when needed
    __N_MODS__,
} active_mods_t;

typedef struct PACKED {
    const char *strings[__N_MODS__];
} replacements_t;

static hash_map_t replacements_map;

static inline replacements_t *find_replacements(const char *find) {
    replacements_t *p_replacements;

    // disable hash logging momentarily, as a lot of strings wont be in the replacements map
    log_level_t old_level = get_level_for(HASH);
    set_level_for(HASH, LOG_NONE);
    replacements_map.get(&replacements_map, find, (void **)&p_replacements);
    set_level_for(HASH, old_level);

    return p_replacements;
}

static inline bool add_replacement(const char *find, const char *replace, active_mods_t mods) {
    // try and find existing entry
    replacements_t *replacement = find_replacements(find);

    // if not found, create it
    // ... on heap because hashmap
    if (LIKELY(replacement == NULL)) { // most keycodes have a single replacement, wont exist yet when adding it
        replacement = calloc(1, sizeof(replacements_t));  // ensure we have zeros

        if (UNLIKELY(replacement == NULL)) { // we should have spare heap
            logging(UNKNOWN, LOG_ERROR, "%s: fail (allocation)", __func__);
            return false;
        }

        // couldnt add -> dealloc
        if (UNLIKELY(!replacements_map.add(&replacements_map, find, replacement))) { // lets hope there's no collisions
            free(replacement);
            return false;
        }
    }

    // add the new replacement
    replacement->strings[mods] = replace;

    return true;
}

static void init_replacements(void) {
    replacements_map = new_hash_map();

    // *** Mod-independent ***
    add_replacement("SPC", " ", NO_MODS);
    // arrow (ish)
    add_replacement("UPPR",  "▲", NO_MODS);
    add_replacement("LOWR",  "▼", NO_MODS);
    add_replacement("TAB",   "⇥", NO_MODS);
    add_replacement("BSPC",  "⇤", NO_MODS);
    add_replacement("CAPS",  "↕", NO_MODS);
    add_replacement("ENT",   "↲", NO_MODS);
    add_replacement("UP",    "↑", NO_MODS);
    add_replacement("DOWN",  "↓", NO_MODS);
    add_replacement("RIGHT", "→", NO_MODS);
    add_replacement("LEFT",  "←", NO_MODS);
    // various symbols
    add_replacement("PLUS", "+",  NO_MODS);
    add_replacement("MINS", "-",  NO_MODS);
    add_replacement("NTIL", "´",  NO_MODS);
    add_replacement("QUOT", "'",  NO_MODS);
    add_replacement("GRV",  "`",  NO_MODS);
    add_replacement("COMM", ",",  NO_MODS);
    add_replacement("DOT",  ".",  NO_MODS);
    add_replacement("BSLS", "\\", NO_MODS);
    add_replacement("HASH", "#",  NO_MODS);
    add_replacement("AT",   "@",  NO_MODS);
    add_replacement("PIPE", "|",  NO_MODS);
    add_replacement("TILD", "~",  NO_MODS);
    add_replacement("LBRC", "[",  NO_MODS);
    add_replacement("RBRC", "]",  NO_MODS);
    add_replacement("LCBR", "{",  NO_MODS);
    add_replacement("RCBR", "}",  NO_MODS);
    add_replacement("VOLU", "♪",  NO_MODS);
    // shorter aliases
    add_replacement("DB_TOGG", "DBG", NO_MODS);
    add_replacement("XXXXXXX", "XX",  NO_MODS);
    add_replacement("_______", "__",  NO_MODS);

    // *** Mods ***
    // shift
    add_replacement("1", "!",  SHIFT);
    add_replacement("2", "\"", SHIFT);
    add_replacement("3", "·",  SHIFT);
    add_replacement("4", "$",  SHIFT);
    add_replacement("5", "%",  SHIFT);
    add_replacement("6", "&",  SHIFT);
    add_replacement("7", "/",  SHIFT);
    add_replacement("8", "(",  SHIFT);
    add_replacement("9", ")",  SHIFT);
    add_replacement("0", "=",  SHIFT);
    add_replacement("+", "*",  SHIFT);
    add_replacement("'", "?",  SHIFT);
    add_replacement("`", "^",  SHIFT);
    add_replacement(".", ":",  SHIFT);
    add_replacement(",", ";",  SHIFT);
    add_replacement("-", "_",  SHIFT);

    add_replacement("1", "|", AL_GR);
    add_replacement("2", "@", AL_GR);
    add_replacement("3", "#", AL_GR);
    add_replacement("4", "~", AL_GR);
}


// *** Formatting helpers ***

static void skip_prefix(const char **str) {
    char *prefixes[] = { "KC_", "RGB_", "QK_", "ES_", "TD_", "TL_" };

    for (uint8_t i = 0; i < ARRAY_SIZE(prefixes); ++i) {
        char *  prefix = prefixes[i];
        uint8_t len    = strlen(prefix);

        if (strncmp(prefix, *str, len) == 0) {
            *str += len;
            return;
        }
    }
}

static void maybe_symbol(const char **str) {
    replacements_t *p_replacements = find_replacements(*str);

    if (LIKELY(p_replacements == NULL)) { // most keycodes dont have replacements
        return;
    }

    const char *target;
    switch (get_mods()) {
        case 0:
            target = p_replacements->strings[NO_MODS];
            break;

        case MOD_BIT(KC_LSFT):
        case MOD_BIT(KC_RSFT):
            target = p_replacements->strings[SHIFT];
            break;

        case MOD_BIT(KC_ALGR):
            target = p_replacements->strings[AL_GR];
            break;

        default:
            // nothing to be done here
            return;
    }

    // we may get here with a combinatin with no replacement, eg shift+arrows
    // dont want to assign str to NULL
    if (target != NULL) {
        *str = target;
    }
}

// convert to lowercase based on shift/caps
// overengineered so it can also work on strings and whatnot on future
static void apply_casing(const char **str) {
    // not a single char
    if (strlen(*str) > 1) {
        return;
    }

    // not a letter
    if (**str < 'A' || **str > 'Z') {
        return;
    }

    uint8_t mods  = get_mods();
    bool    shift = mods & MOD_MASK_SHIFT;
    bool    caps  = host_keyboard_led_state().caps_lock;

    // if writing uppercase, string already is, just return
    if (shift ^ caps) {
        return;
    }

    char *lowercase_letters[] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z" };

    *str = lowercase_letters[**str - 'A'];
}


// *** Updating the log ***

static bool is_utf8_continuation(char c) {
    return (c & 0xC0) == 0x80;
}

UNUSED static bool is_utf8(char c) {
    return (c & 0xF0);
}

static void keylog_clear(void) {
    // spaces (not 0) so `qp_drawtext` actually renders something
    memset(keylog, ' ', KEYLOG_SIZE);
    keylog[KEYLOG_SIZE] = '\0';
}

static void _keylog_shift_right_byte(void) {
    for (uint8_t i = KEYLOG_SIZE - 1; i > 0; --i) {
        keylog[i] = keylog[i - 1];
    }
    keylog[0] = ' ';
}

static void keylog_shift_right(void) {
    // pop all utf-continuation bytes
    while (is_utf8_continuation(keylog[KEYLOG_SIZE - 1])) {
       _keylog_shift_right_byte();
    }

    // this is either an ascii char or the heading byte of utf
    _keylog_shift_right_byte();
}

static void keylog_shift_left(uint8_t len) {
    memmove(keylog, keylog + len, KEYLOG_SIZE - len);

    uint8_t counter = 0;
    while (is_utf8_continuation(keylog[0])) {
        memmove(keylog, keylog + 1, KEYLOG_SIZE - 1);
        ++counter;
    }

    // pad buffer to the right, to align after a utf8 symbol is deleted
    memmove(keylog + counter, keylog, KEYLOG_SIZE - counter);
    memset(keylog, ' ', counter);
}

static void keylog_append(const char *str) {
    uint8_t len = strlen(str);

    keylog_shift_left(len);
    for (uint8_t i = 0; i < len; ++i) {
        keylog[KEYLOG_SIZE - len + i] = str[i];
    }
}


// *** API ***

void keycode_repr(const char **str) {
    skip_prefix(str);
    maybe_symbol(str);
}

void keylog_process(uint16_t keycode, keyrecord_t *record) {
    // initial setup
    static bool keylog_init = false;
    if (!keylog_init) {
        keylog_init = true;

        keylog_clear();
        init_replacements();
    }

    // nothing on release (for now)
    if (!record->event.pressed) {
        return;
    }

    // dont want to show some keycodes
    if ((IS_QK_LAYER_TAP(keycode) && !record->tap.count)
        || keycode >= QK_USER  // dont want my custom keycodes on keylog
        || IS_RGB_KEYCODE(keycode)
        || IS_QK_LAYER_MOD(keycode)
        || IS_QK_MOMENTARY(keycode)
        || IS_QK_DEF_LAYER(keycode)
        || IS_MODIFIER_KEYCODE(keycode)
       )
    {
        return;
    }

    // get the string representation of the pressed key
    uint8_t     layer_num = get_highest_layer(layer_state);
    uint8_t     row       = record->event.key.row;
    uint8_t     column    = record->event.key.col;
    const char *str       = get_keycode_str_at(layer_num, row, column);

    uint8_t mods = get_mods();
    bool    ctrl = mods & MOD_MASK_CTRL;

    // delete from tail
    if (strstr(str, "BSPC") != NULL) {
        // ctrl + backspace clears whole log
        if (ctrl) {
            keylog_clear();
        }
        // backspace = remove last char
        else {
            keylog_shift_right();
        }
        return;
    }

    // convert string into symbols
    keycode_repr((const char **)&str);

    // casing is separate so that drawing keycodes on screen is always uppercase
    apply_casing((const char **)&str);

    keylog_append(str);
}

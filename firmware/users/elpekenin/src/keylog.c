// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>

#include "action_layer.h" // get_highest_layer
#include "action_util.h" // get_mods
#include "host.h" // host_keyboard_led_state
#include "debug.h" // debug_config

#include "elpekenin/keylog.h"
#include "elpekenin/logging.h"
#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/init.h"
#include "elpekenin/utils/map.h"
#include "elpekenin/utils/shortcuts.h"
#include "elpekenin/utils/string.h"

#include "generated/keycode_str.h"


// *** Internal variables ***

#ifndef KEYLOG_SIZE
#    define KEYLOG_SIZE (40)
#endif

static bool keylog_dirty = true;
static char keylog[KEYLOG_SIZE + 1] = " "; // extra space for terminator


// *** Replacements implementation ***

typedef enum {
    NO_MODS,
    SHIFT,
    AL_GR,
    // TODO: implement more when needed
    __N_MODS__,
} active_mods_t;

typedef struct PACKED {
    const char *find;
    const char *strings[__N_MODS__];
} replacement_t;

static new_map(replacement_t *, replacements_map);

// TODO: Dont duplicate data in ROM and RAM
static const replacement_t replacements[] = {
    {.find="0",       .strings={                 [SHIFT]="="              }},
    {.find="1",       .strings={                 [SHIFT]="!",  [AL_GR]="|"}},
    {.find="2",       .strings={                 [SHIFT]="\"", [AL_GR]="@"}},
    {.find="3",       .strings={                               [AL_GR]="#"}}, // · breaks keylog
    {.find="4",       .strings={                 [SHIFT]="$",  [AL_GR]="~"}},
    {.find="5",       .strings={                 [SHIFT]="%"              }},
    {.find="6",       .strings={                 [SHIFT]="&"              }},
    {.find="7",       .strings={                 [SHIFT]="/"              }},
    {.find="8",       .strings={                 [SHIFT]="("              }},
    {.find="9",       .strings={                 [SHIFT]=")"              }},
    {.find="_______", .strings={[NO_MODS]="__"                            }},
    {.find="AT",      .strings={[NO_MODS]="@"                             }},
    {.find="BSLS",    .strings={[NO_MODS]="\\"                            }},
    {.find="BSPC",    .strings={[NO_MODS]="⇤"                             }},
    {.find="CAPS",    .strings={[NO_MODS]="↕"                             }},
    {.find="COMM",    .strings={[NO_MODS]=",",   [SHIFT]=";"              }},
    {.find="DB_TOGG", .strings={[NO_MODS]="DBG"                           }},
    {.find="DOT",     .strings={[NO_MODS]=".",   [SHIFT]=":"              }},
    {.find="DOWN",    .strings={[NO_MODS]="↓"                             }},
    {.find="ENT",     .strings={[NO_MODS]="↲"                             }},
    {.find="GRV",     .strings={[NO_MODS]="`",   [SHIFT]="^"              }},
    {.find="HASH",    .strings={[NO_MODS]="#"                             }},
    {.find="LBRC",    .strings={[NO_MODS]="["                             }},
    {.find="LCBR",    .strings={[NO_MODS]="{"                             }},
    {.find="LEFT",    .strings={[NO_MODS]="←"                             }},
    {.find="LOWR",    .strings={[NO_MODS]="▼"                             }},
    {.find="MINS",    .strings={[NO_MODS]="-",   [SHIFT]="_"              }},
    {.find="NTIL",    .strings={[NO_MODS]="´",                            }},
    {.find="R_SPC",   .strings={[NO_MODS]=" "                             }},
    {.find="RBRC",    .strings={[NO_MODS]="]"                             }},
    {.find="RCBR",    .strings={[NO_MODS]="}"                             }},
    {.find="RIGHT",   .strings={[NO_MODS]="→"                             }},
    {.find="PLUS",    .strings={[NO_MODS]="+",   [SHIFT]="*"              }},
    {.find="PIPE",    .strings={[NO_MODS]="|"                             }},
    {.find="QUOT",    .strings={[NO_MODS]="'",   [SHIFT]="?"              }},
    {.find="SPC",     .strings={[NO_MODS]=" "                             }},
    {.find="TAB",     .strings={[NO_MODS]="⇥"                             }},
    {.find="TILD",    .strings={[NO_MODS]="~"                             }},
    {.find="UP",      .strings={[NO_MODS]="↑"                             }},
    {.find="UPPR",    .strings={[NO_MODS]="▲"                             }},
    {.find="VOLU",    .strings={[NO_MODS]="♪",   [SHIFT]="♪"              }},
};

static void replacements_init(void) {
    // store pointers to the replacements, as they are statically allocated and wontget a freeafteruse
    map_init(replacements_map, ARRAY_SIZE(replacements), NULL);

    // add replacements to the map
    for (
        replacement_t *replacement = (replacement_t *)replacements;
        replacement < &replacements[ARRAY_SIZE(replacements)];
        ++replacement
    ) {
        map_set(replacements_map, replacement->find, replacement);
    }
}
PEKE_INIT(replacements_init, 100);


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
    bool found;
    replacement_t *replacements;

    // disable hash logging momentarily, as a lot of strings wont be in the replacements map
    WITHOUT_LOGGING(
        MAP,
        replacements = map_get(replacements_map, *str, found);
    );

    if (LIKELY(found == false)) {
        return;
    }

    const char *target;
    switch (get_mods()) {
        case 0:
            target = replacements->strings[NO_MODS];
            break;

        case MOD_BIT(KC_LSFT):
        case MOD_BIT(KC_RSFT):
            target = replacements->strings[SHIFT];
            break;

        case MOD_BIT(KC_ALGR):
            target = replacements->strings[AL_GR];
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

bool is_keylog_dirty(void) {
    return keylog_dirty;
}

const char *get_keylog(void) {
    // after code gets gets a "view", reset dirty-ness
    keylog_dirty = false;
    return keylog;
}

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

    keylog_dirty = true;

    const char *str = get_keycode_name(keycode);

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

    // unknown keycode, quit
    if (UNLIKELY(str == NULL)) {
        return;
    }

    // convert string into symbols
    keycode_repr((const char **)&str);

    // casing is separate so that drawing keycodes on screen is always uppercase
    apply_casing((const char **)&str);

    keylog_append(str);
}

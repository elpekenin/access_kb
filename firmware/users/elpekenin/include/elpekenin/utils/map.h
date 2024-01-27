// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/logging.h"
#include "elpekenin/utils/dyn_array.h"
#include "elpekenin/utils/shortcuts.h"

// a map is just a convenience wrapper for two dynamic arrays

#define new_map(__type, __name) \
    struct { \
        const char **keys; \
        __type      *values; \
    } __name

#define map_init(__map, __size, __allocator) \
    __map.keys   = new_array(         const char *, __size, __allocator); \
    __map.values = new_array(typeof(*__map.values), __size, __allocator)

// can not overwrite
// does not handle errors if array_append fails
// which should be very unlikely (no space left to allocate)
#define map_set(__map, __key, __value) \
    do { \
        /* if key already used, do nothing */ \
        bool found; \
        WITHOUT_LOGGING(MAP, map_get(__map, __key, found);); \
        if (!found) { \
            array_append(__map.keys, __key); \
            array_append(__map.values, __value); \
        } \
    } while (0)

#define map_get(__map, __key, __found) ({ \
    __found = false; \
    size_t i; \
    for (i = 0; i < array_len(__map.keys); ++i) { \
        if (strcmp(__map.keys[i], __key) == 0) { \
            __found = true; \
            logging(MAP, LOG_TRACE, "Read '%s'", __key); \
            break; /* without this, we get one off (next ++) */ \
        } \
    } \
    \
    if (!__found) { \
        logging(MAP, LOG_ERROR, "Key '%s' not found", __key); \
    } \
    \
    __map.values[i]; \
})

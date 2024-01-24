// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// TODO: Error handling on calls to dyn_array API

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "elpekenin/logging.h"
#include "elpekenin/utils/dyn_array.h"
#include "elpekenin/utils/map.h"
#include "elpekenin/utils/memory.h"


bool map_set(map_t map, const char *key, const void *value) {
    header_t *val_header = get_header(map.values);

    for (size_t i = 0; i < array_len(map.keys); ++i) {
        if (strcmp(map.keys[i], key) == 0) {
            size_t offset = val_header->item_size * i;
            memcpy(map.values + offset, value, val_header->item_size);

            logging(MAP, LOG_TRACE, "%s '%s'", "Overwrote", key);
            return true;
        }
    }

    // store a copy of the string
    header_t *key_header = get_header(map.keys);
    char *cpy = key_header->allocator->malloc(strlen(key));
    strcpy(cpy, key);

    bool _; // silence warn-unused. error handling todo
    _ = array_append((void **)&map.keys,   &cpy);
    _ = array_append((void **)&map.values, value);
    (void)_;

    logging(MAP, LOG_TRACE, "%s '%s'", "Wrote", key);
    return true;
}

bool map_get(map_t map, const char *key, void *value) {
    header_t *val_header = get_header(map.values);

    for (size_t i = 0; i < array_len(map.keys); ++i) {
        if (strcmp(map.keys[i], key) == 0) {
            size_t offset = val_header->item_size * i;
            memcpy(value, map.values + offset, val_header->item_size);

            logging(MAP, LOG_TRACE, "Read '%s'", key);
            return true;
        }
    }

    logging(MAP, LOG_ERROR, "Key '%s' not found", key);
    return false;
}

map_t _new_map(size_t item_size, allocator_t *allocator) {
    return (map_t) {
        .keys   = new_array(const char *, allocator),
        .values = _new_array(item_size, allocator),
    };
}
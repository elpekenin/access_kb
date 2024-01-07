// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// TODO?: store strings and `strcmp` instead of storing hashes

#pragma once

#include "elpekenin/utils/compiler.h"

/*
 * Usage:
 *    SET
 *    >>> uint32_t value = 42;
 *    >>> map.set(&map, "key", &value)
 *
 *    GET - Option 1
 *    >>> uint32_t *p_value;
 *    >>> map.get(&map, "key", (const void **)&p_value);
 *    >>> if (p_value != NULL) {
 *    >>>     uint32_t value = *p_value;
 *    >>> }
 *
 *    GET - Option 2
 *    >>> uint32_t *p_value;
 *    >>> if (map.get(&map, "key", (const void **)&p_value)) {
 *    >>>     uint32_t value = *p_value;
 *    >>> }
 *
 *    NOTE Updating items is not supported.
 */

typedef struct map_t map_t;

typedef bool (*add_fn)(map_t *self, const char *key, const void *value);
typedef bool (*get_fn)(map_t *self, const char *key, const void **value);

typedef struct PACKED {
    const char *key;
    const void *value;
} item_t;

struct PACKED map_t {
    size_t  n_items;
    item_t *items; // dynamically allocated array
    add_fn  add;
    get_fn  get;
};

// NOTE: should be fairly bigger than the amount of items actually stored, to prevent colissions
//       still better than blindly allocating a fixed-size of entries
map_t new_map(size_t n_items);

size_t get_total_map_items(void);
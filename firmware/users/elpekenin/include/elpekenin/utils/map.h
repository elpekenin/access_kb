// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/utils/compiler.h"

/*
 * Usage:
 *    SET
 *    >>> uint32_t value = 42;
 *    >>> set(&map, "key", &value)
 *
 *    GET - Option 1
 *    >>> uint32_t *p_value;
 *    >>> get(&map, "key", (const void **)&p_value);
 *    >>> if (p_value != NULL) {
 *    >>>     uint32_t value = *p_value;
 *    >>> }
 *
 *    GET - Option 2
 *    >>> uint32_t *p_value;
 *    >>> if (get(&map, "key", (const void **)&p_value)) {
 *    >>>     uint32_t value = *p_value;
 *    >>> }
 */

typedef struct map_t map_t;

NON_NULL(1) NON_NULL(2) NON_NULL(3) READ_ONLY(2) READ_ONLY(3) bool set(map_t *self, const char *key, const void *value);
NON_NULL(1) NON_NULL(2) NON_NULL(3) READ_ONLY(1) READ_ONLY(2) WRITE_ONLY(3) bool get(map_t *self, const char *key, const void **value);

typedef struct PACKED {
    const char *key;
    const void *value;
} item_t;

struct PACKED map_t {
    size_t  n_items;
    item_t *items; // dynamically allocated array
};

map_t new_map(size_t n_items);

PURE size_t get_total_map_items(void);
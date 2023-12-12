// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "utils/compiler.h"

/* This implementation of hash map is intended for string->anything mappings
 *
 * Mapping to any value is achieved by storing a pointer to the value. As such:
 *    - Value must be in heap, not stack, so the pointer is still valid
 *    - "user" code has to cast to the appropriate type(s)
 *
 * Limitations:
 *    - Write-only: Once a hash is set, it's value can't be re-assigned
 *    - No collisions: Can't have two items with same hash
 *
 * Usage:
 *    >>> uint32_t value = 42;
 *    >>> map.set(&map, key, &value)
 *
 *    >>> uint32_t *p_value;
 *    >>> map.get(&map, key, (void **)&p_value);
 *    >>> uint32_t value = *p_value;
 */

typedef struct hash_map_t hash_map_t;

typedef bool (*add_fn)(hash_map_t *self, const char *key, const void *value);
typedef bool (*get_fn)(hash_map_t *self, const char *key, void **value);

typedef uint8_t hash_t;

struct PACKED hash_map_t {
    void *addrs[((hash_t)(~0))];
    add_fn add;
    get_fn get;
};

hash_map_t new_hash_map(void);
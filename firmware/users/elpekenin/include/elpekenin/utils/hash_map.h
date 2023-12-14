// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/utils/compiler.h"

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
 *    SET
 *    >>> uint32_t value = 42;
 *    >>> map.set(&map, key, &value)
 *
 *    GET - Option 1
 *    >>> uint32_t *p_value;
 *    >>> map.get(&map, key, (void **)&p_value);
 *    >>> if (p_value != NULL) {
 *    >>>     uint32_t value = *p_value;
 *    >>> }
 *
 *    GET - Option 2
 *    >>> uint32_t *p_value;
 *    >>> if (map.get(&map, key, (void **)&p_value)) {
 *    >>>     uint32_t value = *p_value;
 *    >>> }
 */

typedef struct hash_map_t hash_map_t;

typedef bool (*add_fn)(hash_map_t *self, const char *key, const void *value);
typedef bool (*get_fn)(hash_map_t *self, const char *key, void **value);

typedef uint16_t hash_t;

#ifndef HASH_MAP_SIZE
#    define HASH_MAP_SIZE (300)
#endif

struct PACKED hash_map_t {
    void *addrs[HASH_MAP_SIZE];
    add_fn add;
    get_fn get;
};

hash_map_t new_hash_map(void);
// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/utils/allocator.h"
#include "elpekenin/utils/compiler.h"

/*
 * Usage:
 *    CREATE
 *    >>> map_t map = new_map(uint32_t, &allocator); // NULL will use stdlib allocating mechanism
 *
 *    SET (add or overwrite)
 *    >>> uint32_t value = 42;
 *    >>> map_set(&map, "key", &value);
 *
 *    GET
 *    >>> uint32_t value;
 *    >>> bool found = map_get(map, "key", &value);
 *    >>> if (!found) {
 *    >>>     // error handling
 *    >>> }
 *
 * Notes:
 *    - `key` can be a str on the stack, it gets copied ("safer" but uses more memory)
 *    - Since C has no runtime type information, everything is computed in terms of sizeof(type)
 *      Thus, there can't be any check like "created as u32 but reading as u64", beware the types you use.
 */

// a map is just a convenience wrapper for two dynamic arrays
typedef struct {
    const char **keys;
    void *       values;
} map_t;

#define new_map(__type, __allocator) _new_map(sizeof(__type), __allocator)
READ_ONLY(2) map_t _new_map(size_t item_size, allocator_t *allocator);

READ_ONLY(2) READ_ONLY(3) bool map_set(map_t map, const char *key, const void *value);
READ_ONLY(2) WRITE_ONLY(3) bool map_get(map_t map, const char *key, void *value);

// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/utils/allocator.h"
#include "elpekenin/utils/compiler.h"

typedef struct PACKED {
    allocator_t *allocator;
    size_t       capacity;  // allocated slots
    size_t       length;    // filled slots
    size_t       item_size;
} header_t;

#define get_header(array) (((header_t *)(array)) - 1)
#define array_len(array) ((get_header(array))->length)

#define new_array(__type, __allocator) (__type *)_new_array(sizeof(__type), __allocator)
READ_ONLY(2) void *_new_array(size_t item_size, allocator_t *allocator);

READ_ONLY(2) WARN_UNUSED bool array_append(void **array, const void *value);
WARN_UNUSED bool array_pop(void *array, size_t n_items);

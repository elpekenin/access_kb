// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/logging.h"
#include "elpekenin/utils/dyn_array.h"

void *_new_array(size_t item_size, size_t initial_size, allocator_t *allocator) {
    if (LIKELY(allocator == NULL)) {
        allocator = &default_allocator;
    }

    size_t items_size = item_size * initial_size;
    size_t total_size = sizeof(header_t) + items_size;

    header_t *ptr = allocator->malloc(total_size);
    if (UNLIKELY(ptr == NULL)) {
        logging(ALLOC, LOG_ERROR, "%s", __func__);
        return NULL;
    }

    ptr->allocator = allocator;
    ptr->capacity  = initial_size;
    ptr->length    = 0;
    ptr->item_size = item_size;

    return ptr + 1;
}

header_t *get_header(void *array) {
    if (UNLIKELY(array == NULL)) {
        return NULL;
    }

    return (header_t *)array - 1;
}

size_t array_len(void *array) {
    header_t *header = get_header(array);

    if (UNLIKELY(header == NULL)) {
        return 0;
    }

    return header->length;
}

bool expand_if_needed(void **array) {
    if (UNLIKELY(*array == NULL)) {
        return false;
    }

    header_t *header = get_header(*array);

    // allocate more space if needed
    if (UNLIKELY(header->length == header->capacity)) {
        size_t current_size = header->item_size * header->capacity;
        size_t total_size    = sizeof(header_t) + current_size * 2;

        header = header->allocator->realloc(header, total_size);
        if (UNLIKELY(header == false)) {
            logging(ALLOC, LOG_ERROR, "%s failed", __func__);
            return false;
        }

        *array = header + 1;
        header->capacity *= 2;
    }

    return true;
}


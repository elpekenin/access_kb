// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/logging.h"
#include "elpekenin/utils/dyn_array.h"

void *_new_array(size_t item_size, allocator_t *allocator) {
    size_t capacity = 8;

    if (UNLIKELY(allocator == NULL)) {
        allocator = &default_allocator;
    }

    size_t items_size = item_size * capacity;
    size_t total_size = sizeof(header_t) + items_size;

    header_t *ptr = allocator->malloc(total_size);
    if (UNLIKELY(ptr == NULL)) {
        logging(ALLOC, LOG_ERROR, "%s", __func__);
        return NULL;
    }

    ptr->allocator = allocator;
    ptr->capacity  = capacity;
    ptr->length    = 0;
    ptr->item_size = item_size;

    return ptr + 1;
}

bool array_append(void **array, const void *value) {
    header_t *header = get_header(*array);

    // allocate more space if needed
    if (header->length == header->capacity) {
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

    size_t offset = header->item_size * header->length;
    memcpy((*array) + offset, value, header->item_size);
    header->length++;

    return true;
}

bool array_pop(void *array, size_t n_items) {
    header_t *header = get_header(array);

    if (UNLIKELY(header->length < n_items)) {
        logging(ALLOC, LOG_ERROR, "Not that many items (%d)", n_items);
        return false;
    }

    header->length -= n_items;
    return true;
}

// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/logging.h"
#include "elpekenin/utils/allocator.h"
#include "elpekenin/utils/dyn_array.h"

void *_new_array(size_t item_size, size_t initial_size, allocator_t *allocator) {
    if (UNLIKELY(allocator == NULL)) {
        allocator = get_default_allocator();
    }

    size_t items_size = item_size * initial_size;
    size_t total_size = sizeof(header_t) + items_size;

    header_t *ptr = malloc_with(allocator, total_size);
    // TODO?: Add error handling for malloc failing
    //        Shouldn't happen as we always call this very early

    ptr->allocator = allocator;
    ptr->capacity  = initial_size;
    ptr->length    = 0;
    ptr->item_size = item_size;

    return ptr + 1;
}

int expand_if_needed(void **array) {
    if (UNLIKELY(*array == NULL)) {
        return -EINVAL;
    }

    header_t *header = get_header(*array);

    // allocate more space if needed
    if (UNLIKELY(header->length == header->capacity)) {
        size_t current_size = header->item_size * header->capacity;
        size_t total_size    = sizeof(header_t) + current_size * 2;

        header_t *new_header = realloc_with(header->allocator, header, total_size);
        if (UNLIKELY(new_header == NULL)) {
            _ = logging(ALLOC, LOG_ERROR, "%s failed", __func__);
            return -ENOMEM;
        }

        *array = new_header + 1;
        new_header->capacity *= 2;
    }

    return 0;
}


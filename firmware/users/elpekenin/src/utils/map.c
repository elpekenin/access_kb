// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdlib.h>

#include "elpekenin/logging.h"
#include "elpekenin/utils/map.h"
#include "elpekenin/utils/memory.h"


static bool _add(map_t *self, const char *key, const void *value) {
    if (UNLIKELY(ptr_in_stack(value))) { // lets hope users do the intended thing
        logging(MAP, LOG_ERROR, "Value on stack");
        logging(MAP, LOG_TRACE, "Address: %p", value);
        return false;
    }

    for (size_t i = 0; i < self->n_items; ++i) {
        item_t *item = &self->items[i];
        if (item->key == NULL) {
            item->key = key;
            item->value = value;
            logging(MAP, LOG_TRACE, "Stored '%s': (%p)", key, value);
            return true;
        }
    }

    logging(MAP, LOG_ERROR, "No slots left '%s'", key);
    return false;
}

static bool _get(map_t *self, const char *key, const void **value) {
    for (size_t i = 0; i < self->n_items; ++i) {
        item_t *item = &self->items[i];
        if (
            item->key != NULL
            && strcmp(item->key, key) == 0
        ) {
            *value = item->value;
            logging(MAP, LOG_TRACE, "Read '%s': (%p)", key, *value);
            return true;
        }
    }

    logging(MAP, LOG_ERROR, "Key '%s' not found", key);
    return false;
}

static size_t total_items = 0;

size_t get_total_map_items(void) {
    return total_items;
}

map_t new_map(size_t n_items) {
    item_t *items = (item_t *)calloc(n_items, sizeof(item_t));

    total_items += n_items;

    return (map_t) {
        .n_items = n_items,
        .items   = items,
        .add     = _add,
        .get     = _get,
    };
}
// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/logging.h"
#include "elpekenin/utils/hash_map.h"
#include "elpekenin/utils/memory.h"


// slight edit of the implementation at <http://www.cse.yorku.ca/~oz/hash.html>
static inline hash_t _hash(const char *key) {
    char   c;
    hash_t hash = 0;

    while ((c = *key++)) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash % HASH_MAP_SIZE;
}

static bool _add(hash_map_t *self, const char *key, const void *value) {
    if (UNLIKELY(ptr_in_stack(value))) { // lets hope users do the intended thing
        logging(HASH, LOG_ERROR, "Value on stack");
        logging(HASH, LOG_TRACE, "Address: %p", value);
        return false;
    }

    hash_t hash = _hash(key);

    if (UNLIKELY(self->addrs[hash] != NULL)) { // lets hope we don't try and add the same key twice (nor get collisions)
        logging(HASH, LOG_ERROR, "Duplicated hash for key '%s'", key);
        return false;
    }

    self->addrs[hash] = (void *)value; // discard const
    logging(HASH, LOG_TRACE, "Stored '%s': (%p)", key, value);
    return true;
}

static bool _get(hash_map_t *self, const char *key, void **value) {
    hash_t hash = _hash(key);

    if (UNLIKELY(self->addrs[hash] == NULL)) { // lets hope users dont try reading something that hasn't been set
        logging(HASH, LOG_ERROR, "Key '%s' not found", key);
        *value = NULL; // so the address can be checked too
        return false;
    }

    *value = self->addrs[hash];
    logging(HASH, LOG_TRACE, "Read '%s': (%p)", key, *value);
    return true;
}

hash_map_t new_hash_map(void) {
    return (hash_map_t) {
        .add = _add,
        .get = _get,
    };
}
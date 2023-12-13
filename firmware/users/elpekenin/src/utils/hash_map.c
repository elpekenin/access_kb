// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/logging.h"

#include "elpekenin/utils/hash_map.h"

static inline hash_t _hash(const char *key) {
    size_t i    = 0;
    hash_t hash = 0;

    const char *cpy = key;
    while (*cpy) {
        hash += *cpy++ * ++i;
    }

    return hash;
}

static bool _add(hash_map_t *self, const char *key, const void *value) {
    if (UNLIKELY(addr_in_stack(value))) {
        logging(HASH, LOG_ERROR, "Value on stack");
        logging(HASH, LOG_TRACE, "Address: %p", value);
        return false;
    }

    hash_t hash = _hash(key);

    if (UNLIKELY(self->addrs[hash] != NULL)) {
        logging(HASH, LOG_ERROR, "Duplicated hash for key '%s'", key);
        return false;
    }

    self->addrs[hash] = (void *)value; // discard const
    logging(HASH, LOG_TRACE, "Stored '%s': (%p)", key, value);
    return true;
}

static bool _get(hash_map_t *self, const char *key, void **value) {
    hash_t hash = _hash(key);

    if (UNLIKELY(self->addrs[hash] == NULL)) {
        logging(HASH, LOG_ERROR, "Key '%s' not found", key);
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
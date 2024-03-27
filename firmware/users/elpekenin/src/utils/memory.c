// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <malloc.h>

#include "elpekenin/logging.h"
#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/wrapper.h"


// *** Analyze memory locations ***

#define ADDR(__var) ((void *)&__var)

// from ChibiOS' ld
extern uint8_t __main_stack_base__,
               __main_stack_end__,
               __process_stack_base__,
               __process_stack_end__,
               __bss_end__,
               __flash_binary_start,
               __flash_binary_end,
               __flash1_base__,
               __flash1_end__;

bool ptr_in_heap(const void *ptr) {
   return ADDR(__bss_end__) <= ptr && ptr <= ADDR(__process_stack_end__);
}

bool ptr_in_main_stack(const void *ptr) {
    return ADDR(__main_stack_base__) <= ptr && ptr <= ADDR(__main_stack_end__);
}

bool ptr_in_process_stack(const void *ptr) {
    return ADDR(__process_stack_base__) <= ptr && ptr <= ADDR(__process_stack_end__);
}

bool ptr_in_stack(const void *ptr) {
    return ptr_in_main_stack(ptr) || ptr_in_process_stack(ptr);
}

// adapted from <https://forums.raspberrypi.com/viewtopic.php?t=347638>
size_t get_heap_size(void) {
   return ADDR(__process_stack_end__) - ADDR(__bss_end__);
}

// *** Track heap usage ***

extern void *REAL(calloc)(size_t nmemb, size_t size);
extern void  REAL(free)(void *ptr);
extern void *REAL(malloc)(size_t size);
extern void *REAL(realloc)(void *ptr, size_t size);

static size_t used_heap = 0;

typedef struct PACKED {
    void * ptr;
    size_t size;
} usage_entry_t;

// size is fixed to prevent the tracker to be dynamic (use malloc) itself
static usage_entry_t entries[100] = {0};


size_t get_used_heap(void) {
    return used_heap;
}

static void push_record(void *ptr, size_t size) {
    if (UNLIKELY(ptr == NULL)) {
        return;
    }

    for (
        usage_entry_t *entry = entries;
        entry < &entries[ARRAY_SIZE(entries)];
        ++entry
    ) {
        if (entry->ptr == NULL) {
            used_heap += size;

            entry->ptr  = ptr;
            entry->size = size;

            logging(ALLOC, LOG_TRACE, "Pushed entry");
            return;
        }
    }

    logging(ALLOC, LOG_ERROR, "No slots for new entry");
}

static void pop_record(void *ptr) {
    if (UNLIKELY(ptr == NULL)) {
        return;
    }

    for (
        usage_entry_t *entry = entries;
        entry < &entries[ARRAY_SIZE(entries)];
        ++entry
    ) {
        if (entry->ptr == ptr && ptr != NULL) {
            used_heap -= entry->size;

            entry->ptr  = NULL;
            entry->size = 0;

            logging(ALLOC, LOG_TRACE, "Poped entry");
            return;
        }
    }

    logging(ALLOC, LOG_ERROR, "Entry not found");
}

// ^ data tracking
// -----
// v new malloc API

static inline void __alloc_entry(void) {
    logging(ALLOC, LOG_TRACE, "%s called", __func__);
}

static inline void __alloc_error(void) {
    logging(ALLOC, LOG_ERROR, "%s", __func__);
}

void *WRAP(calloc)(size_t nmemb, size_t size) {
    __alloc_entry();

    void *ptr = REAL(calloc)(nmemb, size);
    if (ptr == NULL) {
        __alloc_error();
        return NULL;
    }

    push_record(ptr, nmemb * size);

    return ptr;
}

void WRAP(free)(void *ptr) {
    __alloc_entry();

    REAL(free)(ptr);

    pop_record(ptr);
}

void *WRAP(malloc)(size_t size) {
    __alloc_entry();

    void *ptr = REAL(malloc)(size);
    if (ptr == NULL) {
        __alloc_error();
        return NULL;
    }

    push_record(ptr, size);

    return ptr;
}

void *WRAP(realloc)(void *ptr, size_t size) {
    __alloc_entry();

    void *new_ptr = REAL(realloc)(ptr, size);
    if (new_ptr == NULL) {
        __alloc_error();
        return NULL;
    }

    pop_record(ptr);
    push_record(new_ptr, size);

    return ptr;
}


#if defined(MCU_RP)
size_t get_flash_size(void) {
    return ADDR(__flash1_end__) - ADDR(__flash1_base__);
}

size_t get_used_flash(void) {
    return ADDR(__flash_binary_end) - ADDR(__flash_binary_start);
}
#endif

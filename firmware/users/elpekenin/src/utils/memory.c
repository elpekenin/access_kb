// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <malloc.h>

#include "elpekenin/logging.h"
#include "elpekenin/utils/compiler.h"


// *** Analyze memory locations ***

// from ChibiOS' ld
extern uint8_t __main_stack_base__[];
extern uint8_t __main_stack_end__[];
extern uint8_t __process_stack_base__[];
extern uint8_t __process_stack_end__[];

bool ptr_in_main_stack(const void *ptr) {
    return (
        (void *)__main_stack_base__ <= ptr
        && ptr <= (void *)__main_stack_end__
    );
}

bool ptr_in_process_stack(const void *ptr) {
    return (
        (void *)__process_stack_base__ <= ptr
        && ptr <= (void *)__process_stack_end__
    );
}

bool ptr_in_stack(const void *ptr) {
    return ptr_in_main_stack(ptr) || ptr_in_process_stack(ptr);
}


// *** Track heap usage ***

extern void *__real_calloc(size_t size);
extern void __real_free(void *ptr);
extern void *__real_malloc(size_t size);
extern void *__real_realloc(void *ptr, size_t size);

static size_t used_heap = 0;

typedef struct PACKED {
    void * ptr;
    size_t size;
} usage_entry_t;

static usage_entry_t entries[100] = {0};


size_t get_used_heap(void) {
    return used_heap;
}

static void push_record(void *ptr, size_t size) {
    for (uint8_t i = 0; i < ARRAY_SIZE(entries); ++i) {
        usage_entry_t *entry = &entries[i];
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
    for (uint8_t i = 0; i < ARRAY_SIZE(entries); ++i) {
        usage_entry_t *entry = &entries[i];
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

void *__wrap_calloc(size_t size) {
    void *ptr = __real_calloc(size);
    if (ptr == NULL) {
        logging(ALLOC, LOG_ERROR, "%s", __func__);
        return NULL;
    }
    push_record(ptr, size);
    return ptr;
}

void __wrap_free(void *ptr) {
    __real_free(ptr);
    pop_record(ptr);
}

void *__wrap_malloc(size_t size) {
    void *ptr = __real_malloc(size);
    if (ptr == NULL) {
        logging(ALLOC, LOG_ERROR, "%s", __func__);
        return NULL;
    }
    push_record(ptr, size);
    return ptr;
}

void *__wrap_realloc(void *ptr, size_t size) {
    pop_record(ptr);
    void *new_ptr = __real_realloc(ptr, size);
    if (new_ptr == NULL) {
        logging(ALLOC, LOG_ERROR, "%s", __func__);
        return NULL;
    }
    push_record(new_ptr, size);
    return ptr;
}
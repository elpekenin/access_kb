// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/utils/compiler.h"

typedef void * (*calloc_fn)(size_t, size_t);
typedef void   (*free_fn)(void *);
typedef void * (*malloc_fn)(size_t);
typedef void * (*realloc_fn)(void *, size_t);

typedef struct PACKED {
    calloc_fn  calloc;
    free_fn    free;
    malloc_fn  malloc;
    realloc_fn realloc;
} allocator_t;

extern allocator_t default_allocator;
// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdlib.h>

#include "elpekenin/utils/allocator.h"

allocator_t default_allocator = {
    .calloc  = calloc,
    .free    = free,
    .malloc  = malloc,
    .realloc = realloc,
};
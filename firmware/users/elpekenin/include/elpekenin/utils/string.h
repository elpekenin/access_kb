// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/utils/compiler.h"

// scratch buffer to do some string ops
extern char g_scratch[100];

// these operate inplace and return a pointer to the input for convenience

NON_NULL(2) RETURN_NO_NULL WRITE_ONLY(2) const char *_itoa(uint32_t value, char *buffer);
NON_NULL(1) RETURN_NO_NULL READ_WRITE(1, 2) const char *_reverse(char *str, size_t len);
NON_NULL(2) RETURN_NO_NULL WRITE_ONLY(2, 3) const char *pretty_bytes(size_t n, char *buffer, uint16_t buffer_size);

CONST bool is_utf8(char c);
CONST bool is_utf8_continuation(char c);
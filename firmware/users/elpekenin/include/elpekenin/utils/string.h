// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// these operate inplace and return a pointer to the input for convenience
const char *_itoa(uint32_t value, char *buffer);
const char *_reverse(char *str, size_t len);

bool is_utf8(char c);
bool is_utf8_continuation(char c);
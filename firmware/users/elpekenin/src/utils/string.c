// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>

#include "elpekenin/utils/shortcuts.h"

// reverse a string (inplace)
void _reverse(char *str) {
    size_t len = strlen(str);

    char   c;
    size_t left  = 0;
    size_t right = len - 1;

    while (left < right) {
        c          = str[left];
        str[left]  = str[right];
        str[right] = c;

        left++;
        right--;
    }
}

// custom (small-ish) impl of itoa
void _itoa(uint32_t value, char *result) {
    // convert to string by doing value/10
    char *copy = result;
    do {
        *copy++ = '0' + (value % 10); // this digit
        *copy   = '\0'; // add terminator upfront
    } while (value /= 10);

    _reverse(result); // invert the str
}

bool is_utf8(char c) {
    return GET_BIT(c, 7); // 1xxx xxxx
}

bool is_utf8_continuation(char c) {
    return is_utf8(c) & !GET_BIT(c, 6); // 10xx xxxx
}

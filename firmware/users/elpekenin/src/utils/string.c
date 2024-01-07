// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>
#include <stdio.h>

#include "elpekenin/utils/shortcuts.h"

bool is_utf8(char c) {
    return GET_BIT(c, 7); // 1xxx xxxx
}

bool is_utf8_continuation(char c) {
    return is_utf8(c) & !GET_BIT(c, 6); // 10xx xxxx
}

const char *_reverse(char *str) {
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

    return str;
}

const char *_itoa(uint32_t value, char *buffer) {
    // convert to string by doing value/10
    char *copy = buffer;
    do {
        *copy++ = '0' + (value % 10); // this digit
        *copy   = '\0'; // add terminator upfront
    } while (value /= 10);

    return _reverse(buffer); // invert the str
}

// returns pretty representation of an amount in bytes, eg: 8B or 1.3kB
char *pretty_bytes(size_t n, char *buffer, uint16_t buffer_size) {
    const static char *magnitudes[] = {"_", "kB", "MB", "GB"};

    // bytes
    if (n < 1024) {
        snprintf(buffer, buffer_size, "%5dB", n);
        return buffer;
    }

    uint8_t index = 0;
    size_t  copy  = n;
    while (copy >= 1024) {
        copy /= 1024;
        index++;
    }

    snprintf(buffer, buffer_size, "%3.2f%s", (float)n / 1024, magnitudes[index]);
    return buffer;
}
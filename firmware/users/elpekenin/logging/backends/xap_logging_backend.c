// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "xap.h"
#include "usb_descriptor.h"
#include "timer.h"
#define MAX_PAYLOAD_SIZE (XAP_EPSIZE - sizeof(xap_broadcast_header_t))

#include "user_utils.h"


static uint8_t i = 0;
static uint8_t buffer[MAX_PAYLOAD_SIZE] = {0};

void sendchar_xap_hook(uint8_t c) {
    // sending early on leads to issues, instead wait a bit for USB to settle
    if (timer_read() < 3000) {
        return;
    }

    // append data
    buffer[i++] = c;

    // send on '\n' or when buffer is filled
    // -1 because we need to keep last element 0 for terminator
    if (i == (ARRAY_SIZE(buffer) - 1) || c == '\n') {
        xap_broadcast(0x00, buffer, i);

        i = 0;
        memset(buffer, 0,ARRAY_SIZE(buffer));
    }
}
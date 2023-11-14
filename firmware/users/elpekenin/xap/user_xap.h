// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "quantum.h"
#include "usb_descriptor.h" // contains XAP_EPSIZE

#include "touch_driver.h"

// =====
// Helper to compute max string length
#define MAX_PAYLOAD (XAP_EPSIZE - sizeof(xap_broadcast_header_t))
/* the final type will be
 *
 * struct {
 *     base_type;
 *     string;
 *     byte; // make sure we have a terminator
 * }
 */
 #define _MAX_STR_LEN(base_type) (MAX_PAYLOAD - sizeof(base_type) - 1)

// =====
// Identifiers
typedef enum {
    _SCREEN_PRESSED,
    _SCREEN_RELEASED,
    _LAYER_CHANGE,
    _KEYEVENT,
    _SHUTDOWN,
} _xap_msg_id_t;

typedef uint8_t xap_msg_id_t;

// =====
// Actual messages
typedef struct {
    xap_msg_id_t msg_id;
    uint8_t      screen_id;
    uint16_t     x;
    uint16_t     y;
} PACKED screen_pressed_msg_t;

void xap_screen_pressed(uint8_t screen_id, touch_report_t report);


// ---
typedef struct {
    xap_msg_id_t msg_id;
    uint8_t      screen_id;
} PACKED screen_released_msg_t;

void xap_screen_released(uint8_t screen_id);

// ---
typedef struct {
    xap_msg_id_t msg_id;
    uint8_t      layer;
} PACKED layer_change_msg_t;

void xap_layer(layer_state_t state);


// ---
typedef struct {
    xap_msg_id_t msg_id;
    uint16_t     keycode;
    bool         pressed;
    uint8_t      layer;
    uint8_t      row;
    uint8_t      col;
    uint8_t      mods;
} PACKED _keyevent_msg_t;

typedef struct {
    _keyevent_msg_t base;
    char            str[_MAX_STR_LEN(_keyevent_msg_t)];
    uint8_t         null;
} PACKED keyevent_msg_t;
_Static_assert(sizeof(keyevent_msg_t) == MAX_PAYLOAD, "wrong size for keyevent_msg_t");


void xap_keyevent(uint16_t keycode, keyrecord_t *record);


// ---
typedef struct {
    xap_msg_id_t msg_id;
    bool         bootloader;
} PACKED shutdown_msg_t;

void xap_shutdown(bool jump_to_bootloader);

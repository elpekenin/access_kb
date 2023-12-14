// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "action.h" // keyrecord_t

void keycode_repr(const char **str);
void keylog_process(uint16_t keycode, keyrecord_t *record);

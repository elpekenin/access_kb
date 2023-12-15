// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include_next "transactions.h" // QMK's

void split_init(void);

void reset_ee_slave(void);
void build_info_sync_keymap_callback(void);

// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "transaction_ids.h"

void user_data_sync_keymap_callback(void);
void housekeeping_split_sync(uint32_t now);
void transactions_init(void);
void reset_ee_slave(void);

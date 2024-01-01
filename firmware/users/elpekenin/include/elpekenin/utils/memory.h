// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once 

bool ptr_in_main_stack(const void *addr);
bool ptr_in_process_stack(const void *addr);
bool ptr_in_stack(const void *addr);

size_t get_used_heap(void);
size_t get_total_heap(void);
float used_heap_percentage(void);
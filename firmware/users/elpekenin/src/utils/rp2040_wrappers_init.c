// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

extern void (*__preinit_array_base__)(void);
extern void (*__preinit_array_end__)(void);

void __late_init(void) {
    for (
        void (**func)(void) = &__preinit_array_base__;
        func < &__preinit_array_end__;
        func++
    ) {
        (*func)();
    }
}
// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// some RP-specific wrappers need to be initialized, the "good" way of
// doing it requires special handling (symbols) on the linker script
// so... just hardcode the functions on this file :)

// sorted alphabetically
extern void __aeabi_bits_init(void);
extern void __aeabi_double_init(void);
extern void __aeabi_float_init(void);
extern void __aeabi_mem_init(void);

void early_hardware_init_post(void) {
    // sorted by priority on the SDK
    __aeabi_mem_init();
    __aeabi_bits_init();
    __aeabi_double_init();
    __aeabi_float_init();
}
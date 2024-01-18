// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// some RP-specific wrappers need to be initialized, the "good" way of
// doing it requires special handling (symbols) on the linker script
// so... just hardcode the functions on this file :)

// Start and end points of the constructor list,
// defined by the linker script.
extern void (*__preinit_array_start)(void);
extern void (*__preinit_array_end)(void); 

// Start and end points of the constructor list,
// defined by the linker script.
extern void (*__init_array_start)(void);
extern void (*__init_array_end)(void);

// sorted by priority on the SDK
// constructor -> called before `main`
void __late_init(void) {
    // Call each function in the list.
    // We have to take the address of the symbols, as __preinit_array_start *is*
    // the first function pointer, not the address of it.
    for (void (**p)(void) = &__preinit_array_start; p < &__preinit_array_end; ++p) {
        (*p)();
    }

    // Call each function in the list.
    // We have to take the address of the symbols, as __init_array_start *is*
    // the first function pointer, not the address of it.
    for (void (**p)(void) = &__init_array_start; p < &__init_array_end; ++p) {
        (*p)();
    }
}
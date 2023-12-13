// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

bool addr_in_stack(const void *addr) {
    // from ChibiOS' ld
    extern uint8_t __main_stack_base__;
    extern uint8_t __main_stack_end__;
    extern uint8_t __process_stack_base__;
    extern uint8_t __process_stack_end__;

    void *msb = &__main_stack_base__;
    void *mse = &__main_stack_end__;
    void *psb = &__process_stack_base__;
    void *pse = &__process_stack_end__;

    return (
        (msb <= addr && addr <= mse)
        || (psb <= addr && addr <= pse)
    );
}
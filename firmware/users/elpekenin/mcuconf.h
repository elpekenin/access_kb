// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include_next <mcuconf.h>

#if defined(MCU_RP)
#    undef  RP_CORE1_START
#    define RP_CORE1_START TRUE
#endif

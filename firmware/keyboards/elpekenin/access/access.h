// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "qp.h"
#    if defined(INIT_EE_HANDS_LEFT)
extern painter_device_t il91874;
#    else
extern painter_device_t ili9163;
extern painter_device_t ili9341;
#    endif
#endif
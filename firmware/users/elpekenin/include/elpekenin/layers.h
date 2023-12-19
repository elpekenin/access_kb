// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

enum layers {
    _QWERTY,
    _FN1,
    _FN2,
    _FN3,
    _RST
};

#if defined(TRI_LAYER_ENABLE)
void configure_tri_layer(void);
#endif

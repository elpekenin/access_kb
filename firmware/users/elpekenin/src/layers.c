// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/layers.h"
#include "elpekenin/placeholders.h"

#if defined(TRI_LAYER_ENABLE)
#    include "tri_layer.h"
#endif

#if defined(XAP_ENABLE)
#    include "elpekenin/xap.h"
#endif

#if defined(TRI_LAYER_ENABLE)
void configure_tri_layer(void) {
    set_tri_layer_lower_layer(_FN1);
    set_tri_layer_upper_layer(_FN2);
    set_tri_layer_adjust_layer(_RST);
}
#endif // defined(TRI_LAYER_ENABLE)

layer_state_t layer_state_set_user(layer_state_t state) {
    state = layer_state_set_keymap(state);

#if defined(XAP_ENABLE)
    xap_layer(state);
#endif // defined(XAP_ENABLE)

    return state;
}

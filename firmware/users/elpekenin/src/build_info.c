// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>

#include <quantum/keyboard.h>
#include "version.h"

#include "elpekenin/build_info.h"

static build_info_t build_info = {0};

build_info_t get_build_info(void) {
    if (is_keyboard_master()) {
        strcpy(build_info.commit, QMK_GIT_HASH);
        build_info.features = get_enabled_features();
    }

    return build_info;
}

void set_build_commit(const char *commit) {
    strcpy(build_info.commit, commit);
}

void set_build_features(enabled_features_t features) {
    build_info.features = features;
}
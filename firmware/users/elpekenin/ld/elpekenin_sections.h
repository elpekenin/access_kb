#pragma once

#include "elpekenin/utils/ld.h"

#define LD_BLOCK(x) \
    . = ALIGN(4); \
    LD_START(x) = .; \
    KEEP(*(SORT(LD_NAME(x).*))); \
    LD_END(x) = .;

SECTIONS {
   .elpekenin_funcs : ALIGN(4) {
      LD_BLOCK(init)
      LD_BLOCK(deinit)
      LD_BLOCK(sendchar)
    } > flash1
}
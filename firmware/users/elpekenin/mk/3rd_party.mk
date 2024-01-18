3RD_PARTY = $(USER_PATH)/3rd_party

BACKTRACE_ENABLE ?= yes
ifeq ($(strip $(BACKTRACE_ENABLE)), yes)
    BACKTRACE_BASE = $(3RD_PARTY)/backtrace

    CFLAGS += -funwind-tables \
              -Wframe-address \
              -mpoke-function-name \
              -fno-omit-frame-pointer

    VPATH += $(BACKTRACE_BASE)/include
    SRC += $(BACKTRACE_BASE)/backtrace/backtrace.c
endif

MATH_M0_ENABLE ?= no
ifeq ($(strip $(MATH_M0_ENABLE)), yes)
    MATH_BASE = $(3RD_PARTY)/ArmMathM0/src

    VPATH += $(MATH_BASE)/include
    SRC += $(wildcard $(MATH_BASE)/**/*.S)
endif
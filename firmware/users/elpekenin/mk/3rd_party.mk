3RD_PARTY = $(USER_PATH)/3rd_party

# backtrace
BACKTRACE_BASE = $(3RD_PARTY)/backtrace
VPATH += $(BACKTRACE_BASE)/include

CFLAGS += -funwind-tables \
          -Wframe-address \
          -mpoke-function-name \
          -fno-omit-frame-pointer

SRC += $(BACKTRACE_BASE)/backtrace/backtrace.c

# math M0
MATH_BASE = $(3RD_PARTY)/ArmMathM0/src
SRC += $(wildcard $(MATH_BASE)/**/*.S)
VPATH += $(MATH_BASE)/include
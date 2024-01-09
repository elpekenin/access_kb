SRC += $(USER_SRC)/3rd_party/backtrace.c
CFLAGS += -funwind-tables \
          -mpoke-function-name \
          -fno-omit-frame-pointer

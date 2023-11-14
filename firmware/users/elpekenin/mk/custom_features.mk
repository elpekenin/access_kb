ONE_HAND ?= no
ifeq ($(strip $(ONE_HAND)), yes)
    RGB_MATRIX_CUSTOM_USER = yes
    OPT_DEFS += -DONE_HAND_ENABLE
endif

TOUCH_SCREEN ?= no
ifeq ($(strip $(TOUCH_SCREEN)), yes)
    QUANTUM_LIB_SRC += spi_master.c
    OPT_DEFS += -DTOUCH_SCREEN_ENABLE
    SRC += touch_driver.c
endif

SIPO_PINS ?= no
ifeq ($(strip $(SIPO_PINS)), yes)
    OPT_DEFS += -DSIPO_PINS_ENABLE
    SRC += sipo_pins.c

    # needs a second SPI driver to work, currently not supported on QMK
    SRC += custom_spi_master.c

    ifeq ($(strip $(TOUCH_SCREEN)), yes)
        SRC += touch_sipo.c
    endif

    ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
        SRC += qp_comms_sipo.c
    endif
endif

KEYLOG_ENABLE ?= no
ifeq ($(strip $(KEYLOG_ENABLE)), yes)
    OPT_DEFS += -DKEYLOG_ENABLE
    SRC += user_keylog.c
endif
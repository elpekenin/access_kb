CUSTOM_MATRIX = lite
SRC += matrix.c

SERIAL_DRIVER = vendor

# Custom features
SIPO_PINS = yes

# **dont** want to default to GENERIC_PROMICRO_RP2040
BOARD = GENERIC_RP_RP2040

# built with 16M Pico's
EXTRALDFLAGS = -Wl,--defsym,FLASH_LEN=16384k

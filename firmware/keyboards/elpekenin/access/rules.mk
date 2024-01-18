CUSTOM_MATRIX = lite
SRC += matrix.c

SERIAL_DRIVER = vendor

# Custom features
SIPO_PINS = yes

# **dont** want to default to GENERIC_PROMICRO_RP2040
BOARD = GENERIC_RP_RP2040

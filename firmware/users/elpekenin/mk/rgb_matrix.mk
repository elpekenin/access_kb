ifeq ($(strip $(RGB_MATRIX_ENABLE)), yes)
    SRC += user_rgb_matrix.c \
           user_rgb_matrix_indicators.c
endif

VPATH += $(USER_PATH)/autocorrect \
         $(USER_PATH)/keylog \
         $(USER_PATH)/layers \
         $(USER_PATH)/logging \
         $(USER_PATH)/logging/backends \
         $(USER_PATH)/painter \
         $(USER_PATH)/painter/images \
         $(USER_PATH)/painter/fonts \
         $(USER_PATH)/placeholders \
         $(USER_PATH)/registers \
         $(USER_PATH)/rgb \
         $(USER_PATH)/spi \
         $(USER_PATH)/split \
         $(USER_PATH)/touch \
         $(USER_PATH)/xap

SRC += elpekenin.c \
       placeholders.c \
       user_layers.c \
       user_power.c \
       user_process.c \
       user_utils.c

# Enabled by default
AUTOCORRECT_ENABLE ?= yes
DEFERRED_EXEC_ENABLE ?= yes
KEY_OVERRIDE_ENABLE ?= yes
TRI_LAYER_ENABLE ?= yes

DYNAMIC_KEYMAP_ENABLE = no

MK_PATH = $(USER_PATH)/mk
# conditionals on these files, less boilerplate here
include $(MK_PATH)/codegen.mk

include $(MK_PATH)/custom_features.mk
include $(MK_PATH)/logging.mk
include $(MK_PATH)/painter.mk
include $(MK_PATH)/rgb_matrix.mk
include $(MK_PATH)/split.mk
include $(MK_PATH)/unicode.mk
include $(MK_PATH)/xap.mk

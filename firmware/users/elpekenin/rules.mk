USER_SRC = $(USER_PATH)/src
USER_INCLUDE = $(USER_PATH)/include

VPATH += $(USER_INCLUDE)

SRC += $(USER_SRC)/elpekenin.c \
       $(USER_SRC)/placeholders.c \
       $(USER_SRC)/layers.c \
       $(USER_SRC)/power.c \
       $(USER_SRC)/process.c \
       $(USER_SRC)/utils/compiler.c \
       $(USER_SRC)/utils/hash_map.c \
       $(USER_SRC)/utils/string.c

# Default configuration
AUTOCORRECT_ENABLE ?= yes
DEFERRED_EXEC_ENABLE ?= yes
KEY_OVERRIDE_ENABLE ?= yes
TRI_LAYER_ENABLE ?= yes
DYNAMIC_KEYMAP_ENABLE ?= no

MK_PATH = $(USER_PATH)/mk
# Include regardless, check inside the files
include $(MK_PATH)/codegen.mk

include $(MK_PATH)/custom_features.mk
include $(MK_PATH)/games.mk
include $(MK_PATH)/logging.mk
include $(MK_PATH)/painter.mk
include $(MK_PATH)/rgb_matrix.mk
include $(MK_PATH)/rng.mk
include $(MK_PATH)/split.mk
include $(MK_PATH)/unicode.mk
include $(MK_PATH)/xap.mk

# downloaded (sometimes) using qmk_build
# dont error if it is missing
-include $(MK_PATH)/rtc.mk

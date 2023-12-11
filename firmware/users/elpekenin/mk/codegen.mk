USER_SCRIPTS = $(USER_PATH)/scripts
USER_GENERATED = $(USER_PATH)/generated

# make sure folder exists and is empty
$(shell mkdir -p $(USER_GENERATED))
$(shell rm -rf $(USER_GENERATED)/*)

# enabled_features_t
$(shell $(USER_SCRIPTS)/features.py $(USER_GENERATED))
SRC += $(USER_GENERATED)/features.c

# keycode_str
$(shell $(USER_SCRIPTS)/keycode_str.py $(USER_GENERATED) $(KEYMAP_PATH))
SRC += $(USER_GENERATED)/keycode_str.c

# QP assets
ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    # not really codegen, but let's convert images to .qgf here
    # it will iterate all files on every folder under $(USER_PATH)/painter/images
    QGF_FORMAT ?= rgb565
    $(shell $(USER_PATH)/painter/images/img2qgf $(QGF_FORMAT))

    # actual codegen
    QP_DIRS := $(KEYBOARD_PATHS) $(KEYMAP_PATH) $(USER_PATH)
    $(shell $(USER_SCRIPTS)/qp_resources.py $(USER_GENERATED) $(QP_DIRS))
    SRC += $(USER_GENERATED)/qp_resources.c \
           $(USER_GENERATED)/features_draw.c
    include $(USER_GENERATED)/qp_resources.mk
endif

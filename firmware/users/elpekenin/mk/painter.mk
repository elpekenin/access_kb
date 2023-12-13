ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    # defines `load_qp_resources` to load them into RAM
    SRC += $(USER_SRC)/qp/graphics.c
endif

ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    # defines `load_qp_resources` to load them into RAM
    SRC += graphics.c \
           qp_logging_backend.c
endif

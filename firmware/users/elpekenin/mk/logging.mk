ifeq ($(strip $(QUANTUM_PAINTER_ENABLE_ENABLE)), yes)
    SRC += qp_logging_backend.c
endif

ifeq ($(strip $(SPLIT_KEYBOARD)), yes)
    SRC += split_logging_backend.c
endif

ifeq ($(strip $(XAP_ENABLE)), yes)
    SRC += xap_logging_backend.c
endif

SRC += logging_backend.c \
       logging_frontend.c
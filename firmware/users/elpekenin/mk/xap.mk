ifeq ($(strip $(XAP_ENABLE)), yes)
    SRC += user_xap.c

    QP_XAP ?= yes
    ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
        ifeq ($(strip $(QP_XAP)), yes)
            OPT_DEFS += -DQP_XAP_ENABLE
            SRC += qp_over_xap.c
        endif
    endif
endif

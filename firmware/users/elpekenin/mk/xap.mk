ifeq ($(strip $(XAP_ENABLE)), yes)
    SRC += $(USER_SRC)/xap.c

    QP_XAP ?= yes
    ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
        ifeq ($(strip $(QP_XAP)), yes)
            OPT_DEFS += -DQP_XAP_ENABLE
            SRC += $(USER_SRC)/painter/xap.c
        endif
    endif
endif

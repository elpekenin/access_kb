# tell linker to wrap a function, applied to a list of names
define WRAP
    $(foreach FUNC,$1,$(eval CFLAGS += -Wl,--wrap=$(FUNC)))
endef


# manually track dynamic memory usage
ALLOC_FUNCTIONS := calloc free malloc realloc
$(call WRAP, $(ALLOC_FUNCTIONS))

# ===== pico-sdk wrappers =====
# NOTE: QMK adds divider and int64_ops
ifeq ($(MCU_SERIES), RP2040)
    OPT_DEFS += -DPICO_BITS_IN_RAM=1 \
                -DPICO_DOUBLE_IN_RAM=1 \
                -DPICO_DOUBLE_SUPPORT_ROM_V1=1 \
                -DPICO_FLOAT_IN_RAM=1 \
                -DPICO_FLOAT_SUPPORT_ROM_V1=1 \
                -DPICO_MEM_IN_RAM=1

    EXTRAINCDIRS += $(PICOSDKROOT)/src/rp2_common/pico_double/include \
                    $(PICOSDKROOT)/src/rp2_common/pico_float/include

    SRC += $(PICOSDKROOT)/src/rp2_common/pico_bit_ops/bit_ops_aeabi.S \
           $(PICOSDKROOT)/src/rp2_common/pico_double/double_aeabi.S \
           $(PICOSDKROOT)/src/rp2_common/pico_double/double_v1_rom_shim.S \
           $(PICOSDKROOT)/src/rp2_common/pico_double/double_math.c \
           $(PICOSDKROOT)/src/rp2_common/pico_double/double_init_rom.c \
           $(PICOSDKROOT)/src/rp2_common/pico_float/float_aeabi.S \
           $(PICOSDKROOT)/src/rp2_common/pico_float/float_v1_rom_shim.S \
           $(PICOSDKROOT)/src/rp2_common/pico_float/float_math.c \
           $(PICOSDKROOT)/src/rp2_common/pico_float/float_init_rom.c \
           $(PICOSDKROOT)/src/rp2_common/pico_mem_ops/mem_ops_aeabi.S

    SRC += $(USER_SRC)/utils/rp2040_wrappers_init.c

    BIT_FUNCTIONS := __clzsi2 __clzdi2 __ctzsi2 __ctzdi2 __popcountsi2 __popcountdi2

    DOUBLE_BUILTINS := __aeabi_dadd __aeabi_ddiv __aeabi_dmul __aeabi_drsub __aeabi_dsub __aeabi_cdcmpeq \
                       __aeabi_cdrcmple __aeabi_cdcmple __aeabi_dcmpeq __aeabi_dcmplt __aeabi_dcmple \
                       __aeabi_dcmpge __aeabi_dcmpgt __aeabi_dcmpun __aeabi_i2d __aeabi_l2d __aeabi_ui2d \
                       __aeabi_ul2d __aeabi_d2iz __aeabi_d2lz __aeabi_d2uiz __aeabi_d2ulz __aeabi_d2f
    DOUBLE_FUNCTIONS := sqrt cos sin tan atan2 exp log ldexp copysign trunc floor ceil round sincos asin \
                        acos atan sinh cosh tanh asinh acosh atanh exp2 log2 exp10 log10 pow powint hypot \
                        cbrt fmod drem remainder remquo expm1 log1p fma

    FLOAT_BUILTINS := __aeabi_fadd __aeabi_fdiv __aeabi_fmul __aeabi_frsub __aeabi_fsub __aeabi_cfcmpeq \
                      __aeabi_cfrcmple __aeabi_cfcmple __aeabi_fcmpeq __aeabi_fcmplt __aeabi_fcmple \
                      __aeabi_fcmpge __aeabi_fcmpgt __aeabi_fcmpun __aeabi_i2f __aeabi_l2f __aeabi_ui2f \
                      __aeabi_ul2f __aeabi_f2iz __aeabi_f2lz __aeabi_f2uiz __aeabi_f2ulz __aeabi_f2d
    FLOAT_FUNCTIONS := sqrtf cosf sinf tanf atan2f expf logf ldexpf copysignf truncf floorf ceilf roundf \
                       sincosf asinf acosf atanf sinhf coshf tanhf asinhf acoshf atanhf exp2f log2f exp10f \
                       log10f powf powintf hypotf cbrtf fmodf dremf remainderf remquof expm1f log1pf fmaf

    MEM_FUNCTIONS := memcpy __aeabi_memcpy __aeabi_memcpy4 __aeabi_memcpy8 memset __aeabi_memset __aeabi_memset4 __aeabi_memset8

    # $(call WRAP, $(BIT_FUNCTIONS))
    # $(call WRAP, $(DOUBLE_BUILTINS))
    # $(call WRAP, $(DOUBLE_FUNCTIONS))
    # $(call WRAP, $(FLOAT_BUILTINS))
    # $(call WRAP, $(FLOAT_FUNCTIONS))
    # $(call WRAP, $(MEM_FUNCTIONS))
endif
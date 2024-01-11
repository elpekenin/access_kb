# tell linker to wrap a function, applied to a list of names
define WRAP
    $(foreach FUNC,$1,$(eval CFLAGS += -Wl,--wrap=$(FUNC)))
endef


# manually track dynamic memory usage
ALLOC_FUNCTIONS := calloc free malloc realloc
$(call WRAP, $(ALLOC_FUNCTIONS))

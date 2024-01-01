# make some funcs go thru wrappers, to extend/edit functionality

EXTRALDFLAGS += -Wl,--wrap=calloc \
                -Wl,--wrap=free \
                -Wl,--wrap=malloc \
                -Wl,--wrap=realloc

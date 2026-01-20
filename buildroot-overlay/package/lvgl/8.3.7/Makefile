#
# Makefile
#
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= $(CURDIR)
CFLAGS ?= -mcpu=c908v -O3 -I$(LVGL_DIR)/ -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wall -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wtype-limits -Wshift-negative-value -Wstack-usage=2048 -Wno-unused-value -Wno-unused-parameter -Wno-missing-field-initializers -Wuninitialized -Wmaybe-uninitialized -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wpointer-arith -Wno-cast-qual -Wmissing-prototypes -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wno-discarded-qualifiers -Wformat-security -Wno-ignored-qualifiers -Wno-sign-compare -D_DEFAULT_SOURCE -std=c99
LDFLAGS ?= -mcpu=c908v
LIBS += -lm -ldrm
BIN = lvgl_demo_music
obj ?= output

CFLAGS += -flto
LDFLAGS += -flto

# LDFLAGS += -Wl,-Map=$(BIN).map,--cref
CFLAGS += $(CFLAG)
LDFLAGS += $(LDFLAG) $(CFLAG)

prefix ?= /usr
bindir ?= $(prefix)/bin

#Collect the files to compile
MAINSRC = main.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk

CSRCS +=$(LVGL_DIR)/mouse_cursor_icon.c

OBJEXT ?= .o

AOBJS =  $(addprefix $(obj)/,  $(patsubst $(LVGL_DIR)/%, %, $(ASRCS:.S=$(OBJEXT)))   )
COBJS =  $(addprefix $(obj)/,  $(patsubst $(LVGL_DIR)/%, %, $(CSRCS:.c=$(OBJEXT)))   )
MAINOBJ =$(addprefix $(obj)/,  $(patsubst $(LVGL_DIR)/%, %, $(MAINSRC:.c=$(OBJEXT))) )


SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

#$(info "---------------------$(COBJS)    ")

all: default

${obj}/%.o: %.c
	@echo "CC $<"
	@mkdir -p $(dir $@)
	@$(CC)  $(CFLAGS) -c $< -o $@

default: $(AOBJS) $(COBJS) $(MAINOBJ)
	@echo LD $(BIN)
	@$(CC) $(LDFLAGS) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LIBS)

clean:
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)

install:
	install -d $(DESTDIR)$(bindir)
	install $(BIN) $(DESTDIR)$(bindir)

uninstall:
	$(RM) -r $(addprefix $(DESTDIR)$(bindir)/,$(BIN))

#
# Makefile
#
AUDIO_DEMO_DIR_NAME ?= audio_demo
AUDIO_DEMO_DIR ?= ${shell pwd}
# CFLAGS := -mcpu=c908v -O2
# LDFLAGS := -mcpu=c908v -O2

ifeq ($(BR2_RISCV_32), y)
CFLAGS += -march=rv32gcv_xtheadc
LDFLAGS += -march=rv32gcv_xtheadc
else
CFLAGS += -mcpu=c908v -O2
LDFLAGS += -mcpu=c908v -O2
endif

BIN = audio_demo

LIBS+=-lalure2
LIBS+=-lasound

SRCS:=$(wildcard *.c)
COBJS:=$(SRCS:.c=.o)

all:$(BIN)
	@rm -f $(COBJS)

$(COBJS) : %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)

$(BIN):$(COBJS)
	$(CC) $(LDFLAGS) -o $(BIN) $(COBJS) $(LIBPATH) $(LIBS)
	@rm -f  $(COBJS)

clean:
	@rm -f $(BIN) $(COBJS)


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

#ALSA_LIB_PATH=${K230_SDK_ROOT}/output/k230_canmv_defconfig/little/buildroot-ext/target/usr/lib
#ALSA_INC_PATH=${K230_SDK_ROOT}/output/k230_canmv_defconfig/little/buildroot-ext/host/riscv64-buildroot-linux-gnu/sysroot/usr/include

#MINIMP3_LIB_PATH=${K230_SDK_ROOT}/output/k230_canmv_defconfig/target/usr/lib    #
#MINIMP3_INC_PATH=${K230_SDK_ROOT}/output/k230_canmv_defconfig/build/alure-14beed2a86d5a36030e907b21c46614d505f07cd/src/decoders #minimp3.h

# LIBPATH+=-L$(ALSA_LIB_PATH)
# LIBPATH+=-L$(MINIMP3_LIB_PATH)

# INCS+=-I$(ALSA_INC_PATH)
# INCS+=-I$(MINIMP3_INC_PATH)

LIBS+=-lalure2
LIBS+=-lasound

SRCS:=$(wildcard *.c)
COBJS:=$(SRCS:.c=.o)

all:$(BIN)
	@rm -f $(COBJS)
	cp $(BIN) ~/nfs

$(COBJS) : %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)

$(BIN):$(COBJS)
	$(CC) $(LDFLAGS) -o $(BIN) $(COBJS) $(LIBPATH) $(LIBS)
	@rm -f  $(COBJS)

clean:
	@rm -f $(BIN) $(COBJS)


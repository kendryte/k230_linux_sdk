#
# Makefile
#
CPP=riscv64-unknown-linux-gnu-g++
AUDIO_REC_PLAY_DIR_NAME ?= audio_rec_play
AUDIO_REC_PLAY_DIR ?= ${shell pwd}
# CFLAGS := -mcpu=c908v -O2
# LDFLAGS := -mcpu=c908v -O2

# ifeq ($(BR2_RISCV_32), y)
# CFLAGS += -march=rv32gcv_xtheadc
# LDFLAGS += -march=rv32gcv_xtheadc
# else
# CFLAGS += -mcpu=c908v -O2
# LDFLAGS += -mcpu=c908v -O2
# endif

BIN = audio_rec_play

LIBS+=-lalure2
LIBS+=-lasound
LIBS+=-lspeexdsp
LIBS+=-lpthread
LIBS+=-lwebrtc-audio-processing-1
INCS+=-I$(STAGING_DIR)/usr/include
INCS+=-I$(STAGING_DIR)/usr/include/webrtc-audio-processing-1

SRCS:=$(wildcard *.cc)
COBJS:=$(SRCS:.cc=.o)

all:$(BIN)
	@rm -f $(COBJS)

$(COBJS) : %.o: %.cc
	$(CPP) -c $(CFLAGS) $< -o $@ $(INCS)

$(BIN):$(COBJS)
	$(CPP) $(LDFLAGS) -o $(BIN) $(COBJS) $(LIBPATH) $(LIBS)
	@rm -f  $(COBJS)

clean:
	@rm -f $(BIN) $(COBJS)


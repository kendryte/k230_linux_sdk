#
# Makefile
#
CPP=riscv64-unknown-linux-gnu-g++
CAMERA_RTSP_DEMO_DIR_NAME ?= camera_rtsp_demo
CAMERA_RTSP_DEMO_DIR ?= ${shell pwd}

ifeq ($(BR2_RISCV_32), y)
CFLAGS += -march=rv32gcv_xtheadc
LDFLAGS += -march=rv32gcv_xtheadc
else
CFLAGS += -mcpu=c908v -O2
LDFLAGS += -mcpu=c908v -O2
endif

BIN = camera_rtsp_demo

INCS+=-I./include -I$(STAGING_DIR)/usr/include
LIBPATH += -L$(STAGING_DIR)/usr/lib


LIBS+= -lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment -lrtsp_server
LIBS+= -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lz -lswscale -lavresample -lpthread

LDFLAGS += -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DNO_OPENSSL=1

SRCS:=$(wildcard *.cpp)
CPPOBJS:=$(SRCS:.cpp=.o)

all:$(BIN)
	@rm -f $(CPPOBJS)

$(CPPOBJS) : %.o: %.cpp
	$(CPP) -c $(CFLAGS) $< -o $@ $(INCS)

$(BIN):$(CPPOBJS)
	$(CPP) $(LDFLAGS) -o $(BIN) $(CPPOBJS) $(LIBPATH) $(LIBS)
	@rm -f  $(CPPOBJS)

clean:
	@rm -f $(BIN) $(CPPOBJS)


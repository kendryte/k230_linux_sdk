#
# Makefile
#
CPP=riscv64-unknown-linux-gnu-g++
LIBRTSP_SERVER_DIR_NAME ?= rtsp_server
LIBRTSP_SERVER_DIR ?= ${shell pwd}

ifeq ($(BR2_RISCV_32), y)
CFLAGS += -march=rv32gcv_xtheadc
LDFLAGS += -march=rv32gcv_xtheadc
else
CFLAGS += -mcpu=c908v -O2
LDFLAGS += -mcpu=c908v -O2
endif

BIN = librtsp_server.so
LIVE555_USAGEENVIRONMENT= UsageEnvironment
LIVE555_BASICUSAGEENVIRONMENT= BasicUsageEnvironment
LIVE555_GROUPSOCK = groupsock
LIVE555_LIVEMEDIA = liveMedia

INCS+=-I./include -I$(STAGING_DIR)/usr/include/$(LIVE555_GROUPSOCK) -I$(STAGING_DIR)/usr/include/$(LIVE555_USAGEENVIRONMENT) -I$(STAGING_DIR)/usr/include/$(LIVE555_BASICUSAGEENVIRONMENT) -I$(STAGING_DIR)/usr/include/$(LIVE555_LIVEMEDIA)


LIBS+= -lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment
LDFLAGS += -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DNO_OPENSSL=1
CFLAGS+= -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DNO_OPENSSL=1

SRCS:=$(wildcard *.cpp)
CPPOBJS:=$(SRCS:.cpp=.o)

all:$(BIN)
	@rm -f $(CPPOBJS)

$(CPPOBJS) : %.o: %.cpp
	$(CPP) -c -fPIC $(CFLAGS) $< -o $@ $(INCS)

$(BIN):$(CPPOBJS)
	$(CPP) -shared -o $(BIN) $(CPPOBJS)
	@rm -f  $(CPPOBJS)

clean:
	@rm -f $(BIN) $(CPPOBJS)


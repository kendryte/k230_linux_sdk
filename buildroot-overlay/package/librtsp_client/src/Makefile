#
# Makefile
#
# CPP=riscv64-unknown-linux-gnu-g++
LIBRTSP_CLIENT_DIR_NAME ?= rtsp_client
LIBRTSP_CLIENT_DIR ?= ${shell pwd}

BIN = librtsp_client.so
LIVE555_USAGEENVIRONMENT= UsageEnvironment
LIVE555_BASICUSAGEENVIRONMENT= BasicUsageEnvironment
LIVE555_GROUPSOCK = groupsock
LIVE555_LIVEMEDIA = liveMedia

INCS+=-I./include -I$(STAGING_DIR)/usr/include/$(LIVE555_GROUPSOCK) -I$(STAGING_DIR)/usr/include/$(LIVE555_USAGEENVIRONMENT) -I$(STAGING_DIR)/usr/include/$(LIVE555_BASICUSAGEENVIRONMENT) -I$(STAGING_DIR)/usr/include/$(LIVE555_LIVEMEDIA)

LIBS+= -lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment -lpthread
LDFLAGS += -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DNO_OPENSSL=1
CFLAGS+= -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DNO_OPENSSL=1

# librtsp_server.so has its own LiveFrameSource/G711LiveFrameSource classes with
# these same names but a different layout.  A program that links both libraries
# would otherwise get whichever copy the dynamic linker resolved first, and then
# read one class through the other's vtable.  exports.map keeps everything except
# the KdRtspClient API internal to this library, so the two cannot be confused.
LDFLAGS += -Wl,--version-script=$(LIBRTSP_CLIENT_DIR)/exports.map

SRCS:=$(wildcard *.cpp)
CPPOBJS:=$(SRCS:.cpp=.o)

all:$(BIN)
	@rm -f $(CPPOBJS)

$(CPPOBJS) : %.o: %.cpp
	$(CPP) -c -fPIC $(CFLAGS) $< -o $@ $(INCS)

$(BIN):$(CPPOBJS) exports.map
	$(CPP) -shared -o $(BIN) $(CPPOBJS) $(LDFLAGS) $(LIBS)
	@rm -f  $(CPPOBJS)

clean:
	@rm -f $(BIN) $(CPPOBJS)

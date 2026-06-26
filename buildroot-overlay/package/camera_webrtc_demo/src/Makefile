#
# Makefile for camera_webrtc_demo
#
BIN = camera_webrtc_demo

INCS += -I./include -I$(STAGING_DIR)/usr/include
INCS += -I$(STAGING_DIR)/usr/include/peer
INCS += -I$(STAGING_DIR)/usr/include/cjson

LIBPATH += -L$(STAGING_DIR)/usr/lib -L./

# Shared libraries (ffmpeg + libevent)
LIBS += -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lz -lswscale
LIBS += -levent -lpthread

# Static libraries from webrtc package (order matters for static linking:
# dependencies must come AFTER the libraries that reference them)
WEBRTC_LIBS = $(STAGING_DIR)/usr/lib/libpeer.a
WEBRTC_LIBS += $(STAGING_DIR)/usr/lib/libsrtp2.a
WEBRTC_LIBS += $(STAGING_DIR)/usr/lib/libusrsctp.a
WEBRTC_LIBS += $(STAGING_DIR)/usr/lib/libmbedtls.a
WEBRTC_LIBS += $(STAGING_DIR)/usr/lib/libmbedx509.a
WEBRTC_LIBS += $(STAGING_DIR)/usr/lib/libmbedcrypto.a
LIBS += -lcjson

LDFLAGS += -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DNO_OPENSSL=1

# C sources (http_server from sample_webrtc)
C_SRCS := $(wildcard *.c)
C_OBJS := $(C_SRCS:.c=.o)

# C++ sources
CXX_SRCS := $(wildcard *.cpp)
CXX_OBJS := $(CXX_SRCS:.cpp=.o)

all: $(BIN)
	@rm -f $(C_OBJS) $(CXX_OBJS)

$(C_OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCS)

$(CXX_OBJS): %.o: %.cpp
	$(CPP) -c $(CFLAGS) $< -o $@ $(INCS)

$(BIN): $(C_OBJS) $(CXX_OBJS)
	$(CPP) $(LDFLAGS) -o $(BIN) $(CXX_OBJS) $(C_OBJS) $(WEBRTC_LIBS) $(LIBPATH) $(LIBS)
	@rm -f $(C_OBJS) $(CXX_OBJS)

clean:
	@rm -f $(BIN) $(C_OBJS) $(CXX_OBJS)

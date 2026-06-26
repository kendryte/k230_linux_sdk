include ../../mkenv.mk
include $(SDK_RTSMART_SRC_DIR)/libs/mk/toolchain_riscv64_musl.mk

include $(SDK_RTSMART_SRC_DIR)/libs/mk/librtsmart_hal.mk
include $(SDK_RTSMART_SRC_DIR)/libs/mk/libmpp.mk

DIR = $(shell basename `pwd`)
LOCAL_DIR = $(shell pwd)
LIB = $(RTSMART_3RD_PARTY_LIB_INSTALL_PATH)/$(DIR).a
LIB_DIR = $(LOCAL_DIR)/3rd-party/lib
BUILD = $(SDK_RTSMART_BUILD_DIR)/libs/3rd-party/$(DIR)/

CWARN :=

C_CXX_FLAGS := -march=rv64imafdcv -mabi=lp64d -mcmodel=medany -Os $(CWARN) $(LIB_CFLAGS)
C_CXX_FLAGS += -I$(SDK_SRC_ROOT_DIR)/include -include "generated/autoconf.h"

CFLAGS := $(C_CXX_FLAGS) -std=gnu99
CXXFLAGS := $(C_CXX_FLAGS) -std=c++20

CFLAGS += -D_GNU_SOURCE
CFLAGS += -DCONFIG_IPV6=0
CFLAGS += -DCONFIG_USE_LWIP=0
CFLAGS += -DCONFIG_USE_USRSCTP=1
CFLAGS += -DHTTP_DO_NOT_USE_CUSTOM_CONFIG
CFLAGS += -DMQTT_DO_NOT_USE_CUSTOM_CONFIG
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wno-discarded-qualifiers

# libpeer own source includes
CFLAGS += -Isrc

# coreHTTP includes
CFLAGS += -I3rd-party/coreHTTP/source/include
CFLAGS += -I3rd-party/coreHTTP/source/interface
CFLAGS += -I3rd-party/coreHTTP/source/dependency/3rdparty/llhttp/include

# coreMQTT includes
CFLAGS += -I3rd-party/coreMQTT/source/include
CFLAGS += -I3rd-party/coreMQTT/source/interface

# SDK-installed 3rd-party includes
CFLAGS += -I$(RTSMART_3RD_PARTY_INC_INSTALL_PATH)/cJSON

# Private 3rd-party dependency includes (relative paths, self-contained)
CFLAGS += -I3rd-party/mbedtls/include
CFLAGS += -I3rd-party/libsrtp/include
CFLAGS += -I3rd-party/libsrtp/crypto/include
CFLAGS += -I3rd-party/usrsctp/usrsctplib

# libpeer core source files
CFILES := \
	src/agent.c \
	src/base64.c \
	src/address.c \
	src/dtls_srtp.c \
	src/ice.c \
	src/mdns.c \
	src/peer.c \
	src/peer_connection.c \
	src/peer_signaling.c \
	src/ports.c \
	src/rtcp.c \
	src/rtp.c \
	src/sctp.c \
	src/sdp.c \
	src/socket.c \
	src/ssl_transport.c \
	src/stun.c \
	src/utils.c \

# coreHTTP source files
CFILES += \
	3rd-party/coreHTTP/source/core_http_client.c \
	3rd-party/coreHTTP/source/dependency/3rdparty/llhttp/src/api.c \
	3rd-party/coreHTTP/source/dependency/3rdparty/llhttp/src/llhttp.c \
	3rd-party/coreHTTP/source/dependency/3rdparty/llhttp/src/http.c \

# coreMQTT source files
CFILES += \
	3rd-party/coreMQTT/source/core_mqtt.c \
	3rd-party/coreMQTT/source/core_mqtt_state.c \
	3rd-party/coreMQTT/source/core_mqtt_serializer.c \

CXXFILES :=

COBJS   := $(addprefix $(BUILD)/, $(CFILES:.c=.o))
CXXOBJS := $(addprefix $(BUILD)/, $(CXXFILES:.cpp=.o))

OBJS    := $(COBJS) $(CXXOBJS)
DEPS    := $(OBJS:.o=.o.d)

OBJ_DIRS := $(sort $(dir $(OBJS) $(LIB)))

# --- 3rd-party sub-library build ---
.PHONY: sub-libs sub-libs-clean
sub-libs:
	@$(MAKE) -C 3rd-party all || exit $?;

sub-libs-clean:
	@$(MAKE) -C 3rd-party clean || exit $?;

# --- main build ---
.PHONY: all
all: sub-libs $(LIB)
	@$(ECHO) [BUILD] libs 3rd-party $(DIR) done.

$(LIB): $(OBJS) | $(OBJ_DIRS)
	@echo [LD] $@
	$(Q)$(AR) rcs $@ $(OBJS)

$(COBJS) : $(BUILD)/%.o : %.c | $(OBJ_DIRS)
	@echo [CC] $<
	$(Q)$(CC) $(CFLAGS) -MD -MP -MF $@.d -c $< -o $@

$(CXXOBJS) : $(BUILD)/%.o : %.cpp | $(OBJ_DIRS)
	@echo [CXX] $<
	$(Q)$(CXX) $(CXXFLAGS) -MD -MP -MF $@.d -c $< -o $@

.PHONY: clean
clean: sub-libs-clean
	@rm -rf $(LIB) $(OBJS) $(DEPS) $(LIB_DIR)

$(OBJ_DIRS):
	@$(MKDIR) -p $@

# Header install
HEADER_DIRS :=
HEADER_FILES := src/peer.h src/peer_connection.h src/peer_signaling.h
HEADER_INSTALL := $(RTSMART_3RD_PARTY_INC_INSTALL_PATH)/libpeer

include $(SDK_RTSMART_SRC_DIR)/libs/mk/tools_install_header.mk

-include $(DEPS)

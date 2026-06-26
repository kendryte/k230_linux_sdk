include ../../../../mkenv.mk
include $(SDK_RTSMART_SRC_DIR)/libs/mk/toolchain_riscv64_musl.mk

include $(SDK_RTSMART_SRC_DIR)/libs/mk/librtsmart_hal.mk
include $(SDK_RTSMART_SRC_DIR)/libs/mk/libmpp.mk

LOCAL_DIR = $(shell pwd)
LIB_DIR = $(shell pwd)/../lib
BUILD = $(SDK_RTSMART_BUILD_DIR)/libs/3rd-party/libpeer/usrsctp/

CWARN :=

C_CXX_FLAGS := -march=rv64imafdcv -mabi=lp64d -mcmodel=medany -Os $(CWARN) $(LIB_CFLAGS)
C_CXX_FLAGS += -I$(SDK_SRC_ROOT_DIR)/include -include "generated/autoconf.h"

CFLAGS := $(C_CXX_FLAGS) -std=gnu99
CFLAGS += -D__Userspace__
CFLAGS += -DSCTP_SIMPLE_ALLOCATOR
CFLAGS += -DSCTP_PROCESS_LEVEL_LOCKS
CFLAGS += -DSCTP_DEBUG
CFLAGS += -DINET
CFLAGS += -D_GNU_SOURCE
CFLAGS += -DHAVE_STDATOMIC_H
CFLAGS += -Wno-address-of-packed-member
CFLAGS += -Wno-deprecated-declarations
CFLAGS += -Wno-unused-function
CFLAGS += -Wno-unused-parameter
CFLAGS += -Iusrsctplib

CFILES := \
	usrsctplib/netinet/sctp_asconf.c \
	usrsctplib/netinet/sctp_auth.c \
	usrsctplib/netinet/sctp_bsd_addr.c \
	usrsctplib/netinet/sctp_callout.c \
	usrsctplib/netinet/sctp_cc_functions.c \
	usrsctplib/netinet/sctp_crc32.c \
	usrsctplib/netinet/sctp_indata.c \
	usrsctplib/netinet/sctp_input.c \
	usrsctplib/netinet/sctp_output.c \
	usrsctplib/netinet/sctp_pcb.c \
	usrsctplib/netinet/sctp_peeloff.c \
	usrsctplib/netinet/sctp_sha1.c \
	usrsctplib/netinet/sctp_ss_functions.c \
	usrsctplib/netinet/sctp_sysctl.c \
	usrsctplib/netinet/sctp_timer.c \
	usrsctplib/netinet/sctp_userspace.c \
	usrsctplib/netinet/sctp_usrreq.c \
	usrsctplib/netinet/sctputil.c \
	usrsctplib/netinet6/sctp6_usrreq.c \
	usrsctplib/user_environment.c \
	usrsctplib/user_mbuf.c \
	usrsctplib/user_recv_thread.c \
	usrsctplib/user_socket.c \

COBJS   := $(addprefix $(BUILD)/, $(CFILES:.c=.o))
OBJS    := $(COBJS)
DEPS    := $(OBJS:.o=.o.d)
OBJ_DIRS := $(sort $(dir $(OBJS)) $(LIB_DIR))

.PHONY: all
all: $(LIB_DIR)/libusrsctp.a

$(LIB_DIR)/libusrsctp.a: $(OBJS) | $(OBJ_DIRS)
	@echo [LD] $@
	$(Q)$(AR) rcs $@ $(OBJS)

$(COBJS) : $(BUILD)/%.o : %.c | $(OBJ_DIRS)
	@echo [CC] $<
	$(Q)$(CC) $(CFLAGS) -MD -MP -MF $@.d -c $< -o $@

.PHONY: clean
clean:
	@rm -rf $(LIB_DIR)/libusrsctp.a $(OBJS) $(DEPS)

$(OBJ_DIRS):
	@$(MKDIR) -p $@

-include $(DEPS)

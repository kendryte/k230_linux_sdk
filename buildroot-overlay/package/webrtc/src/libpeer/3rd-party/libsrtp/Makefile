include ../../../../mkenv.mk
include $(SDK_RTSMART_SRC_DIR)/libs/mk/toolchain_riscv64_musl.mk

include $(SDK_RTSMART_SRC_DIR)/libs/mk/librtsmart_hal.mk
include $(SDK_RTSMART_SRC_DIR)/libs/mk/libmpp.mk

LOCAL_DIR = $(shell pwd)
LIB_DIR = $(shell pwd)/../lib
BUILD = $(SDK_RTSMART_BUILD_DIR)/libs/3rd-party/libpeer/libsrtp/

CWARN :=

C_CXX_FLAGS := -march=rv64imafdcv -mabi=lp64d -mcmodel=medany -Os $(CWARN) $(LIB_CFLAGS)
C_CXX_FLAGS += -I$(SDK_SRC_ROOT_DIR)/include -include "generated/autoconf.h"

CFLAGS := $(C_CXX_FLAGS) -std=gnu99
CFLAGS += -DHAVE_CONFIG_H
CFLAGS += -D_GNU_SOURCE
CFLAGS += -I.
CFLAGS += -Iinclude
CFLAGS += -Icrypto/include
CFLAGS += -I$(LOCAL_DIR)/../mbedtls/include

CFILES := \
	srtp/srtp.c \
	crypto/cipher/cipher.c \
	crypto/cipher/cipher_test_cases.c \
	crypto/cipher/null_cipher.c \
	crypto/cipher/aes_icm_mbedtls.c \
	crypto/cipher/aes_gcm_mbedtls.c \
	crypto/hash/auth.c \
	crypto/hash/auth_test_cases.c \
	crypto/hash/null_auth.c \
	crypto/hash/hmac_mbedtls.c \
	crypto/kernel/alloc.c \
	crypto/kernel/crypto_kernel.c \
	crypto/kernel/err.c \
	crypto/kernel/key.c \
	crypto/math/datatypes.c \
	crypto/replay/rdb.c \
	crypto/replay/rdbx.c \

COBJS   := $(addprefix $(BUILD)/, $(CFILES:.c=.o))
OBJS    := $(COBJS)
DEPS    := $(OBJS:.o=.o.d)
OBJ_DIRS := $(sort $(dir $(OBJS)) $(LIB_DIR))

.PHONY: all
all: $(LIB_DIR)/libsrtp2.a

$(LIB_DIR)/libsrtp2.a: $(OBJS) | $(OBJ_DIRS)
	@echo [LD] $@
	$(Q)$(AR) rcs $@ $(OBJS)

$(COBJS) : $(BUILD)/%.o : %.c | $(OBJ_DIRS)
	@echo [CC] $<
	$(Q)$(CC) $(CFLAGS) -MD -MP -MF $@.d -c $< -o $@

.PHONY: clean
clean:
	@rm -rf $(LIB_DIR)/libsrtp2.a $(OBJS) $(DEPS)

$(OBJ_DIRS):
	@$(MKDIR) -p $@

-include $(DEPS)

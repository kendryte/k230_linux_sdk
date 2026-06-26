include ../../../../mkenv.mk
include $(SDK_RTSMART_SRC_DIR)/libs/mk/toolchain_riscv64_musl.mk

include $(SDK_RTSMART_SRC_DIR)/libs/mk/librtsmart_hal.mk
include $(SDK_RTSMART_SRC_DIR)/libs/mk/libmpp.mk

LOCAL_DIR = $(shell pwd)
LIB_DIR = $(shell pwd)/../lib

CFLAGS_MBEDTLS := -march=rv64imafdcv -mabi=lp64d -mcmodel=medany -Os $(LIB_CFLAGS)
CFLAGS_MBEDTLS += -I$(LOCAL_DIR)/include

LDFLAGS_MBEDTLS := $(LIB_CFLAGS) $(LIB_LDFLAGS)

.PHONY: all
all: mbedtls_library
	@cp -rp library/libmbedtls.a $(LIB_DIR)/
	@cp -rp library/libmbedx509.a $(LIB_DIR)/
	@cp -rp library/libmbedcrypto.a $(LIB_DIR)/

.PHONY: mbedtls_library
mbedtls_library: | $(LIB_DIR)
	@$(MAKE) -C library CFLAGS='$(CFLAGS_MBEDTLS)' LDFLAGS='$(LDFLAGS_MBEDTLS)' || exit $?;

.PHONY: clean
clean:
	@$(MAKE) -C library clean || exit $?;
	@rm -rf $(LIB_DIR)/libmbedtls*.a

$(LIB_DIR):
	@$(MKDIR) -p $@

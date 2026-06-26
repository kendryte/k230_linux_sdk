include ../../../mkenv.mk
include $(SDK_RTSMART_SRC_DIR)/libs/mk/toolchain_riscv64_musl.mk

include $(SDK_RTSMART_SRC_DIR)/libs/mk/librtsmart_hal.mk
include $(SDK_RTSMART_SRC_DIR)/libs/mk/libmpp.mk

LIB_DIR = $(shell pwd)/lib

SUBDIRS := mbedtls libsrtp usrsctp

.PHONY: all clean $(SUBDIRS)

all: $(SUBDIRS)
	@$(ECHO) [BUILD] libpeer 3rd-party done.

$(SUBDIRS):
	@echo "[BUILD] libpeer 3rd-party $@"
	@$(MAKE) -C $@ all || exit $?;

clean:
	@for dir in $(SUBDIRS); do \
		echo "[CLEAN] libpeer 3rd-party $$dir"; \
		$(MAKE) -C $$dir clean || exit $?; \
	done
	@rm -rf $(LIB_DIR)

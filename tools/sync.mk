
BR_TAR_NAME=$(BR_NAME).tar.xz

BR_SITES = $(BR2_PRIMARY_SITE)  \
			https://buildroot.org/downloads/

BR_SITES_FOR_DL =  $(addprefix  -u https+, $(BR_SITES))

BR_OVERLAY_FS=$(shell find $(BR_OVERLAY_DIR) -type f -a -not -path  *u-boot-2022.10-overlay*  -a -not -path *linux-6.6.22*  -not -name .overlay_sync)
UBOOT_OVERLAY_FS=$(shell find $(BR_OVERLAY_DIR)/boot/uboot/u-boot-2022.10-overlay/  -type f )
#LINUX_OVERLAY_FS=$(shell find $(BR_OVERLAY_DIR)/linux/linux-6.6.22-overlay/  -type f )

OPENSBI_OVERLAY_FS=$(shell find $(BR_OVERLAY_DIR)/boot/opensbi/opensbi-1.4-overlay/  -type f )


$(BR_OVERLAY_FS)  $(UBOOT_OVERLAY_FS) :
	@:
$(BR_SRC_DIR)/.download_extract:
	@mkdir -p output  $(dir $(BR_SRC_DIR)) dl
	@BUILD_DIR=$(CURDIR)/output/ ./tools/download/dl-wrapper  -f $(BR_TAR_NAME) -H tools/download/buildroot.hash -o  dl/$(BR_TAR_NAME) $(BR_SITES_FOR_DL)  --
	@rm -rf $(BR_SRC_DIR)/ ; tar -xf dl/$(BR_TAR_NAME) -C $(dir $(BR_SRC_DIR))  ;   rm -rf $(BR_SRC_DIR)/package/python3;  rm -rf $(BR_SRC_DIR)/package/ffmpeg;
	@touch $@

# $(error $(BR_OVERLAY_FS))
$(BR_SRC_DIR)/.overlay_sync: $(BR_SRC_DIR)/.download_extract $(BR_OVERLAY_FS)
	rsync -a  $(BR_OVERLAY_DIR)/ $(BR_SRC_DIR)/ --exclude "*-overlay"  --exclude "linux-6.6.22"
	@# Drop renamed/removed overlay linux *.patch left behind by rsync -a (no --delete).
	@# Do not touch stock *.patch.conditional files that only exist in Buildroot.
	@# Otherwise e.g. 0025-mcm + 0026-mcm both apply and patching fails.
	@for f in $(BR_SRC_DIR)/linux/*.patch; do \
		[ -e "$$f" ] || continue; \
		bn=$$(basename "$$f"); \
		if [ ! -e "$(BR_OVERLAY_DIR)/linux/$$bn" ]; then \
			echo "sync: remove stale linux patch $$bn"; \
			rm -f "$$f"; \
		fi; \
	done
	@touch $@

output/.uboot_overlay_sync:$(UBOOT_OVERLAY_FS)
	mkdir -p $(@D);touch $@

# output/.linux_overlay_sync:$(LINUX_OVERLAY_FS)
# 	mkdir -p $(@D);touch $@

output/.opensbi_overlay_sync:$(OPENSBI_OVERLAY_FS)
	mkdir -p $(@D);touch $@

.PHONY:sync
sync :$(BR_SRC_DIR)/.overlay_sync  output/.uboot_overlay_sync  output/.opensbi_overlay_sync
	@:

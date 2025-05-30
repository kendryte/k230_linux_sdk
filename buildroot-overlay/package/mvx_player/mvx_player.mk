################################################################################
#
# mvx_player
#
################################################################################
MVX_PLAYER_LOCAL_PATH:= $(realpath $(TOPDIR))"/package/mvx_player"
MVX_PLAYER_DIR_NAME := mvx_player
MVX_PLAYER_APP_NAME := mvx

MVX_PLAYER_SITE = $(MVX_PLAYER_LOCAL_PATH)/src
MVX_PLAYER_SITE_METHOD = local

MVX_PLAYER_INSTALL_IMAGES := YES


define MVX_PLAYER_BUILD_CMDS
	$(TARGET_MAKE_ENV) CXX="$(TARGET_CXX)" $(MAKE) -C $(@D)
endef

define MVX_PLAYER_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/mvx_encoder $(TARGET_DIR)/usr/bin/mvx_encoder
	$(INSTALL) -D -m 0755 $(@D)/mvx_encoder_multi $(TARGET_DIR)/usr/bin/mvx_encoder_multi
	$(INSTALL) -D -m 0755 $(@D)/mvx_decoder $(TARGET_DIR)/usr/bin/mvx_decoder
	$(INSTALL) -D -m 0755 $(@D)/mvx_decoder_multi $(TARGET_DIR)/usr/bin/mvx_decoder_multi
endef






define MVX_PLAYER_BUILD_DEB
	$(call COPYFILE ,$(TARGET_DIR)/usr/bin/mvx_encoder,$(@D)/deb/usr/bin)
	$(call COPYFILE ,$(TARGET_DIR)/usr/bin/mvx_encoder_multi,$(@D)/deb/usr/bin)
	$(call COPYFILE ,$(TARGET_DIR)/usr/bin/mvx_decoder,$(@D)/deb/usr/bin)
	$(call COPYFILE ,$(TARGET_DIR)/usr/bin/mvx_decoder_multi,$(@D)/deb/usr/bin)
	dpkg -b  $(@D)/deb  $(BINARIES_DIR)/deb/$(call LOWERCASE, k230-$(PKG)).deb
endef

MVX_PLAYER_POST_INSTALL_TARGET_HOOKS += MVX_PLAYER_BUILD_DEB


$(eval $(generic-package))

AUDIO_REC_PLAY_SITE = $(realpath $(TOPDIR))"/package/audio_rec_play"
AUDIO_REC_PLAY_SITE_METHOD = local
AUDIO_REC_PLAY_INSTALL_STAGING = YES
AUDIO_REC_PLAY_DEPENDENCIES = speex

define AUDIO_REC_PLAY_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" AR="$(TARGET_AR)" -C $(@D)
endef

define AUDIO_REC_PLAY_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 $(@D)/audio_rec_play $(TARGET_DIR)/usr/bin/audio_rec_play
	$(INSTALL) -m 0755 $(@D)/audio.pcm $(TARGET_DIR)/usr/bin/audio.pcm
endef

$(eval $(generic-package))

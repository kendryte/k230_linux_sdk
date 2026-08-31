################################################################################
#
# isp-media-server
#
################################################################################

ISP_MEDIA_SERVER_VERSION = v0.8.1
ISP_MEDIA_SERVER_SITE = https://download.kendryte.com/k230/downloads/dl/isp-media-server
ISP_MEDIA_SERVER_SOURCE = isp_media_server_$(ISP_MEDIA_SERVER_VERSION).tar.gz
# To release a new version, package the binary and upload to the SITE:
#   mkdir isp_media_server_<ver> && cp isp_media_server isp_media_server_<ver>/
#   tar -czf isp_media_server_<ver>.tar.gz isp_media_server_<ver>/
#   upload to https://download.kendryte.com/k230/downloads/dl/isp-media-server/


define ISP_MEDIA_SERVER_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/isp_media_server $(TARGET_DIR)/usr/bin/isp_media_server
endef

$(eval $(generic-package))

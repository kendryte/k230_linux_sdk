CUR_MK_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_MK_DIR := $(dir $(CUR_MK_PATH))
CUR_MK_DIR := $(realpath $(CUR_MK_DIR))

include $(CUR_MK_DIR)/$(DIR_MAP_CONFIG_FILE)
include $(WSCFG_KCONFIG_CONFIG)

LOCAL_WSCFG_CROSS_COMPILE:=$(subst ",,$(WSCFG_CROSS_COMPILE))
LOCAL_WSCFG_EXTRA_CFLAGS:=$(subst ",,$(WSCFG_EXTRA_CFLAGS))
LOCAL_WSCFG_EXTRA_PARAMS:=$(subst ",,$(WSCFG_EXTRA_PARAMS))

# get relative path
REL_ALG_DIR := $(call calc_relpath,$(ALG_DIR),$(CUR_MK_DIR))
REL_WIFI_DEVICE_DIR := $(call calc_relpath,$(WIFI_DEVICE_DIR),$(CUR_MK_DIR))
REL_HMAC_DIR := $(call calc_relpath,$(HMAC_DIR),$(CUR_MK_DIR))
REL_FEATURE_DIR := $(call calc_relpath,$(FEATURE_DIR),$(CUR_MK_DIR))
REL_FORWARD_DIR := $(call calc_relpath,$(FORWARD_DIR),$(CUR_MK_DIR))
REL_FRW_DIR := $(call calc_relpath,$(FRW_DIR),$(CUR_MK_DIR))
REL_HAL_DIR := $(call calc_relpath,$(HAL_DIR),$(CUR_MK_DIR))
REL_MAC_DIR := $(call calc_relpath,$(MAC_DIR),$(CUR_MK_DIR))
REL_MAIN_DIR := $(call calc_relpath,$(MAIN_DIR),$(CUR_MK_DIR))
REL_OAL_DIR := $(call calc_relpath,$(OAL_DIR),$(CUR_MK_DIR))
REL_VERSION_DIR := $(call calc_relpath,$(VERSION_DIR),$(CUR_MK_DIR))
REL_WAL_DIR := $(call calc_relpath,$(WAL_DIR),$(CUR_MK_DIR))
REL_FE_DIR := $(call calc_relpath,$(FE_DIR),$(CUR_MK_DIR))
REL_COMMON_DIR := $(call calc_relpath,$(COMMON_DIR),$(CUR_MK_DIR))

KO_NAME := wifi_soc

#add alg makefile
include $(ALG_DIR)/ws73_alg_host.mk

SECURE_LIBC_PATH = $(PLATFROM_LIBC_SEC_DIR)/include

$(warning defconfig: ================$(SECURE_LIBC_PATH)=======================)
ifneq ($(KERNELRELEASE),)

#need confirm
include $(CUR_MK_DIR)/ws73_comm_defconfig
include $(CUR_MK_DIR)/ws73_wifi_defconfig
#add alg macro
COPTS +=$(WIFI_ALG_MACRO_DEFINES)
#===========================.o===========================
#===========================host.o===========================
hmac-objs := hmac_main.o hmac_user.o hmac_vap.o hmac_mgmt_bss_comm.o hmac_mgmt_classifier.o
hmac-objs += hmac_fsm.o hmac_mgmt_ap.o hmac_mgmt_sta.o hmac_encap_frame_ap.o hmac_encap_frame_sta.o hmac_encap_frame.o hmac_sme_sta.o
hmac-objs += hmac_11i.o hmac_11w.o hmac_config.o hmac_alg_config.o hmac_protection.o hmac_crypto_tkip.o hmac_chan_mgmt.o hmac_keep_alive.o
hmac-objs += hmac_smps.o hmac_opmode.o hmac_data_acq.o hmac_rx_filter.o hmac_proxy_arp.o hmac_reset.o hmac_scan.o
hmac-objs += hmac_p2p.o hmac_device.o hmac_resource.o hmac_cali_mgmt.o
hmac-objs += hmac_blockack.o hmac_ampdu_config.o hmac_dfs.o hmac_psm_ap.o hmac_arp_offload.o hmac_dhcp_offload.o hmac_rekey_offload.o hmac_tcp_opt.o hmac_stat.o
hmac-objs += hmac_isolation.o hmac_obss_ap.o hmac_obss_sta.o
hmac-objs += hmac_chr.o hmac_feature_dft.o hmac_ftm.o hmac_m2s.o hmac_dfr.o
hmac-objs += hmac_ant_switch.o  hmac_tx_mgmt.o hmac_fcs.o hmac_alg_notify.o hmac_tid.o hmac_beacon.o hmac_radar.o hmac_power.o hmac_thruput_test.o hmac_sta_pm.o
hmac-objs += hmac_wur_ap.o hmac_txopps.o hmac_psm_sta.o hmac_sniffer.o hmac_wmm.o hmac_bsle.o
hmac-objs += hmac_hook.o hmac_sigma_traffic_stat.o

hal-ram-objs := hal_chip.o hal_device_fsm.o hal_device.o hal_init.o hal_main.o hal_reg_opt.o
hal-hh503-ram-objs := hal_alg_cca.o hal_alg_rts.o hal_alg_tpc.o hal_alg_txbf.o hal_anti_intf.o hal_ce.o hal_chan_mgmt.o
hal-hh503-ram-objs += hal_edca.o hal_ftm.o hal_gp_reg.o hal_mac.o hal_mfg.o hal_phy.o hal_pm.o hal_power.o hal_psd.o hal_reset.o
hal-hh503-ram-objs += hal_rf.o hal_rx_filter.o hal_sr.o hal_tbtt.o hal_tpc.o hal_vap.o
#wow compile before mfg
ifeq ($(WIFI_WOW),y)
COPTS +=-D_PRE_WLAN_FEATURE_WOW_OFFLOAD
COPTS +=-D_PRE_WLAN_FEATURE_DYNAMIC_OFFLOAD
hmac-objs += hmac_wow.o
endif

ifeq ($(_PRE_WLAN_DFT_STAT),y)
hmac-objs += hmac_dfx.o
hal-hh503-ram-objs += hal_dft.o
endif

ifeq ($(WIFI_LITE_EXTREME),y)
COPTS +=-D_PRE_WLAN_ANDROID_CROP
COPTS +=-D_PRE_WLAN_NO_SUPPORT_INI
else
#信道评分特性宏与hmac_sta_channel_scoring.c文件编译必须同时开启或关闭
COPTS +=-D_PRE_WLAN_FEATURE_STA_CHANNEL_SCORING
hmac-objs += hmac_sta_channel_scoring.o
endif

#产测开，常发常收必开；产测关，可以根据选择决定常发常收是否要开
ifeq ($(WIFI_ALWAYS_TX),y)
COPTS +=-D_PRE_WLAN_FEATURE_ALWAYS_TX
hmac-objs += hmac_al_tx_rx.o
endif

ifeq ($(WSCFG_MFG_TEST),y)
hmac-objs += hmac_mfg.o
COPTS +=-D_PRE_WLAN_FEATURE_MFG_TEST
endif

ifeq ($(_PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN),y)
hmac-objs += hmac_traffic_classify.o
endif

ifeq ($(WIFI_DAQ),y)
hmac-objs += hmac_sample_daq.o hmac_sample_daq_phy.o
hal-hh503-ram-objs += hal_daq.o
COPTS +=-D_PRE_WLAN_FEATURE_DAQ
endif

hmac-objs := $(addprefix $(REL_HMAC_DIR)/,$(hmac-objs))

#解耦特性编译
feature-objs := hmac_frag.o
ifeq ($(WIFI_CSA),y)
COPTS +=-D_PRE_WLAN_FEATURE_CSA
feature-objs += hmac_csa_ap.o hmac_csa_sta.o
endif
ifeq ($(WIFI_SLP),y)
COPTS +=-D_PRE_WLAN_FEATURE_SLP
feature-objs += hmac_slp.o
endif
ifeq ($(WIFI_APF),y)
COPTS +=-D_PRE_WLAN_FEATURE_APF
feature-objs += hmac_apf.o
endif
ifeq ($(WIFI_MBO),y)
COPTS +=-D_PRE_WLAN_FEATURE_MBO
feature-objs += hmac_mbo.o
endif
ifeq ($(WIFI_11KVR),y)
COPTS +=-D_PRE_WLAN_FEATURE_11K
COPTS +=-D_PRE_WLAN_FEATURE_11V
COPTS +=-D_PRE_WLAN_FEATURE_11R
feature-objs += hmac_11k.o hmac_11v.o hmac_11r.o
endif
ifeq ($(WIFI_BASE_ROAM),y)
COPTS +=-D_PRE_WLAN_FEATURE_ROAM
feature-objs += hmac_roam_alg.o hmac_roam_connect.o hmac_roam_main.o
endif
ifeq ($(WIFI_AUTO_ADJUST_FREQ),y)
feature-objs += hmac_auto_adjust_freq.o
hal-hh503-ram-objs += hal_auto_adjust_freq.o
endif
ifeq ($(WIFI_SINGLE_PROXYSTA),y)
COPTS +=-D_PRE_WLAN_FEATURE_SINGLE_PROXYSTA
feature-objs += hmac_single_proxysta.o
endif
ifeq ($(WIFI_CSI),y)
COPTS +=-D_PRE_WLAN_FEATURE_CSI
feature-objs += hmac_csi.o
hal-hh503-ram-objs += hal_csi.o
endif
ifeq ($(WIFI_M2U),y)
COPTS +=-D_PRE_WLAN_FEATURE_M2U
feature-objs += hmac_m2u.o
endif
ifeq ($(WIFI_BLACKLIST),y)
feature-objs += hmac_blacklist.o
endif
ifeq ($(WIFI_WAPI),y)
feature-objs += hmac_wapi.o hmac_wapi_sms4.o hmac_wapi_wpi.o
endif
ifeq ($(WIFI_SDP),y)
feature-objs += hmac_sdp.o hmac_sdp_test.o
endif
ifeq ($(WIFI_WPS),y)
feature-objs += hmac_wps.o
endif
ifeq ($(WIFI_LATENCY_STAT),y)
COPTS +=-D_PRE_WLAN_LATENCY_STAT
feature-objs += hmac_latency_stat.o
endif
ifeq ($(WIFI_PROMISC),y)
feature-objs += hmac_promisc.o
endif
ifeq ($(WIFI_TX_AMSDU),y)
feature-objs += hmac_tx_amsdu.o
endif
ifeq ($(WIFI_UAPSD),y)
COPTS +=-D_PRE_WLAN_FEATURE_STA_UAPSD
feature-objs += hmac_uapsd_sta.o
feature-objs += hmac_uapsd.o
endif
ifeq ($(WIFI_ANT_SEL),y)
COPTS +=-D_PRE_WLAN_FEATURE_ANT_SEL
feature-objs += hmac_ant_sel.o
hal-hh503-ram-objs += hal_ant_sel.o
endif
ifeq ($(WIFI_PSD),y)
COPTS +=-D_PRE_WLAN_FEATURE_PSD
feature-objs += hmac_psd.o
endif
ifeq ($(WIFI_BTCOEX),y)
COPTS +=-D_PRE_WLAN_FEATURE_BTCOEX
feature-objs += hmac_btcoex.o hmac_btcoex_ba.o hmac_btcoex_btsta.o hmac_btcoex_m2s.o hmac_btcoex_notify.o hmac_btcoex_ps.o
hal-hh503-ram-objs += hal_coex_reg.o
endif
ifeq ($(WIFI_DNB),y)
feature-objs += hmac_dnb_sta.o
endif
ifeq ($(WIFI_BSRP_NFRP),y)
COPTS +=-D_PRE_WLAN_FEATURE_BSRP
feature-objs += hmac_bsrp_nfrp.o
endif
ifeq ($(WIFI_TWT),y)
COPTS +=-D_PRE_WLAN_FEATURE_TWT
feature-objs += hmac_twt.o
endif
ifeq ($(WIFI_SR_STA),y)
COPTS +=-D_PRE_WLAN_FEATURE_SR
feature-objs += hmac_sr_sta.o
endif
ifeq ($(WIFI_DEBUG),y)
COPTS +=-D_PRE_WIFI_DEBUG
COPTS +=-D_PRE_WLAN_CFGID_DEBUG
COPTS +=-D_PRE_WIFI_PRINTK
COPTS +=-D_PRE_WLAN_FEATURE_SPECIAL_PKT_LOG
endif
ifeq ($(WLAN_WFA_TEST),y)
COPTS +=-D_PRE_WLAN_FEATURE_WFA_CODE
endif
ifeq ($(WIFI_TCP_ACK_FILTER),y)
COPTS +=-D_PRE_WLAN_TCP_OPT
COPTS +=-D_PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
endif

feature-objs := $(addprefix $(REL_FEATURE_DIR)/,$(feature-objs))

feature-interface-objs := hmac_feature_interface.o hmac_ccpriv.o
feature-interface-objs := $(addprefix $(REL_FEATURE_DIR)/interface/,$(feature-interface-objs))

mac-objs := mac_device.o mac_frame.o mac_user.o mac_vap.o mac_regdomain.o mac_ie.o mac_resource.o mac_data.o oal_fsm.o mac_mem.o
mac-objs := $(addprefix $(REL_MAC_DIR)/,$(mac-objs))

wal-common-objs := wal_utils.o wal_common.o wal_ccpriv_common.o
wal-common-objs := $(addprefix $(REL_WAL_DIR)/common/,$(wal-common-objs))

common-objs := wlan_util_common_rom.o

ifeq ($(_PRE_WLAN_DFT_STAT),y)
common-objs += common_dft_rom.o
endif

common-objs := $(addprefix $(REL_WIFI_DEVICE_DIR)/source/common/romable/,$(common-objs))


wal-objs := wal_config.o wal_config_debug.o wal_main.o wal_linux_util.o wal_linux_scan.o wal_linux_rx_rsp.o wal_regdb.o wal_config_acs.o
wal-objs += wal_linux_ioctl.o wal_linux_ioctl_debug.o wal_linux_cfg80211.o wal_linux_cfgvendor.o
wal-objs += wal_linux_11d.o wal_linux_iwhandler.o wal_linux_ccpriv.o wal_linux_netdev.o wal_linux_vap_proc.o
wal-objs += wal_linux_witp_cmd.o wal_config_sdp.o wal_linux_psd.o wal_linux_dfx.o wal_linux_iwlist.o
ifneq ($(_PRE_WLAN_ANDROID_CROP), y)
wal-objs += wal_linux_android_cmd.o
endif
wal-objs := $(addprefix $(REL_WAL_DIR)/release/linux/,$(wal-objs))

frw-objs := frw_main.o frw_timer.o frw_util_notifier.o
frw-objs += frw_hmac.o frw_hmac_adapt.o frw_thread.o frw_common.o  frw_hcc_adapt.o
frw-objs := $(addprefix $(REL_FRW_DIR)/,$(frw-objs))

forward-objs := hmac_tx_data.o hmac_tx_encap.o hmac_tx_mpdu_adapt.o hmac_tx_mpdu_queue.o hmac_rx_data_event.o hmac_rx_data_filter.o hmac_rx_data.o hmac_rx_data_feature.o
forward-objs := $(addprefix $(REL_FORWARD_DIR)/,$(forward-objs))
netbuf-objs := oal_netbuf.o
netbuf-objs := $(addprefix $(REL_FORWARD_DIR)/linux/,$(netbuf-objs))

alg-objs := $(addprefix $(REL_ALG_DIR)/,$(patsubst %.c,%.o,$(WIFI_ALG_SRC_LIST)))

main-objs := main.o
main-objs := $(addprefix $(REL_MAIN_DIR)/linux/,$(main-objs))

oal-objs := oal_net.o oal_cfg80211.o
oal-objs := $(addprefix $(REL_OAL_DIR)/linux/,$(oal-objs))

version-objs := hmac_version.o
version-objs := $(addprefix $(REL_VERSION_DIR)/,$(version-objs))

hal-ram-objs := $(addprefix $(REL_HAL_DIR)/,$(hal-ram-objs))
hal-hh503-ram-objs := $(addprefix $(REL_HAL_DIR)/hh503/,$(hal-hh503-ram-objs))
hal-rom-objs := $(addprefix $(REL_HAL_DIR)/romable/,$(notdir $(patsubst %.c,%.o,$(wildcard $(HAL_DIR)/romable/*.c))))
hal-hh503-rom-objs := $(addprefix $(REL_HAL_DIR)/romable/hh503/,$(notdir $(patsubst %.c,%.o,$(wildcard $(HAL_DIR)/romable/hh503/*.c))))

customize-objs := customize_wifi.o
customize-objs := $(addprefix $(REL_COMMON_DIR)/,$(customize-objs))

cali-online-objs := $(addprefix $(REL_FE_DIR)/,spec/ws73/power_ppa_ctrl_spec.o)
cali-online-objs += $(addprefix $(REL_FE_DIR)/,spec/ws73/power_ctrl_spec.o)
cali-online-objs += $(addprefix $(REL_FE_DIR)/,power_ctrl/fe_init_pow.o)
ifeq ($(WIFI_ONLINE_CALI), y)
COPTS +=-D_PRE_WLAN_ONLINE_CALI
cali-online-objs += $(addprefix $(REL_FE_DIR)/,calibrate/online_cali/cali_online.o)
cali-online-objs += $(addprefix $(REL_FE_DIR)/,calibrate/online_cali/cali_online_tx_pwr.o)
cali-online-objs += $(addprefix $(REL_FE_DIR)/,calibrate/online_cali/fe_log_calc_rom.o)
cali-online-objs += $(addprefix $(REL_FE_DIR)/,calibrate/online_cali/cali_online_common.o)
endif
wifi-fe-pwr-ctrl-objs := fe_tpc_rate_pow.o
wifi-fe-pwr-ctrl-objs := $(addprefix $(REL_FE_DIR)/power_ctrl/,$(wifi-fe-pwr-ctrl-objs))
wifi-fe-cunstomize-objs := fe_rf_customize_rx_insert_loss.o fe_rf_customize_power.o fe_rf_customize_power_cali.o
wifi-fe-cunstomize-objs := $(addprefix $(REL_FE_DIR)/rf_customize/,$(wifi-fe-cunstomize-objs))
ifeq ($(WSCFG_MFG_TEST),y)
wifi-fe-equipment-objs := fe_equipment.o
wifi-fe-equipment-objs := $(addprefix $(REL_FE_DIR)/equipment/,$(wifi-fe-equipment-objs))
endif
rf-temp-code-objs := $(addprefix $(REL_FE_DIR)/,calibrate/common_utils/cali_rf_temp_code.o)
ifeq ($(WIFI_DAQ),y)
wifi-fe-hal-objs := $(addprefix $(REL_FE_DIR)/,fe_hal/phy/ws73/fe_hal_phy_daq.o)
endif
wifi-fe-hal-objs += $(addprefix $(REL_FE_DIR)/,fe_hal/phy/ws73/fe_hal_phy_if_host.o)
wifi-fe-hal-objs += $(addprefix $(REL_FE_DIR)/,fe_hal/phy/ws73/fe_hal_phy_reg_if_host.o)
wifi-fe-hal-objs += $(addprefix $(REL_FE_DIR)/,fe_hal/phy/ws73/fe_power_host.o)
ifeq ($(BOARD_ASIC),y)
wifi-fe-hal-objs += $(addprefix $(REL_FE_DIR)/,fe_hal/rf/ws73/fe_hal_gp_if.o)
wifi-fe-hal-objs += $(addprefix $(REL_FE_DIR)/,fe_hal/rf/ws73/fe_hal_rf_if_temperate.o)
wifi-fe-hal-objs += $(addprefix $(REL_FE_DIR)/,fe_hal/rf/ws73/fe_hal_rf_reg_if_temperate.o)
wifi-fe-hal-objs += $(addprefix $(REL_FE_DIR)/,fe_hal/rf/ws73/fe_hal_equipment.o)
else
wifi-fe-hal-objs += $(addprefix $(REL_FE_DIR)/,fe_hal/rf/mpw/fe_hal_gp_if.o)
wifi-fe-hal-objs += $(addprefix $(REL_FE_DIR)/,fe_hal/rf/mpw/fe_hal_rf_if_temperate.o)
wifi-fe-hal-objs += $(addprefix $(REL_FE_DIR)/,fe_hal/rf/mpw/fe_hal_equipment.o)
endif
COPTS +=-DCONFIG_PLAT_TRNG_TRIG_RPT
#===============================ko============================
#==================================host======================
$(KO_NAME)-objs := $(wal-objs)
$(KO_NAME)-objs += $(wal-common-objs)
$(KO_NAME)-objs += $(feature-objs)
$(KO_NAME)-objs += $(feature-interface-objs)
$(KO_NAME)-objs += $(common-objs)
$(KO_NAME)-objs += $(main-objs)
$(KO_NAME)-objs += $(oal-objs)
$(KO_NAME)-objs += $(hmac-objs)
$(KO_NAME)-objs += $(mac-objs)
$(KO_NAME)-objs += $(frw-objs)
$(KO_NAME)-objs += $(version-objs)
$(KO_NAME)-objs += $(forward-objs)
$(KO_NAME)-objs += $(netbuf-objs)
$(KO_NAME)-objs += $(alg-objs)
$(KO_NAME)-objs += $(hal-ram-objs)
$(KO_NAME)-objs += $(hal-hh503-ram-objs)
$(KO_NAME)-objs += $(hal-rom-objs)
$(KO_NAME)-objs += $(hal-hh503-rom-objs)
$(KO_NAME)-objs += $(wifi-fe-pwr-ctrl-objs)
$(KO_NAME)-objs += $(wifi-fe-cunstomize-objs)
ifeq ($(WSCFG_MFG_TEST),y)
$(KO_NAME)-objs += $(wifi-fe-equipment-objs)
endif
$(KO_NAME)-objs += $(cali-online-objs)
$(KO_NAME)-objs += $(customize-objs)
$(KO_NAME)-objs += $(rf-temp-code-objs)
$(KO_NAME)-objs += $(wifi-fe-hal-objs)

# 替换路径中的//为/（Android编译中可能无法识别//）
$(KO_NAME)-objs := $(subst //,/,$($(KO_NAME)-objs))
# 按文件名排序
$(KO_NAME)-objs := $(shell echo $($(KO_NAME)-objs)|python3 -c "import os;files=input().strip().split();files.sort(key=lambda x:os.path.basename(x));print(' '.join(files))")
#==================================hdevice====================

#=====================================================
#=====================================================
obj-m += $(KO_NAME).o

#====================include==============================
#====================include:host================================
_INCLUDES := -I$(_TOP_DIR)/tmpdir
_INCLUDES += -I$(INC_DIR)

_INCLUDES += -I$(WIFI_DEVICE_DIR)/sample/ws73
_INCLUDES += -I$(WIFI_DEVICE_DIR)/source/common
_INCLUDES += -I$(WIFI_DEVICE_DIR)/source/inc/romable/hal/ws73
_INCLUDES += -I$(WIFI_DEVICE_DIR)/source/inc/romable/hal/ws73/spec
_INCLUDES += -I$(WIFI_DEVICE_DIR)/source/inc/romable
_INCLUDES += -I$(WIFI_DEVICE_DIR)/source/inc/romable/device_common
_INCLUDES += -I$(PLATFORM_DEVICE_DIR)/romable/include
_INCLUDES += -I$(PLATFORM_DEVICE_DIR)/romable/include/soc
_INCLUDES += -I$(WIFI_DEVICE_DIR)/source/inc/romable/alg
_INCLUDES += -I$(WIFI_DEVICE_DIR)/source/inc
#host need include source/inc/romable/msg
_INCLUDES += -I$(WIFI_DEVICE_DIR)/source/inc/romable/msg

_INCLUDES += -I$(FE_DIR)/calibrate/online_cali
_INCLUDES += -I$(FE_DIR)/calibrate/outer_if
_INCLUDES += -I$(FE_DIR)/fe_extern
_INCLUDES += -I$(FE_DIR)/fe_hal/phy/ws73
_INCLUDES += -I$(FE_DIR)/fe_hal/phy
_INCLUDES += -I$(FE_DIR)/fe_hal/rf
_INCLUDES += -I$(FE_DIR)/power_ctrl
_INCLUDES += -I$(FE_DIR)/rf_customize
_INCLUDES += -I$(FE_DIR)/equipment
_INCLUDES += -I$(FE_DIR)/spec/ws73
_INCLUDES += -I$(FE_DIR)/calibrate/common_utils

_INCLUDES += -I$(INC_DIR)
_INCLUDES += -I$(INC_DIR)/oal
_INCLUDES += -I$(INC_DIR)/oal/linux
_INCLUDES += -I$(INC_DIR)/mac
_INCLUDES += -I$(INC_DIR)/frw
_INCLUDES += -I$(INC_DIR)/hal
_INCLUDES += -I$(INC_DIR)/linux
_INCLUDES += -I$(FEATURE_DIR)/interface

_INCLUDES += -I$(HAL_DIR)/
_INCLUDES += -I$(HAL_DIR)/spec
_INCLUDES += -I$(HAL_DIR)/hh503

_INCLUDES += -I$(INC_DIR)/hmac
_INCLUDES += -I$(INC_DIR)/wal
_INCLUDES += -I$(INC_DIR)frw
_INCLUDES += -I$(HMAC_DIR)
_INCLUDES += -I$(WAL_DIR)/common
_INCLUDES += -I$(WAL_DIR)/release/linux
_INCLUDES += -I$(FRW_DIR)
_INCLUDES += -I$(FEATURE_DIR)
_INCLUDES += -I$(VERSION_DIR)

_INCLUDES += -I$(PLATFROM_OSAL_DIR)/include
_INCLUDES += -I$(PLATFROM_DFX_DIR)/include
_INCLUDES += -I$(PLATFROM_DFX_DIR)/zdiag/include
_INCLUDES += -I$(SECURE_LIBC_PATH)

_INCLUDES += -I$(WSCFG_KERNEL_DIR)

_INCLUDES += -I$(COMMON_DIR)
_INCLUDES += -I$(INC_DIR)/dmac
_INCLUDES += -I$(INC_DIR)/hal
_INCLUDES += -I$(INC_DIR)/oal
_INCLUDES += -I$(INC_DIR)/oal/linux

_INCLUDES += -I$(HAL_DIR)/hal_verify/
_INCLUDES += -I$(HMAC_DIR)/hmac_verify/

_INCLUDES += -I$(MAC_DIR)
_INCLUDES += -I$(FORWARD_DIR)
_INCLUDES += -I$(FORWARD_DIR)/linux

_INCLUDES += -I$(PLATFROM_OSAL_DIR)/include
_INCLUDES += -I$(PLATFROM_HCC_DIR)/inc
_INCLUDES += -I$(PLATFROM_HCC_DIR)/comm
_INCLUDES += -I$(PLATFROM_HCC_DIR)/host
_INCLUDES += -I$(PLATFROM_PM_DIR)

#====================include:hdevice================================
_INCLUDES += -I$(FRW_DIR)
#add alg
_INCLUDES += $(addprefix -I$(ALG_DIR)/,$(WIFI_ALG_INC_LIST))
#add for host start
_INCLUDES += -I$(INC_DIR)/hmac
_INCLUDES += -I$(INC_DIR)/wal
_INCLUDES += -I$(WAL_DIR)/common
_INCLUDES += -I$(WAL_DIR)/release/linux
#add for host end

_INCLUDES += -I$(INC_DIR)/frw
_INCLUDES += -I$(INC_DIR)/wifi_oneimage

_INCLUDES += -I$(INC_DIR)/oal
_INCLUDES += -I$(PLATFROM_DRV_DIR)
_INCLUDES += -I$(PLATFROM_DRV_DIR)/include
_INCLUDES += -I$(PLATFROM_HCC_DIR)/slave
_INCLUDES += -I$(PLATFROM_HCC_DIR)/cfg
_INCLUDES += -I$(PLATFROM_ZDIAG_DIR)/zdiag_sample
_INCLUDES += -I$(PLATFROM_ZDIAG_ADAPT_DIR)
_INCLUDES += -I$(PLATFORM_INC_DIR)/osal_adapt
_INCLUDES += -I$(PLATFROM_DRV_DIR)/host_efuse
ifeq ($(_PRE_PLAT_SHA256SUM_CHECK),y)
_INCLUDES += -I$(PLATFROM_DRV_DIR)/crypto
endif
#_INCLUDES += -I$(SECURE_LIBC_PATH)
#This is not good
#_INCLUDES += -I$(WSCFG_KERNEL_DIR)
#===========================================================
ifneq ($(TARGET_BUILD_VARIANT),user)
KBUILD_CFLAGS += -DPLATFORM_DEBUG_ENABLE
KBUILD_CFLAGS += -DPLATFORM_SSI_FULL_LOG
endif

EXTRA_CFLAGS = $(_INCLUDES) $(COPTS)
EXTRA_CFLAGS += $(COPTS)
EXTRA_CFLAGS += $(COMM_COPTS)
EXTRA_CFLAGS += -Werror
EXTRA_CFLAGS += -fno-pic -Os
EXTRA_CFLAGS += -DDMAC_ON_HOST
MODFLAGS = -fno-pic


ifeq ($(WSCFG_BUS_USB),y)
EXTRA_CFLAGS +=-D_PRE_FEATURE_USB_ID_TABLE
endif

EXTRA_CFLAGS +=-include $(WSCFG_AUTOCONFIG_H) -include $(PLATFORM_INC_DIR)/oneimage/oneimage.h
EXTRA_CFLAGS +=-include $(INC_DIR)/wifi_oneimage/wifi_oneimage.h
EXTRA_CFLAGS +=-I$(CUR_MK_DIR)/../../../linux-7d4e1f444f461dbe3833bd99a4640e7b6c2cd529/include/linux/ -Wno-type-limits -Wno-error -Wno-discarded-qualifiers -Wno-implicit-fallthrough -Wno-return-type -Wno-implicit-function-declaration
EXTRA_CFLAGS += ${LOCAL_WSCFG_EXTRA_CFLAGS}

CONFIG_WSXX_KERNEL_MODULES_BUILD_SUPPORT=yes

### Compiler Differences ###
ifeq ($(WSCFG_USING_GCC), y)
EXTRA_CFLAGS += -Wclobbered -Wmissing-parameter-type -Wold-style-declaration -Woverride-init
else ifeq ($(WSCFG_USING_LLVM_CLANG), y)
EXTRA_CFLAGS += -mcmodel=large
EXTRA_CFLAGS += -Wcast-function-type
endif

else
#############################
###### normal makefile ######
#############################
ifeq ($(WSCFG_USING_GCC), y)

default:
	$(MAKE) -C $(WSCFG_KERNEL_DIR) ARCH=$(WSCFG_ARCH_NAME) CROSS_COMPILE=$(LOCAL_WSCFG_CROSS_COMPILE) M=$(CUR_MK_DIR) modules \
		KBUILD_MODPOST_WARN=1 ${LOCAL_WSCFG_EXTRA_PARAMS} ${EXTRA_CFLAGS} 
	$(LOCAL_WSCFG_CROSS_COMPILE)strip --strip-unneeded $(KO_NAME).ko

else ifeq ($(WSCFG_USING_LLVM_CLANG), y)

LOCAL_CLANG_PATH:=$(subst ",,$(WSCFG_CLANG_PATH))

CC=$(LOCAL_CLANG_PATH)/clang
LD=$(LOCAL_CLANG_PATH)/ld.lld
NM=$(LOCAL_CLANG_PATH)/llvm-nm
AR=$(LOCAL_CLANG_PATH)/llvm-ar
OBJCOPY=$(LOCAL_CLANG_PATH)/llvm-objcopy
STRIP=$(LOCAL_CLANG_PATH)/llvm-strip

default:
	$(MAKE) -C $(WSCFG_KERNEL_DIR) ARCH=${WSCFG_ARCH_NAME} CROSS_COMPILE=${LOCAL_WSCFG_CROSS_COMPILE} CC=$(CC) NM=$(NM) AR=$(AR) LD=$(LD) \
		OBJCOPY=$(OBJCOPY) M=$(CUR_MK_DIR) modules -j KBUILD_MODPOST_WARN=1 ${LOCAL_WSCFG_EXTRA_PARAMS}
	$(STRIP) --strip-unneeded $(KO_NAME).ko
endif

clean:
	$(MAKE) -C $(WSCFG_KERNEL_DIR) ARCH=${WSCFG_ARCH_NAME} M=$(CUR_MK_DIR) clean

endif
# end of ifneq ($(KERNELRELEASE),)
#===========================================================

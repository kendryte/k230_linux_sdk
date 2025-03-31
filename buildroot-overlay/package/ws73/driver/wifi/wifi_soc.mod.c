#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

KSYMTAB_FUNC(hmac_config_11i_remove_key_etc, "", "");
KSYMTAB_FUNC(hmac_config_11i_get_key_etc, "", "");
KSYMTAB_FUNC(hmac_config_11i_add_key_etc, "", "");
KSYMTAB_FUNC(hmac_config_11i_add_wep_entry_etc, "", "");
KSYMTAB_FUNC(hmac_m2u_snoop_inspecting_ws73, "", "");
KSYMTAB_FUNC(hmac_m2u_attach_ws73, "", "");
KSYMTAB_FUNC(hmac_m2u_snoop_convert_ws73, "", "");
KSYMTAB_FUNC(hmac_m2u_detach_ws73, "", "");
KSYMTAB_FUNC(hmac_m2u_clear_deny_table_ws73, "", "");
KSYMTAB_FUNC(hmac_m2u_print_all_snoop_list_ws73, "", "");
KSYMTAB_FUNC(hmac_m2u_show_snoop_deny_table_ws73, "", "");
KSYMTAB_FUNC(hmac_m2u_add_snoop_ipv4_deny_entry_ws73, "", "");
KSYMTAB_FUNC(hmac_m2u_add_snoop_ipv6_deny_entry_ws73, "", "");
KSYMTAB_FUNC(hmac_m2u_del_ipv4_deny_entry_ws73, "", "");
KSYMTAB_FUNC(hmac_m2u_del_ipv6_deny_entry_ws73, "", "");
KSYMTAB_FUNC(hmac_m2u_cleanup_snoopwds_node_ws73, "", "");
KSYMTAB_FUNC(hmac_m2u_unicast_convert_multicast_ws73, "", "");
KSYMTAB_FUNC(hmac_tx_process_mgmt_event, "", "");
KSYMTAB_FUNC(hmac_check_capability_mac_phy_supplicant_etc, "", "");
KSYMTAB_FUNC(hmac_get_p2p_status_etc, "", "");
KSYMTAB_FUNC(hmac_set_p2p_status_etc, "", "");
KSYMTAB_FUNC(hmac_tx_traffic_classify_etc, "", "");
KSYMTAB_FUNC(hmac_tx_lan_to_wlan_etc, "", "");
KSYMTAB_FUNC(hmac_free_netbuf_list_etc, "", "");
KSYMTAB_FUNC(hmac_tx_report_eth_frame_etc, "", "");
KSYMTAB_FUNC(hmac_bridge_vap_xmit_etc, "", "");
KSYMTAB_FUNC(hmac_vap_get_net_device_etc, "", "");
KSYMTAB_FUNC(hmac_vap_get_desired_country_etc, "", "");
KSYMTAB_FUNC(hmac_vap_destroy_etc, "", "");
KSYMTAB_FUNC(hmac_vap_get_updata_rd_by_ie_switch_etc, "", "");
KSYMTAB_FUNC(hmac_res_get_mac_dev_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_put_bss_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_get_bss_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_inform_bss_frame_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_scan_done_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_connect_result_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_roamed_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_ft_event_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_disconnected_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_new_sta_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_mic_failure_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_del_sta_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_rx_mgmt_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_mgmt_tx_status_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_ready_on_channel_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_remain_on_channel_expired_etc, "", "");
KSYMTAB_FUNC(oal_kobject_uevent_env_sta_join_etc, "", "");
KSYMTAB_FUNC(oal_kobject_uevent_env_sta_leave_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_vendor_cmd_alloc_reply_skb_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_vendor_cmd_reply_etc, "", "");
KSYMTAB_FUNC(oal_cfg80211_m2s_status_report, "", "");
KSYMTAB_FUNC(oal_netbuf_is_tcp_ack6_etc, "", "");
KSYMTAB_FUNC(oal_netbuf_select_queue_etc, "", "");
KSYMTAB_FUNC(oal_netbuf_is_dhcp_port_etc, "", "");
KSYMTAB_FUNC(oal_netbuf_is_dhcp6_etc, "", "");
KSYMTAB_FUNC(oal_netbuf_is_tcp_ack_etc, "", "");
KSYMTAB_FUNC(oal_netbuf_is_icmp_etc, "", "");
KSYMTAB_FUNC(uapi_ccpriv_send_random_mac_oui, "", "");
KSYMTAB_FUNC(uapi_ccpriv_kick_user, "", "");
KSYMTAB_FUNC(uapi_ccpriv_get_nvram_power, "", "");
KSYMTAB_FUNC(uapi_ccpriv_set_scan_param, "", "");
KSYMTAB_FUNC(uapi_ccpriv_set_ampdu_amsdu, "", "");
KSYMTAB_FUNC(uapi_ccpriv_set_keepalive_mode, "", "");
KSYMTAB_FUNC(uapi_ccpriv_common_debug, "", "");
KSYMTAB_FUNC(uapi_ccpriv_set_ap_user_aging_time, "", "");
KSYMTAB_FUNC(uapi_ccpriv_dhcp_debug, "", "");
KSYMTAB_FUNC(uapi_ccpriv_set_smooth_phase_en, "", "");
KSYMTAB_FUNC(uapi_ccpriv_multi_bssid_enable, "", "");
KSYMTAB_FUNC(uapi_ccpriv_set_2040_coext_support, "", "");
KSYMTAB_FUNC(uapi_ccpriv_set_sts_sung_mung, "", "");
KSYMTAB_FUNC(uapi_ccpriv_rssi_limit, "", "");
KSYMTAB_FUNC(wal_get_user_by_mac, "", "");
KSYMTAB_FUNC(wal_handel_radio_meas_rpt_cmd, "", "");
KSYMTAB_FUNC(wal_sync_post2hmac_no_rsp, "", "");
KSYMTAB_FUNC(wal_sync_send2device_no_rsp, "", "");
KSYMTAB_FUNC(wal_async_send2device_no_rsp, "", "");
KSYMTAB_FUNC(wal_register_debug_cmd, "", "");
KSYMTAB_FUNC(wal_unregister_debug_cmd, "", "");
KSYMTAB_FUNC(wal_ccpriv_get_mac_addr, "", "");
KSYMTAB_FUNC(uapi_ccpriv_addba_req, "", "");
KSYMTAB_FUNC(uapi_ccpriv_delba_req, "", "");
KSYMTAB_FUNC(uapi_ccpriv_beacon_chain_switch, "", "");
KSYMTAB_FUNC(uapi_ccpriv_ampdu_tx_on, "", "");
KSYMTAB_FUNC(uapi_ccpriv_packet_xmit, "", "");
KSYMTAB_FUNC(uapi_ccpriv_wmm_switch, "", "");
KSYMTAB_FUNC(uapi_ccpriv_set_sleep, "", "");
KSYMTAB_FUNC(uapi_ccpriv_show_dhcpoffload_info, "", "");
KSYMTAB_FUNC(uapi_ccpriv_clear_wow_offload_info, "", "");
KSYMTAB_FUNC(uapi_ccpriv_single_proxysta_debug, "", "");
KSYMTAB_FUNC(uapi_ccpriv_set_ru_allocation, "", "");
KSYMTAB_FUNC(uapi_ccpriv_set_11ax_softap_para, "", "");
KSYMTAB_FUNC(uapi_ccpriv_cw_cfg, "", "");
KSYMTAB_FUNC(uapi_ccpriv_aifsn_cfg, "", "");
KSYMTAB_FUNC(uapi_ccpriv_get_vap_stat, "", "");
KSYMTAB_FUNC(uapi_ccpriv_get_vap_aggre_info, "", "");
KSYMTAB_FUNC(uapi_ccpriv_get_hw_stat, "", "");
KSYMTAB_FUNC(uapi_ccpriv_hw_stat_enable, "", "");
KSYMTAB_FUNC(uapi_ccpriv_get_vap_stat_new, "", "");
KSYMTAB_FUNC(uapi_ccpriv_vap_stat_enable, "", "");
KSYMTAB_FUNC(wal_bridge_vap_xmit_etc, "", "");
KSYMTAB_FUNC(wal_main_init_etc, "", "");
KSYMTAB_FUNC(wal_main_exit_etc, "", "");

MODULE_INFO(depends, "");


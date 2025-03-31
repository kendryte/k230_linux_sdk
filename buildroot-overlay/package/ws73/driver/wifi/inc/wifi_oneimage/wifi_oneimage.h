/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: wifi oneimage file
 * Create: 2023-06-27
 */

#ifndef __WIFI_ONEIMAGE_H__
#define __WIFI_ONEIMAGE_H__

#ifdef WSCFG_ONEIMAGE
#define wifi_oneimage_rename(NAME)  NAME##_ws73

#define hmac_m2u_snoop_inspecting           wifi_oneimage_rename(hmac_m2u_snoop_inspecting)
#define hmac_m2u_attach                     wifi_oneimage_rename(hmac_m2u_attach)
#define hmac_m2u_snoop_convert              wifi_oneimage_rename(hmac_m2u_snoop_convert)
#define hmac_m2u_detach                     wifi_oneimage_rename(hmac_m2u_detach)
#define hmac_m2u_clear_deny_table           wifi_oneimage_rename(hmac_m2u_clear_deny_table)
#define hmac_m2u_print_all_snoop_list       wifi_oneimage_rename(hmac_m2u_print_all_snoop_list)
#define hmac_m2u_show_snoop_deny_table      wifi_oneimage_rename(hmac_m2u_show_snoop_deny_table)
#define hmac_m2u_add_snoop_ipv4_deny_entry  wifi_oneimage_rename(hmac_m2u_add_snoop_ipv4_deny_entry)
#define hmac_m2u_add_snoop_ipv6_deny_entry  wifi_oneimage_rename(hmac_m2u_add_snoop_ipv6_deny_entry)
#define hmac_m2u_del_ipv4_deny_entry        wifi_oneimage_rename(hmac_m2u_del_ipv4_deny_entry)
#define hmac_m2u_del_ipv6_deny_entry        wifi_oneimage_rename(hmac_m2u_del_ipv6_deny_entry)
#define hmac_m2u_cleanup_snoopwds_node      wifi_oneimage_rename(hmac_m2u_cleanup_snoopwds_node)
#define hmac_m2u_unicast_convert_multicast  wifi_oneimage_rename(hmac_m2u_unicast_convert_multicast)

#endif

#endif /* #ifndef __WIFI_ONEIMAGE_H__ */

/*
 * Driver interaction with Linux nl80211/cfg80211 - Android specific
 * Copyright (c) 2002-2014, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2007, Johannes Berg <johannes@sipsolutions.net>
 * Copyright (c) 2009-2010, Atheros Communications
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "includes.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <fcntl.h>

#include "utils/common.h"
#include "driver_nl80211.h"
#include "android_drv.h"
#ifdef CONNECTIVITY_SET_P2P_IE_PATCH
#include "wpa_supplicant_i.h"
#endif

typedef struct android_wifi_priv_cmd {
	char *buf;
	int used_len;
	int total_len;
} android_wifi_priv_cmd;

static int drv_errors = 0;

static void wpa_driver_send_hang_msg(struct wpa_driver_nl80211_data *drv)
{
	drv_errors++;
	if (drv_errors > DRV_NUMBER_SEQUENTIAL_ERRORS) {
		drv_errors = 0;
		wpa_msg(drv->ctx, MSG_INFO, WPA_EVENT_DRIVER_STATE "HANGED");
	}
}


static int android_priv_cmd(struct i802_bss *bss, const char *cmd)
{
	struct wpa_driver_nl80211_data *drv = bss->drv;
	struct ifreq ifr;
	android_wifi_priv_cmd priv_cmd;
	char buf[MAX_DRV_CMD_SIZE];
	int ret;

	os_memset(&ifr, 0, sizeof(ifr));
	os_memset(&priv_cmd, 0, sizeof(priv_cmd));
	os_strlcpy(ifr.ifr_name, bss->ifname, IFNAMSIZ);

	os_memset(buf, 0, sizeof(buf));
	os_strlcpy(buf, cmd, sizeof(buf));

	priv_cmd.buf = buf;
	priv_cmd.used_len = sizeof(buf);
	priv_cmd.total_len = sizeof(buf);
	ifr.ifr_data = &priv_cmd;

	ret = ioctl(drv->global->ioctl_sock, SIOCDEVPRIVATE + 1, &ifr);
	if (ret < 0) {
		wpa_printf(MSG_ERROR, "%s: failed to issue private commands",
			   __func__);
		wpa_driver_send_hang_msg(drv);
		return ret;
	}

	drv_errors = 0;
	return 0;
}


int android_pno_start(struct i802_bss *bss,
		      struct wpa_driver_scan_params *params)
{
	struct wpa_driver_nl80211_data *drv = bss->drv;
	struct ifreq ifr;
	android_wifi_priv_cmd priv_cmd;
	int ret = 0, i = 0, bp;
	char buf[WEXT_PNO_MAX_COMMAND_SIZE];

	bp = WEXT_PNOSETUP_HEADER_SIZE;
	os_memcpy(buf, WEXT_PNOSETUP_HEADER, bp);
	buf[bp++] = WEXT_PNO_TLV_PREFIX;
	buf[bp++] = WEXT_PNO_TLV_VERSION;
	buf[bp++] = WEXT_PNO_TLV_SUBVERSION;
	buf[bp++] = WEXT_PNO_TLV_RESERVED;

	while (i < WEXT_PNO_AMOUNT && (size_t) i < params->num_ssids) {
		/* Check that there is enough space needed for 1 more SSID, the
		 * other sections and null termination */
		if ((bp + WEXT_PNO_SSID_HEADER_SIZE + MAX_SSID_LEN +
		     WEXT_PNO_NONSSID_SECTIONS_SIZE + 1) >= (int) sizeof(buf))
			break;
		wpa_hexdump_ascii(MSG_DEBUG, "For PNO Scan",
				  params->ssids[i].ssid,
				  params->ssids[i].ssid_len);
		buf[bp++] = WEXT_PNO_SSID_SECTION;
		buf[bp++] = params->ssids[i].ssid_len;
		os_memcpy(&buf[bp], params->ssids[i].ssid,
			  params->ssids[i].ssid_len);
		bp += params->ssids[i].ssid_len;
		i++;
	}

	buf[bp++] = WEXT_PNO_SCAN_INTERVAL_SECTION;
	os_snprintf(&buf[bp], WEXT_PNO_SCAN_INTERVAL_LENGTH + 1, "%x",
		    WEXT_PNO_SCAN_INTERVAL);
	bp += WEXT_PNO_SCAN_INTERVAL_LENGTH;

	buf[bp++] = WEXT_PNO_REPEAT_SECTION;
	os_snprintf(&buf[bp], WEXT_PNO_REPEAT_LENGTH + 1, "%x",
		    WEXT_PNO_REPEAT);
	bp += WEXT_PNO_REPEAT_LENGTH;

	buf[bp++] = WEXT_PNO_MAX_REPEAT_SECTION;
	os_snprintf(&buf[bp], WEXT_PNO_MAX_REPEAT_LENGTH + 1, "%x",
		    WEXT_PNO_MAX_REPEAT);
	bp += WEXT_PNO_MAX_REPEAT_LENGTH + 1;

	memset(&ifr, 0, sizeof(ifr));
	memset(&priv_cmd, 0, sizeof(priv_cmd));
	os_strlcpy(ifr.ifr_name, bss->ifname, IFNAMSIZ);

	priv_cmd.buf = buf;
	priv_cmd.used_len = bp;
	priv_cmd.total_len = bp;
	ifr.ifr_data = &priv_cmd;

	ret = ioctl(drv->global->ioctl_sock, SIOCDEVPRIVATE + 1, &ifr);

	if (ret < 0) {
		wpa_printf(MSG_ERROR, "ioctl[SIOCSIWPRIV] (pnosetup): %d",
			   ret);
		wpa_driver_send_hang_msg(drv);
		return ret;
	}

	drv_errors = 0;

	return android_priv_cmd(bss, "PNOFORCE 1");
}


int android_pno_stop(struct i802_bss *bss)
{
	return android_priv_cmd(bss, "PNOFORCE 0");
}


#ifdef ANDROID_P2P
#ifdef ANDROID_LIB_STUB

int wpa_driver_set_p2p_noa(void *priv, u8 count, int start, int duration)
{
	return 0;
}


int wpa_driver_get_p2p_noa(void *priv, u8 *buf, size_t len)
{
	return 0;
}


int wpa_driver_set_p2p_ps(void *priv, int legacy_ps, int opp_ps, int ctwindow)
{
	return -1;
}

#endif /* ANDROID_LIB_STUB */
#endif /* ANDROID_P2P */

#ifdef CONNECTIVITY_SET_P2P_IE_PATCH
int wpa_driver_nl80211_driver_cmd(void *priv, char *cmd, char *buf,
				  size_t buf_len)
{
	struct i802_bss *bss = priv;
	struct wpa_driver_nl80211_data *drv = bss->drv;
	struct ifreq ifr;
	android_wifi_priv_cmd priv_cmd;
	int ret = 0;

	if (bss->ifindex <= 0 && bss->wdev_id > 0) {
		/* DRIVER CMD received on the DEDICATED P2P Interface which doesn't
		 * have an NETDEVICE associated with it. So we have to re-route the
		 * command to the parent NETDEVICE
		 */
		struct wpa_supplicant *wpa_s = (struct wpa_supplicant *)(drv->ctx);

		wpa_printf(MSG_DEBUG, "Re-routing DRIVER cmd to parent iface");
		if (wpa_s && wpa_s->parent) {
			/* Update the nl80211 pointers corresponding to parent iface */
			bss = wpa_s->parent->drv_priv;
			drv = bss->drv;
			wpa_printf(MSG_DEBUG, "Re-routing command to iface: %s"
					      " cmd (%s)", bss->ifname, cmd);
		}
	}
	os_memcpy(buf, cmd, strlen(cmd) + 1);
	memset(&ifr, 0, sizeof(ifr));
	memset(&priv_cmd, 0, sizeof(priv_cmd));
	os_strlcpy(ifr.ifr_name, bss->ifname, IFNAMSIZ);
	priv_cmd.buf = buf;
	priv_cmd.used_len = buf_len;
	priv_cmd.total_len = buf_len;
	ifr.ifr_data = (void *)&priv_cmd;

	if ((ret = ioctl(drv->global->ioctl_sock, SIOCDEVPRIVATE + 1, &ifr)) < 0) {
		wpa_printf(MSG_ERROR, "%s: failed to issue private command: %s", __func__, cmd);
		wpa_driver_send_hang_msg(drv);
	} else {
		drv_errors = 0;
		ret = 0;
		if ((os_strcasecmp(cmd, "LINKSPEED") == 0) ||
			(os_strcasecmp(cmd, "RSSI") == 0) ||
			(os_strcasecmp(cmd, "GETBAND") == 0) ||
			(os_strncasecmp(cmd, "WLS_BATCHING", 12) == 0))
			ret = strlen(buf);
		wpa_printf(MSG_DEBUG, "%s %s len = %d, %zu", __func__, buf, ret, strlen(buf));
	}
	return ret;
}

int wpa_driver_set_ap_wps_p2p_ie(void *priv, const struct wpabuf *beacon,
				 const struct wpabuf *proberesp,
				 const struct wpabuf *assocresp)
{
	char *buf;
	const struct wpabuf *ap_wps_p2p_ie = NULL;
	char *_cmd = "SET_AP_WPS_P2P_IE";
	char *pbuf;
	int ret = 0;
	int i, buf_len;
	struct cmd_desc {
		int cmd;
		const struct wpabuf *src;
	} cmd_arr[] = {
		{0x1, beacon},
		{0x2, proberesp},
		{0x4, assocresp},
		{-1, NULL}
	};

	wpa_printf(MSG_DEBUG, "%s: Entry", __func__);
	for (i = 0; cmd_arr[i].cmd != -1; i++) {
		ap_wps_p2p_ie = cmd_arr[i].src;
		if (ap_wps_p2p_ie) {
			buf_len = strlen(_cmd) + 3 + wpabuf_len(ap_wps_p2p_ie);
			buf = os_zalloc(buf_len);
			if (NULL == buf) {
				wpa_printf(MSG_ERROR, "%s: Out of memory",
				__func__);
				ret = -1;
				break;
			}
		} else {
		continue;
		}
		pbuf = buf;
		pbuf += snprintf(pbuf, buf_len - wpabuf_len(ap_wps_p2p_ie),
				"%s %d",_cmd, cmd_arr[i].cmd);
		*pbuf++ = '\0';
		os_memcpy(pbuf, wpabuf_head(ap_wps_p2p_ie), wpabuf_len(ap_wps_p2p_ie));
		ret = wpa_driver_nl80211_driver_cmd(priv, buf, buf, buf_len);
		os_free(buf);
		if (ret < 0)
			break;
	}

	return ret;
}

#elif defined(ANDROID_P2P) && defined(ANDROID_LIB_STUB)
int wpa_driver_set_ap_wps_p2p_ie(void *priv, const struct wpabuf *beacon,
				 const struct wpabuf *proberesp,
				 const struct wpabuf *assocresp)
{
	return 0;
}
#endif

int android_nl_socket_set_nonblocking(struct nl_sock *handle)
{
	return fcntl(nl_socket_get_fd(handle), F_SETFL, O_NONBLOCK);
}

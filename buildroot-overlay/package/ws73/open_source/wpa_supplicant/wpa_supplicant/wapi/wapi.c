/*
 * Copyright (c) CompanyNameMagicTag. 2023-2023. All rights reserved.
 * Êñá ‰ª∂ Âêç   : wapi.c
 * ‰Ωú    ËÄÖ   : CompanyName
 * ÁîüÊàêÊó•Êúü   : 2023Âπ¥1Êúà11Êó•
 * ÂäüËÉΩÊèèËø∞   : wapi‰∏ªÊñá‰ª∂
 */



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*
 * 1 Header File Including
 */
#include "utils/includes.h"
#include "utils/common.h"
#include "utils/os.h"               /* for os_funcs etc */
#include "utils/eloop.h"
#include "utils/base64.h"
#include "wpa_supplicant_i.h"       /* for wpa_supplicant */
#include "utils/wpabuf.h"
#include "l2_packet/l2_packet.h"    /* for L2 functions   */
#include "common/wpa_common.h"      /* for wpa_ie_data */
#include "common/defs.h"
#include "common/wpa_ctrl.h"
#include "driver_i.h"
#include "driver.h"
#include "bss.h"



#include "types.h"
#include "wapi.h"
#include "wai_rxtx.h"
#include "hash.h"
#include "wai_ec.h"
#include "wai_cert.h"
#include "wai_call_back.h"
#ifdef ANDROID
#include <keystore/keystore_get.h>
#define KEYSTORE_MESSAGE_SIZE 65535
#endif
#ifdef CONFIG_ANDROID_LOG
#include <android/log.h>
#endif
#include <limits.h>

/*
 * 2 Global Variable Definition
 */
int32 wapi_event_process(struct wapi_supplicant_stru *pst_wapi,
			 conn_status_enum en_action, uint8* puc_assoc_ie,
			 int32 uc_assoc_ie_len);
static int32 wapi_psk_derivate(struct wpa_supplicant *pst_wpa,
			       struct wpa_ssid *pst_ssid);
static int32 wapi_read_cert(const char *pc_cert_file, uint8 *puc_cert_buf,
			    uint32 *pul_buf_len);

static int32 wapi_config_associate_parm(struct wpa_supplicant  *pst_wpa,
					struct wpa_bss *pst_bss,
					struct wpa_ssid *pst_ssid,
                                        struct wpa_driver_associate_params *pst_params);
int32 wapi_init_ie(struct wpa_supplicant *pst_wpa);
static int32 wapi_cert_parse(struct wpa_supplicant *pst_wpa);
void wapi_notify_wrong_msg(struct wpa_supplicant  *pst_wpa);
int wapi_deinit_dlhandle(void);

/* ¥”wapiø‚÷–ªÒ»°π¥◊”£¨π“‘⁄¥ÀΩ·ππÃÂ…˘√˜µƒ∫Ø ˝¿Ô */
struct wapi_iface_dll_cb_stru {
	void *dl_handle;
	int (*wapi_iface_funcs_init)(struct wapi_iface_funcs_stru * wapi_iface_cb);
	void (*wai_rx_packet)(void *pv_ctx, const uint8 *pauc_src_mac,
			      const uint8 *pauc_buf,uint32 ul_datalen);
	void (*wapi_cert_free_certificate)(cert_stru *pst_cert);
	int32 (*wai_cleanup_fragbuf)(struct wapi_supplicant_stru *pst_wapi);
	int32  (*wapi_cert_parse_ecprivkey)(uint8 *puc_cert_buf,
					    uint32 ul_cert_buf_len,
					    uint8 *puc_priv_key_buf,
					    uint32 *pul_priv_key_len);
	cert_stru *(*wapi_cert_parse_certificate)(uint8 *puc_cert_buf,
						  uint32 ul_cert_buf_len);
	int32 (*wapi_ecc192_verify_keygroup)(uint8 *puc_pub_key,
					     uint32 ul_pub_key_len,
					     uint8 *puc_priv_key,
					     uint32 ul_priv_key_len);
	int32 (*wapi_cert_verify_certificate)(uint8 *puc_cert_buf,
					      uint32 ul_cert_buf_len,
					      uint8 *puc_asu_pubkey,
					      uint32 ul_asu_pubkey_len);
	int32 (*wai_psk_derivation)(uint8 *pauc_password, uint32 ul_ps_len,
				    uint8 *pauc_out_bk);
	int32 (*KD_HMAC_SHA256)(const uint8  *pauc_data,
				uint32        ul_datalen,
				const uint8  *pauc_key,
				uint32        ul_keylen,
				uint8        *pauc_out,
				uint32        ul_out_len);
};
struct wapi_iface_dll_cb_stru wapi_iface_dll_cb;

/*
 * 3 Function Definition
 */
void wapi_supplicant_init(struct wpa_supplicant *pst_wpa)
{
}

void wapi_supplicant_deinit(struct wpa_supplicant *pst_wpa)
{
}

int32  wapi_init_iface(struct wpa_supplicant *pst_wpa)
{
	struct wapi_supplicant_stru *pst_wapi = NULL;
	uint32                       ul_loop;

	if (pst_wpa == NULL) {
		wpa_printf(MSG_WARNING,"wpa struct is null");
		return WAPI_FAILED;
	}

	pst_wapi  = (struct wapi_supplicant_stru *)os_zalloc(sizeof(struct wapi_supplicant_stru));
	if (pst_wapi == NULL) {
		wpa_printf(MSG_WARNING,"Malloc wapi_s memory failed");

		return WAPI_FAILED;
	}

	for (ul_loop = 0; ul_loop < 20; ul_loop++) {
		pst_wapi->pst_wapi_l2 = l2_packet_init(
                                        pst_wpa->ifname,
                                        wpa_drv_get_mac_addr(pst_wpa),
                                        ETH_TYPE_WAI,
                                        wapi_iface_dll_cb.wai_rx_packet,
                                        pst_wpa,
                                        0);

		wpa_printf(MSG_DEBUG,"Initial WAPI L2 packet");
		if (pst_wapi->pst_wapi_l2 != NULL)
			break;
		os_sleep(3, 0);
	} /* end for */

	if (pst_wapi->pst_wapi_l2 == NULL) {
		wpa_printf(MSG_WARNING,"Failed to initialize L2 initerface");
		os_free(pst_wapi);

		return WAPI_FAILED;
	}

	if (l2_packet_get_own_addr(pst_wapi->pst_wapi_l2,
				   pst_wapi->auc_own_mac) != 0) {
		wpa_printf(MSG_WARNING,"Failed to get own L2 address");
		l2_packet_deinit(pst_wapi->pst_wapi_l2);
		os_free(pst_wapi);

		return WAPI_FAILED;
	}

	pst_wapi->en_state = WAISM_INIT;
	pst_wapi->us_nextframeseq = 1;
	pst_wapi->uc_nextfragseq  = 0;
	pst_wpa->pst_wapi = pst_wapi;
	pst_wapi->pst_wpa = pst_wpa;
	pst_wapi->st_usksa.uc_uskid =0;
	pst_wapi->st_msksa.uc_mskid =0;

	wpa_printf(MSG_DEBUG,"wapi iface init sucess");

	return WAPI_SUCCESS;
}

int32 wapi_deinit_iface(struct wpa_supplicant* pst_wpa)
{
    struct wapi_supplicant_stru *pst_wapi = NULL;

    if (NULL == pst_wpa) {
        wpa_printf(MSG_WARNING,"wapi deinit iface! pst_wpa struct is null");
        return WAPI_FAILED;
    }

    if (NULL == pst_wpa->pst_wapi) {
        wpa_printf(MSG_WARNING,"wapi deinit iface!pst_wapi struct is null");
        return WAPI_FAILED;
    }

    pst_wapi = pst_wpa->pst_wapi;
    if (NULL != pst_wapi->pst_wapi_l2) {
        l2_packet_deinit(pst_wapi->pst_wapi_l2);
        pst_wapi->pst_wapi_l2 = NULL;
    }

    if (NULL != pst_wapi->pst_ae_cert) {
        wapi_iface_dll_cb.wapi_cert_free_certificate(pst_wapi->pst_ae_cert);
        pst_wapi->pst_ae_cert = NULL;
    }

    if(NULL != pst_wapi->pst_asue_cert) {
        wapi_iface_dll_cb.wapi_cert_free_certificate(pst_wapi->pst_asue_cert);
        pst_wapi->pst_asue_cert = NULL;
    }

    if(NULL != pst_wapi->pst_as_cert) {
        wapi_iface_dll_cb.wapi_cert_free_certificate(pst_wapi->pst_as_cert);
        pst_wapi->pst_as_cert = NULL;
    }

    wapi_iface_dll_cb.wai_cleanup_fragbuf(pst_wapi);

    if (NULL != pst_wapi->pst_tx_framebuf) {
        wpabuf_free(pst_wapi->pst_tx_framebuf);
        pst_wapi->pst_tx_framebuf = NULL;
    }
    wapi_deinit_dlhandle();
    os_free(pst_wapi);
    pst_wpa->pst_wapi = NULL;

    return WAPI_SUCCESS;
}

int32 wapi_parse_wapi_ie(const uint8 *pauc_wapi_ie, uint32 ul_ie_len, struct wpa_ie_data *pst_ie_data)
{
    /* TODO */
    return 0;
}

int32 wapi_supplicant_event_assoc(struct wpa_supplicant  *pst_wpa,
                                             struct wpa_bss         *pst_bss,
                                             struct wpa_ssid        *pst_ssid,
                                             struct wpa_driver_associate_params *pst_params)

{
    struct wapi_supplicant_stru *pst_wapi = NULL;
#ifdef ANDROID
    int32                        l_result;
    int32                        l_length;
    uint8_t                     *pvalue = NULL;
#endif

    if ((NULL == pst_wpa) || (NULL == pst_wpa->pst_wapi) ||
        (NULL == pst_bss) || (NULL == pst_ssid) || (NULL == pst_params)) {
        wpa_printf(MSG_WARNING, "null pointer, pst_wpa=%p, pst_bss=%p, pst_ssid=%p,pst_params=%p,",
                   pst_wpa, pst_bss, pst_ssid, pst_params);
        return WAPI_FAILED;
    }

    pst_wapi = pst_wpa->pst_wapi;

    os_memcpy(pst_wapi->auc_own_mac, pst_wpa->own_addr, ETH_ALEN);

    os_memcpy(pst_wapi->auc_addid, pst_bss->bssid, ETH_ALEN);
    os_memcpy(&pst_wapi->auc_addid[ETH_ALEN], pst_wapi->auc_own_mac, ETH_ALEN);

    if(WPA_KEY_MGMT_WAPI_PSK & (unsigned int)(pst_wpa->key_mgmt)) {
        pst_wapi->en_auth_type = AUTH_TYPE_WAPI_PSK;
		pst_params->key_mgmt_suite = KEY_MGMT_WAPI_PSK;

        if( WAPI_SUCCESS != wapi_psk_derivate(pst_wpa,pst_ssid) ) {
            wpa_printf(MSG_WARNING,"fail to derivate psk");
            wapi_notify_wrong_msg(pst_wpa);
            return WAPI_FAILED;
        }
    }
    else if(WPA_KEY_MGMT_WAPI_CERT & (unsigned int)(pst_wpa->key_mgmt)) {
#ifdef ANDROID
        pst_wapi->en_auth_type = AUTH_TYPE_WAPI_CERT;
        pst_params->key_mgmt_suite = KEY_MGMT_WAPI_CERT;
        if (pst_ssid->wapi_as_cert && strncmp("keystore://", pst_ssid->wapi_as_cert, 11) == 0) {
		    l_length = (int32)keystore_get(&pst_ssid->wapi_as_cert[11], strlen(pst_ssid->wapi_as_cert)-11, &pvalue);

    		if ( WAPI_FAILED == l_length ) {
    			wpa_printf(MSG_ERROR,"%s: Could not open %s\n", __FUNCTION__, pst_ssid->wapi_as_cert);
    			wapi_notify_wrong_msg(pst_wpa);
    			return WAPI_FAILED;
    		}

            os_memcpy(pst_wapi->auc_as_certfile,pvalue,l_length);
            pst_wapi->ul_as_certfile_len = (uint32)l_length;
            free(pvalue);
            pvalue = NULL;
        } else {
            l_result = wapi_read_cert((int8 *)pst_ssid->wapi_as_cert,
                                           pst_wapi->auc_as_certfile,
                                           &(pst_wapi->ul_as_certfile_len));
            if ( WAPI_FAILED == l_result) {
                 wpa_printf(MSG_ERROR,"ERROR wapi_read_cert,pst_ssid->wapi_as_cert: %s", pst_ssid->wapi_as_cert);
                 wapi_notify_wrong_msg(pst_wpa);
                 return WAPI_FAILED;
            }
       }

        if (pst_ssid->wapi_user_cert && strncmp("keystore://", pst_ssid->wapi_user_cert, 11) == 0) {
		    memset(pst_wapi->auc_user_certfile, 0, 2048);
		    l_length = (int32)keystore_get(&pst_ssid->wapi_user_cert[11], strlen(pst_ssid->wapi_user_cert)-11, &pvalue);

    		if ( WAPI_FAILED == l_length ) {
    			wpa_printf(MSG_ERROR, "%s: Could not open %s\n", __FUNCTION__, pst_ssid->wapi_user_cert);
                wapi_notify_wrong_msg(pst_wpa);
    			return WAPI_FAILED;
    		}

            os_memcpy(pst_wapi->auc_user_certfile,pvalue,l_length);

            pst_wapi->ul_user_certfile_len = (uint32)l_length;
            free(pvalue);
            pvalue = NULL;

        } else {
            l_result = wapi_read_cert((int8 *)pst_ssid->wapi_user_cert,
                                       pst_wapi->auc_user_certfile,
                                       &(pst_wapi->ul_user_certfile_len));
            if ( WAPI_FAILED == l_result ) {
                wpa_printf(MSG_ERROR, "ERROR wapi_read_cert,pst_ssid->wapi_user_cert: %s\n", pst_ssid->wapi_user_cert);
                wapi_notify_wrong_msg(pst_wpa);
                return WAPI_FAILED;
            }
        }
        if( WAPI_SUCCESS != wapi_cert_parse(pst_wpa)) {
            wpa_printf(MSG_WARNING,"wapi_cert_parse fail");
            wapi_notify_wrong_msg(pst_wpa);
            return WAPI_FAILED;
        }
#endif
    } else { /* no WAPI */
       pst_wpa->pst_wapi->en_auth_type= AUTH_TYPE_NONE_WAPI;
    }

    if( WAPI_SUCCESS != wapi_init_ie(pst_wpa) ) {
       wpa_printf(MSG_WARNING,"fail to init ie");
       wapi_iface_dll_cb.wapi_cert_free_certificate(pst_wapi->pst_asue_cert);
       pst_wapi->pst_asue_cert = NULL;
       wapi_iface_dll_cb.wapi_cert_free_certificate(pst_wapi->pst_as_cert);
       pst_wapi->pst_as_cert = NULL;
       return WAPI_FAILED;
    }

    if( WAPI_SUCCESS != wapi_config_associate_parm(pst_wpa,pst_bss,pst_ssid,pst_params)) {
        wpa_printf(MSG_WARNING,"fail to configurate associating param");
        wapi_iface_dll_cb.wapi_cert_free_certificate(pst_wapi->pst_asue_cert);
        pst_wapi->pst_asue_cert = NULL;
        wapi_iface_dll_cb.wapi_cert_free_certificate(pst_wapi->pst_as_cert);
        pst_wapi->pst_as_cert = NULL;
        return WAPI_FAILED;
    }

    return WAPI_SUCCESS;
}

int32 wapi_supplicant_event_disassoc(struct wpa_supplicant *pst_wpa,uint16 us_reason)
{
    struct wapi_supplicant_stru *pst_wapi = NULL;
    uint8                        auc_bssid[ETH_ALEN];

    if ((NULL == pst_wpa) || (NULL == pst_wpa->pst_wapi))
    {
        wpa_printf(MSG_WARNING,"pst_wpa or pst_wpa->pst_wapi is null");
        return WAPI_FAILED;
    }

    pst_wapi = pst_wpa->pst_wapi;

    wpa_drv_get_bssid(pst_wpa, auc_bssid);

    memcpy(pst_wapi->auc_own_mac, pst_wpa->own_addr, ETH_ALEN);
    memcpy(pst_wapi->auc_bssid,auc_bssid,ETH_ALEN);

    pst_wapi->en_state = WAISM_INIT;
    wpa_printf(MSG_DEBUG,"WAI_Msg_Input  success");

    return WAPI_SUCCESS;
}

int32 wapi_supplicant_event(struct wpa_supplicant *pst_wpa,
                                    enum wpa_event_type en_event,
                                    void *data)
{
    struct wapi_supplicant_stru *pst_wapi = NULL;
    uint8                        auc_bssid[ETH_ALEN];

    if ((NULL == pst_wpa) || (NULL == pst_wpa->pst_wapi))
    {
        wpa_printf(MSG_WARNING,"null pointer, pst_wpa=%p", pst_wpa);
        return WAPI_FAILED;
    }

    pst_wapi  =  pst_wpa->pst_wapi;

    switch (en_event) {
        case EVENT_ASSOC:
        {
            wpa_drv_get_bssid(pst_wpa, auc_bssid);
            memcpy(pst_wapi->auc_own_mac, pst_wpa->own_addr, ETH_ALEN);
            memcpy(pst_wapi->auc_bssid,auc_bssid,ETH_ALEN);

            if(pst_wapi->uc_wapi_ie_len) {
                wpa_printf(MSG_DEBUG,"start to call wapi_event_process function\n");
                wapi_event_process(pst_wapi,CONN_ASSOC, pst_wapi->auc_wapi_ie,pst_wapi->uc_wapi_ie_len);
            } else {
                wpa_printf(MSG_DEBUG,"start to call wapi_event_process function, wapi ie len is 0.\n");
            }
            break;
        }
        case EVENT_DISASSOC:
            pst_wapi->en_state = WAISM_INIT;
            break;
        default:
            wpa_printf(MSG_DEBUG,"event is unknown");
            break;
    }

   return WAPI_SUCCESS;
}

static  int32 wapi_drv_get_bssid(struct wpa_supplicant *pst_wpa, uint8 *puc_bssid)
{
	if (pst_wpa->driver->get_bssid) {
		return pst_wpa->driver->get_bssid(pst_wpa->drv_priv, puc_bssid);
	}
	return WAPI_FAILED;
}

static int32 wapi_read_cert(const char *pc_cert_file, uint8 *puc_cert_buf, uint32 *pul_buf_len)
{
    FILE *file = NULL;
    int32    l_id = 0;
    char real_path[PATH_MAX] = {0};

    if (NULL == pc_cert_file)
    {
        wpa_printf(MSG_ERROR, "Error: pc_cert_file is null certificate");
        return WAPI_FAILED;
    }

    if (strlen(pc_cert_file) > PATH_MAX || realpath(pc_cert_file, real_path) == NULL) {
        wpa_printf(MSG_ERROR, "Error: convert pc_cert_file to real path failed");
        return WAPI_FAILED;
    }

    file = fopen((int8 *)real_path, "rb");
    if (NULL == file)
    {
        wpa_printf(MSG_ERROR,"Open file: %s Error", real_path);
        perror("Open cert file Error");
        return WAPI_FAILED;
    }

    while (!feof(file))
    {
        puc_cert_buf[l_id++] = (uint8)fgetc(file);
        if (l_id >= MAX_CERT_BUFF_SIZE) {
            wpa_printf(MSG_ERROR,"cert buf too small, buf_len = %d, i = %d", MAX_CERT_BUFF_SIZE, l_id);
            fclose(file);
            return WAPI_FAILED;
        }
    }

   *pul_buf_len = (uint32)l_id;
    fclose(file);
    return WAPI_SUCCESS;

}

static int32 wapi_cert_parse(struct wpa_supplicant *pst_wpa)
{
    struct wapi_supplicant_stru *pst_wapi = NULL;
    cert_stru                   *pst_asue_cert = NULL;
    cert_stru                   *pst_as_cert = NULL;
    int32                        l_ret;
    if ((NULL == pst_wpa) || (NULL == pst_wpa->pst_wapi))
    {
        wpa_printf(MSG_WARNING,"pst_wapi is null");
        return WAPI_FAILED;
    }

    pst_wapi = pst_wpa->pst_wapi;

    l_ret = wapi_iface_dll_cb.wapi_cert_parse_ecprivkey(pst_wapi->auc_user_certfile,
                                           pst_wapi->ul_user_certfile_len,
                                           pst_wapi->auc_asue_cert_privkey,
                                           &pst_wapi->ul_asue_cert_privkey_len);
    if( WAPI_FAILED == l_ret )
    {
        wpa_printf(MSG_WARNING,"wapi_certificate_parse_privkey fail");
        return WAPI_FAILED;
    }

    pst_asue_cert = wapi_iface_dll_cb.wapi_cert_parse_certificate(pst_wapi->auc_user_certfile,
                                                    pst_wapi->ul_user_certfile_len);
    if( NULL == pst_asue_cert )
    {
        wpa_printf(MSG_WARNING,"parse wapi asue certificate fail");
        return WAPI_FAILED;
    }

    pst_wapi->pst_asue_cert = pst_asue_cert;

    pst_as_cert =wapi_iface_dll_cb.wapi_cert_parse_certificate(pst_wapi->auc_as_certfile,
                                                    pst_wapi->ul_as_certfile_len);
    if( NULL == pst_as_cert )
    {
        wpa_printf(MSG_WARNING,"parse wapi ae certificate fail");
        wapi_iface_dll_cb.wapi_cert_free_certificate(pst_asue_cert);
        pst_wapi->pst_asue_cert = NULL;
        return WAPI_FAILED;
    }

    pst_wapi->pst_as_cert = pst_as_cert;

    l_ret = wapi_iface_dll_cb.wapi_ecc192_verify_keygroup(pst_wapi->pst_asue_cert->st_subject_key.auc_value,
                                        pst_wapi->pst_asue_cert->st_subject_key.us_length,
                                        pst_wapi->auc_asue_cert_privkey,
                                        pst_wapi->ul_asue_cert_privkey_len);
    if( WAPI_FAILED == l_ret)
    {
        wpa_printf(MSG_WARNING,"wapi_ecc192_verify_keygroup fail");
        wapi_iface_dll_cb.wapi_cert_free_certificate(pst_asue_cert);
        pst_wapi->pst_asue_cert = NULL;
        wapi_iface_dll_cb.wapi_cert_free_certificate(pst_as_cert);
        pst_wapi->pst_as_cert = NULL;
        return WAPI_FAILED;
    }

    l_ret = wapi_iface_dll_cb.wapi_cert_verify_certificate(pst_wapi->auc_user_certfile,
                            pst_wapi->ul_user_certfile_len,
                            pst_wapi->pst_as_cert->st_subject_key.auc_value,
                            pst_wapi->pst_as_cert->st_subject_key.us_length);
    if( WAPI_FAILED == l_ret)
    {
        wpa_printf(MSG_WARNING,"verify ause certificate fail");
        wapi_iface_dll_cb.wapi_cert_free_certificate(pst_asue_cert);
        pst_wapi->pst_asue_cert = NULL;
        wapi_iface_dll_cb.wapi_cert_free_certificate(pst_as_cert);
        pst_wapi->pst_as_cert = NULL;

        return WAPI_FAILED;
    }

    l_ret = wapi_iface_dll_cb.wapi_cert_verify_certificate(pst_wapi->auc_as_certfile,
                            pst_wapi->ul_as_certfile_len,
                            pst_wapi->pst_as_cert->st_subject_key.auc_value,
                            pst_wapi->pst_as_cert->st_subject_key.us_length);
    if( WAPI_FAILED == l_ret)
    {
        wpa_printf(MSG_WARNING,"verify as certificate fail");
        wapi_iface_dll_cb.wapi_cert_free_certificate(pst_asue_cert);
        pst_wapi->pst_asue_cert = NULL;
        wapi_iface_dll_cb.wapi_cert_free_certificate(pst_as_cert);
        pst_wapi->pst_as_cert = NULL;

        return WAPI_FAILED;
    }

    return WAPI_SUCCESS;
}

static int32 wapi_psk_derivate(struct wpa_supplicant *pst_wpa,struct wpa_ssid *pst_ssid)
{
    struct wapi_supplicant_stru *pst_wapi = NULL;
    size_t                       l_len;
    int32                        l_ret;
    int32                        psk_len;
    uint8                        auc_buff[128];

    if ((NULL == pst_wpa) || (NULL == pst_wpa->pst_wapi))
    {
        wpa_printf(MSG_WARNING,"pst_wapi is null");
        return WAPI_FAILED;
    }

    pst_wapi = pst_wpa->pst_wapi;

    if (pst_ssid->passphrase == NULL && !pst_ssid->psk_set)
    {
        wpa_printf(MSG_ERROR," WAPI PSK passphrase %p psk_set %d\n",pst_ssid->passphrase, pst_ssid->psk_set);
        return WAPI_FAILED;
    }


    if (pst_ssid->psk_set)
    {
        pst_wapi->st_auth_psk_type.ul_key_length= (uint32)pst_ssid->psk_set;
        os_memset(pst_wapi->st_auth_psk_type.uc_key_val, 0, sizeof(pst_wapi->st_auth_psk_type.uc_key_val));
        os_memcpy(pst_wapi->st_auth_psk_type.uc_key_val, pst_ssid->psk, pst_ssid->psk_set);
    }
    else
    {
        l_len = os_strlen(pst_ssid->passphrase);
        pst_wapi->st_auth_psk_type.ul_key_length = (uint32)l_len;
        os_memset(pst_wapi->st_auth_psk_type.uc_key_val, 0, sizeof(pst_wapi->st_auth_psk_type.uc_key_val));
        os_memcpy(pst_wapi->st_auth_psk_type.uc_key_val, pst_ssid->passphrase, l_len);
    }

    os_memset(pst_wapi->uc_bk,0,WAI_BK_SIZE);
    os_memset(auc_buff,0,128);

    if ( KEY_TYPE_ASCII == pst_ssid->psk_key_type)
    {
        l_ret = wapi_iface_dll_cb.wai_psk_derivation((uint8*)pst_wapi->st_auth_psk_type.uc_key_val,
                                    pst_wapi->st_auth_psk_type.ul_key_length, pst_wapi->uc_bk);
    }
    else
    {
        if (pst_wapi->st_auth_psk_type.ul_key_length % 2)
        {
            wpa_printf(MSG_WARNING,"ul_key_length mod 2 is not zero");
            return WAPI_FAILED;
        }

        psk_len = (int32)(pst_wapi->st_auth_psk_type.ul_key_length / 2);
        if (hexstr2bin((const char *)(pst_wapi->st_auth_psk_type.uc_key_val), auc_buff, psk_len) ||
                   pst_wapi->st_auth_psk_type.uc_key_val[psk_len * 2] != '\0')
        {
            wpa_printf(MSG_ERROR, "Invalid PSK ");
            return WAPI_FAILED;
        }

        l_ret = wapi_iface_dll_cb.wai_psk_derivation(auc_buff, (uint32)psk_len,
                                   pst_wapi->uc_bk);

    }

    return l_ret;

}

static int32 wapi_config_associate_parm(struct wpa_supplicant  *pst_wpa,
                                             struct wpa_bss         *pst_bss,
                                             struct wpa_ssid        *pst_ssid,
                                             struct wpa_driver_associate_params *pst_params)
{
    uint8                       *puc_wapi_ie;
    uint8                       *puc_ie_ssid;
    enum wpa_cipher              en_cipher_pairwise;
    enum wpa_cipher              en_cipher_group;
    en_cipher_pairwise = WPA_CIPHER_NONE;
    en_cipher_group    = WPA_CIPHER_NONE;

    if(pst_bss){
        puc_ie_ssid =(uint8 *)wpa_bss_get_ie(pst_bss, 0); /* WLAN_EID_SSID */
        pst_params->ssid = puc_ie_ssid+2;
        pst_params->ssid_len=puc_ie_ssid[1];
        pst_params->bssid=pst_bss->bssid;

        puc_wapi_ie=(uint8 *)wpa_bss_get_ie(pst_bss, 68); /* WLAN_EID_WAPI */
        if(puc_wapi_ie)
        {
            pst_wpa->pst_wapi->uc_wapi_ie_len = puc_wapi_ie[1]+2;
            if(puc_wapi_ie[1])
            {
                os_memcpy(pst_wpa->pst_wapi->auc_wapi_ie,puc_wapi_ie,puc_wapi_ie[1]+2);
            }
            pst_wpa->pst_wapi->auc_wapi_ie[1] += 2;
            pst_wpa->pst_wapi->uc_wapi_ie_len += 2;
            pst_wpa->pst_wapi->auc_wapi_ie[pst_wpa->pst_wapi->uc_wapi_ie_len - 2] = 0;
            pst_wpa->pst_wapi->auc_wapi_ie[pst_wpa->pst_wapi->uc_wapi_ie_len - 1] = 0;
        }

        pst_params->freq.freq = pst_bss->freq;
    }else{
        pst_params->ssid=pst_ssid->ssid;
        pst_params->ssid_len=pst_ssid->ssid_len;
        puc_wapi_ie=NULL;
    }

    pst_params->mode = 0;                                  /* Modes of operation: Let the driver decides */
    pst_params->wpa_ie_len=(pst_wpa->pst_wapi->uc_wapi_ie_len);
    pst_params->wpa_ie=pst_wpa->pst_wapi->auc_wapi_ie;

    pst_params->pairwise_suite = en_cipher_pairwise;
    pst_params->group_suite = en_cipher_group;

    if (!os_memcmp(pst_wpa->bssid, "\x00\x00\x00\x00\x00\x00", ETH_ALEN)) {
        if(wpa_drv_associate(pst_wpa, pst_params)) {
            wpa_printf(MSG_WARNING,"wapi_supplicant_associate failed\n");
            wpas_connection_failed(pst_wpa, pst_wpa->pending_bssid);
            wpa_supplicant_set_state(pst_wpa, WPA_DISCONNECTED);
            os_memset(pst_wpa->pending_bssid, 0, ETH_ALEN);
            pst_wpa->current_bss = NULL;
            pst_wpa->current_ssid = NULL;

            return WAPI_FAILED;
        }
        wpa_supplicant_req_auth_timeout(pst_wpa, 10, 0);/* Timeout for IEEE 802.11 authentication and association */
    }

    return WAPI_SUCCESS;
}

int32 wapi_init_ie(struct wpa_supplicant *pst_wpa)
{
	struct wapi_supplicant_stru *pst_wapi = NULL;
	uint8 auc_wapi_ie[] = {
	     0x44, 0x16, 0x01, 0x00, 0x01, 0x00, 0x00, 0x14,
	     0x72, 0x01, 0x01, 0x00, 0x00, 0x14, 0x72, 0x01,
	     0x00, 0x14, 0x72, 0x01, 0x00, 0x00, 0x00, 0x00
	};/* little endian*/


    if ((NULL == pst_wpa) || (NULL == pst_wpa->pst_wapi))
    {
        wpa_printf(MSG_WARNING,"pst_wapi is null");
        return WAPI_FAILED;
    }

	pst_wapi = pst_wpa->pst_wapi;

    os_memset(pst_wapi->auc_assoc_wapi_ie, 0, sizeof(pst_wapi->auc_assoc_wapi_ie));
    pst_wapi->uc_assoc_wapi_ie_len = 0;

    if (AUTH_TYPE_NONE_WAPI == pst_wapi->en_auth_type)
    {
        wpa_printf(MSG_DEBUG,"open, needn't set wapi-ie");
        return WAPI_SUCCESS;
    }

	if (AUTH_TYPE_WAPI_PSK == pst_wapi->en_auth_type)
	{
		auc_wapi_ie[9] = 2;
	}

	os_memcpy(pst_wapi->auc_assoc_wapi_ie, auc_wapi_ie, sizeof(auc_wapi_ie));
	pst_wapi->uc_assoc_wapi_ie_len = sizeof(auc_wapi_ie);

	return WAPI_SUCCESS;
}

int32 wapi_event_process(struct wapi_supplicant_stru *pst_wapi, conn_status_enum en_action,uint8* puc_assoc_ie, int32 uc_assoc_ie_len)
{
   uint8 auc_ann_id[] = {
            0x5c, 0x36, 0x5c,0x36,0x5c,0x36,0x5c,0x36,
			0x5c,0x36,0x5c,0x36,0x5c,0x36,0x5c,0x35
			};

   if ((NULL == pst_wapi) || (NULL == puc_assoc_ie))
   {
      wpa_printf(MSG_WARNING,"param is null");
      return WAPI_FAILED;
   }

   if (CONN_ASSOC == en_action) {

         if (AUTH_TYPE_WAPI_PSK == pst_wapi->en_auth_type)
          {
             uint8 uc_bkid[WAI_BKID_SIZE] = {0};

             wapi_iface_dll_cb.KD_HMAC_SHA256(pst_wapi->auc_addid, WAI_ADDID_SIZE,
                pst_wapi->uc_bk, WAI_BK_SIZE, uc_bkid, WAI_BKID_SIZE);


             os_memcpy(pst_wapi->st_bksa.auc_bk, pst_wapi->uc_bk, WAI_BK_SIZE);
             os_memcpy(pst_wapi->st_bksa.auc_bkid, uc_bkid, WAI_BKID_SIZE);
             os_memcpy(pst_wapi->st_bksa.auc_ae_mac,pst_wapi->auc_bssid,ETH_ALEN);
             os_memcpy(pst_wapi->st_bksa.auc_asue_mac,pst_wapi->auc_own_mac,ETH_ALEN);
             /* akmp todo */
         }

         if (AUTH_TYPE_NONE_WAPI != pst_wapi->en_auth_type)
         {
            if (uc_assoc_ie_len >= (int32)sizeof(pst_wapi->auc_wapi_ie)-1)
                uc_assoc_ie_len = (int32)sizeof(pst_wapi->auc_wapi_ie)-1;

            os_memcpy(pst_wapi->auc_wapi_ie, puc_assoc_ie, uc_assoc_ie_len);
            pst_wapi->uc_wapi_ie_len = uc_assoc_ie_len;
         }


      pst_wapi->us_nextframeseq = 1;
      pst_wapi->uc_nextfragseq  = 0;
	  pst_wapi->us_txframe_seq  =0;
      memcpy(pst_wapi->st_msksa.auc_msk_ann_id, auc_ann_id, WAI_IV_LEN);
      pst_wapi->st_usksa.uc_uskid = 0;                                       /* uskid */

      pst_wapi->en_state = WAISM_ALREADY_ASSOC;

   }else if (CONN_DISASSOC == en_action){
       pst_wapi->en_state = WAISM_INIT;
   }

   return WAPI_SUCCESS;

}

static void wapi_notify_handler(void *eloop_ctx, void *timeout_ctx)
{
    struct wpa_supplicant *pst_wpa = eloop_ctx;
    wpa_msg(pst_wpa, MSG_ERROR, "WPA: pre-shared key may be incorrect");
    wpa_msg(pst_wpa, MSG_WARNING, WPA_EVENT_DISCONNECTED "- Disconnect event - remove keys");
}

void wapi_notify_wrong_msg(struct wpa_supplicant  *pst_wpa)
{
    if(NULL == pst_wpa)
    {
       wpa_printf(MSG_WARNING,"pst_wpa is null");
       return;
    }
    if(pst_wpa) {
        /* Give some time for GUI to update current network id
         * before we noitfy  wrong psk message
         */
        eloop_cancel_timeout(wapi_notify_handler, pst_wpa, NULL);
        eloop_register_timeout(0, 200000, wapi_notify_handler, pst_wpa, NULL);
    }
}

int wapi_init_dlhandle(void )
{
    const char *dlerr = NULL;

    if(wapi_iface_dll_cb.dl_handle)
    {
        wpa_printf(MSG_ERROR,"WAPI LIB is already opened");
        return 0;
    }

    dlerr = dlerror(); /* clear the last error. */

    wpa_printf(MSG_INFO,"dlopen LIBWAPIPATH is %s", LIBWAPI_PATH);
    wapi_iface_dll_cb.dl_handle = dlopen(LIBWAPI_PATH, RTLD_LAZY);

    dlerr = dlerror();
    wapi_get_func_from_dl(wapi_iface_dll_cb.dl_handle, wapi_iface_dll_cb.wai_rx_packet, "wai_rx_packet");
    wapi_get_func_from_dl(wapi_iface_dll_cb.dl_handle, wapi_iface_dll_cb.wapi_iface_funcs_init, "wapi_iface_funcs_init");
    wapi_get_func_from_dl(wapi_iface_dll_cb.dl_handle, wapi_iface_dll_cb.wapi_cert_free_certificate, "wapi_cert_free_certificate");
    wapi_get_func_from_dl(wapi_iface_dll_cb.dl_handle, wapi_iface_dll_cb.wai_cleanup_fragbuf, "wai_cleanup_fragbuf");
    //wapi_get_func_from_dl(wapi_iface_dll_cb.dl_handle, wapi_iface_dll_cb.wapi_ecc_deinit_dlhandle, "wapi_ecc_deinit_dlhandle");
    wapi_get_func_from_dl(wapi_iface_dll_cb.dl_handle, wapi_iface_dll_cb.wai_psk_derivation, "wai_psk_derivation");
    wapi_get_func_from_dl(wapi_iface_dll_cb.dl_handle, wapi_iface_dll_cb.wapi_cert_parse_ecprivkey, "wapi_cert_parse_ecprivkey");
    wapi_get_func_from_dl(wapi_iface_dll_cb.dl_handle, wapi_iface_dll_cb.wapi_cert_verify_certificate, "wapi_cert_verify_certificate");
    wapi_get_func_from_dl(wapi_iface_dll_cb.dl_handle, wapi_iface_dll_cb.wapi_ecc192_verify_keygroup, "wapi_ecc192_verify_keygroup");
    wapi_get_func_from_dl(wapi_iface_dll_cb.dl_handle, wapi_iface_dll_cb.wapi_cert_parse_certificate, "wapi_cert_parse_certificate");
    //wapi_get_func_from_dl(wapi_iface_dll_cb.dl_handle, wapi_iface_dll_cb.wapi_crypto_init_dlhandle, "wapi_crypto_init_dlhandle");

    //wapi_get_func_from_dl(wapi_iface_dll_cb.dl_handle, wapi_iface_dll_cb.HMAC_SHA256, "HMAC_SHA256");
    wapi_get_func_from_dl(wapi_iface_dll_cb.dl_handle, wapi_iface_dll_cb.KD_HMAC_SHA256, "KD_HMAC_SHA256");

    return 0;
}
int wapi_deinit_dlhandle(void)
{
    if(NULL == wapi_iface_dll_cb.dl_handle)
    {
        wpa_printf(MSG_ERROR,"WAPI LIB is already closed!");
        return 0;
    }

    dlclose(wapi_iface_dll_cb.dl_handle);
    wapi_iface_dll_cb.dl_handle = NULL;
    os_memset(&wapi_iface_dll_cb, 0, sizeof(wapi_iface_dll_cb));
    return 0;
}

int32 wapi_call_back_init()

{
    struct wapi_iface_funcs_stru wapi_cb;


	wpa_printf(MSG_ERROR,"wapi_call_back_init enter!");
	wapi_cb.base64_decode=base64_decode;
	wapi_cb.eloop_cancel_timeout=eloop_cancel_timeout;
	wapi_cb.eloop_register_timeout=eloop_register_timeout;
	wapi_cb.l2_packet_send=l2_packet_send;
	wapi_cb.os_mktime=os_mktime;
	wapi_cb.os_zalloc=os_zalloc;
	wapi_cb.os_get_random=os_get_random;
	wapi_cb.wpabuf_put=wpabuf_put;
	wapi_cb.wpabuf_put_u8=wpabuf_put_u8;
	wapi_cb.wpabuf_put_be16=wpabuf_put_be16;
	wapi_cb.wpabuf_put_data=wpabuf_put_data;
	wapi_cb.wpabuf_put_buf=wpabuf_put_buf;
	wapi_cb.wpabuf_alloc=wpabuf_alloc;
	wapi_cb.wpabuf_free=wpabuf_free;
	wapi_cb.wpabuf_alloc_ext_data=wpabuf_alloc_ext_data;
#ifdef ANDROID
	wapi_cb.__android_log_vprint=__android_log_vprint;
#endif
	wapi_cb.wpa_hexdump_ascii=wpa_hexdump_ascii;
	wapi_cb.wpa_supplicant_set_state=wpa_supplicant_set_state;
	wapi_cb.wpa_supplicant_deauthenticate=wpa_supplicant_deauthenticate;
	wapi_cb.wpa_supplicant_cancel_auth_timeout=wpa_supplicant_cancel_auth_timeout;
    wapi_cb.wapi_notify_wrong_msg=wapi_notify_wrong_msg;

	if(wapi_iface_dll_cb.wapi_iface_funcs_init(&wapi_cb))
	{
		wpa_printf(MSG_ERROR, "wapi_iface_funcs_init fail");
		return -WAPI_FAILED;
	}
	return WAPI_SUCCESS;


}



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


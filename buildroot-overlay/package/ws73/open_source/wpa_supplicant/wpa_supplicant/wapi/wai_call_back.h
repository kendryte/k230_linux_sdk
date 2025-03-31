/*
 * Copyright (c) CompanyNameMagicTag. 2023-2023. All rights reserved.
 * 文 件 名   : wai_call_back.h
 * 作    者   : CompanyName
 * 生成日期   : 2023年1月11日
 * 功能描述   : wai_call_back接口定义
 */

#ifndef __WAPI_CALL_BACK_H__
#define __WAPI_CALL_BACK_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

struct wapi_iface_funcs_stru {
	unsigned char *(*base64_decode)(const unsigned char *src, size_t len,
					size_t *out_len);
	int (*eloop_cancel_timeout)(eloop_timeout_handler handler,
				    void *eloop_data, void *user_data);
	int (*eloop_register_timeout)(unsigned int secs, unsigned int usecs,
				      eloop_timeout_handler handler,
				      void *eloop_data, void *user_data);
	int (*l2_packet_send)(struct l2_packet_data *l2, const u8 *dst_addr,
			      u16 proto, const u8 *buf, size_t len);
	int (*os_mktime)(int year, int month, int day, int hour, int min,
			 int sec, os_time_t *t);
	void *(*os_zalloc)(size_t size);
	int (*os_get_random)(unsigned char *buf, size_t len);
	void *(*wpabuf_put)(struct wpabuf *buf, size_t len);
	void (*wpabuf_put_u8)(struct wpabuf *buf, u8 data);
	void (*wpabuf_put_be16)(struct wpabuf *buf, u16 data);
	void (*wpabuf_put_data)(struct wpabuf *buf, const void *data,
				size_t len);
	void (*wpabuf_put_buf)(struct wpabuf *dst,const struct wpabuf *src);
	struct wpabuf *(*wpabuf_alloc)(size_t len);
	void (*wpabuf_free)(struct wpabuf *buf);
	struct wpabuf *(*wpabuf_alloc_ext_data)(u8 *data, size_t len);
	int (*__android_log_vprint)(int prio, const char *tag, const char *fmt,
				    va_list ap);
	void (*wpa_hexdump_ascii)(int level, const char *title, const void *buf,
				  size_t len);
	void (*wpa_supplicant_set_state)(struct wpa_supplicant *wpa_s,
					 enum wpa_states state);
	void (*wpa_supplicant_deauthenticate)(struct wpa_supplicant *wpa_s,
					      int reason_code);
	void (*wpa_supplicant_cancel_auth_timeout)(struct wpa_supplicant *wpa_s);
	void (*wapi_notify_wrong_msg)(struct wpa_supplicant  *pst_wpa);

};

__attribute__((visibility ("default"))) int wapi_iface_funcs_init(
	struct wapi_iface_funcs_stru *wapi_iface_call_back);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wai_call_back.h */

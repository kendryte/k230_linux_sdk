/*
 * Copyright (c) CompanyNameMagicTag. 2023-2023. All rights reserved.
 * 文 件 名   : wai_rxtx.h
 * 作    者   : CompanyName
 * 生成日期   : 2023年1月11日
 * 功能描述   : tx rx相关接口
 */


#ifndef __WAI_RXTX_H__
#define __WAI_RXTX_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*
 * 1 Other Header File Including
 */
#include "types.h"
#include "wpabuf.h"


/*
 * 2 Macro Definition
 */

#define WAI_FRAME_MAX_SIZE  (32*1024 - sizeof(struct wpabuf))
#define WAI_FRAME_INIT_SIZE (4096 - sizeof(struct wpabuf))


#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

#define WAI_VERSION         1
#define WAI_TYPE            1

/*
 * 3 Enum Type Definition
 */



/*
 * 4 Global Variable Declaring
 */


/*
 * 5 Message Header Definition
 */


/*
 * 6 Message Definition
 */


/*
 * 7 STRUCT Type Definition
 */

#ifdef WIN32
#define __attribute__(x)
#pragma pack(1)
#endif
typedef struct _wai_hdr_stru {

	uint8 auVersion[2];
	uint8 ucType;
	uint8 ucSubtype;
	uint8 auReserve[2];
	uint8 auLength[2];
	uint8 auFrameSeq[2];
	uint8 ucFragSeq;
	uint8 ucMoreFrag;
} __attribute__((packed)) wai_hdr_stru;
#ifdef WIN32
#pragma pack( )
#endif

#define ETH_MTU         1500UL
#define WAI_MTU         (ETH_MTU - sizeof(wai_hdr_stru))

/*
 * 8 UNION Type Definition
 */


/*
 * 9 OTHERS Definition
 */


/*
 * 10 Function Declare
 */


STATIC void  wai_defrag_firstfrag(struct wapi_supplicant_stru *pst_wapi,
				  struct wpabuf *pst_waibuf, uint16 usFrameSeq,
				  uint8 ucFragSeq);
STATIC struct wpabuf *wai_defrag_nextfrag(struct wapi_supplicant_stru *pst_wapi,
					   struct wpabuf *pst_waibuf,
					   uint16 usFrameSeq, uint8 ucFragSeq,
					   uint8 ucMoreFrag);
STATIC struct wpabuf *wai_defrag_nonfrag(struct wapi_supplicant_stru *pst_wapi,
					 struct wpabuf *pst_waibuf,
					 uint16 usFrameSeq);
STATIC struct wpabuf *wai_defrag(struct wpa_supplicant *pst_wpa,
				 struct wpabuf *pst_waibuf);

STATIC int32 wai_copy2fragbuf(struct wapi_supplicant_stru *pst_wapi,
			      struct wpabuf *pst_newfrag, uint32 ul_1stfrag);
STATIC int32 wai_check_valid_frame(struct wpa_supplicant *pst_wpa,
				   const uint8 *pauc_src_mac,
				   const uint8 *pauc_buf, uint32 ul_datalen);

extern __attribute__((visibility ("default"))) void  wai_rx_packet(void *pv_ctx,
	const uint8 *pauc_src_mac, const uint8 *pauc_buf, uint32 ul_datalen);
extern int32 wai_tx_packet(struct wpa_supplicant *pst_wpa,
	const uint8 *pauc_buf, uint32 ul_datalen);
extern int32 __attribute__((visibility ("default"))) wai_cleanup_fragbuf(
	struct wapi_supplicant_stru *pst_wapi);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wai_rxtx.h */

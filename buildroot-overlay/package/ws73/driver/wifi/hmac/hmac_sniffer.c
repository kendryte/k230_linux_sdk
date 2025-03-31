/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: hook function for wifi sniffer_save_file.
 */

#include "hmac_sniffer.h"
#include "hmac_hook.h"
#include "oal_netbuf_data.h"

#if defined(_PRE_OS_VERSION_LINUX) && defined(_PRE_OS_VERSION) \
    && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "wlan_types_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(_PRE_WLAN_FEATURE_SNIFFER)

#define sniffer_pn(fmt...)                  wifi_printf(fmt)

proc_file_config_struct g_sniffer_st = {
    0, 0, TRIM_DATA_PAYLOAD_LEN, PCAP_FILE_NUM, PCAP_FILE_LEN, 1, PCAP_FILE_LEN, {0}, {OSAL_NULL}, {0}
};

static INLINE__ void proc_set_cur_file_index_to_first(void)
{
    g_sniffer_st.cur_file = 0;
}

static INLINE__ void proc_cur_file_move_to_next(void)
{
    const osal_s32 next_file = (g_sniffer_st.cur_file + 1) % g_sniffer_st.max_file_num;

    oam_warning_log2(0, OAM_SF_ANY, "{proc_cur_file_move_to_next::cur_file[%d] is or going to be full, move next[%d]}",
        g_sniffer_st.cur_file, next_file);

    g_sniffer_st.file_full[g_sniffer_st.cur_file] = 1;
    g_sniffer_st.cur_file = next_file;
}

static INLINE__ void proc_increase_cur_file_pos(int len)
{
    g_sniffer_st.curpos[g_sniffer_st.cur_file] += len;
}

static INLINE__ int proc_write_pcap_hdr_for_empty_file(int flag_rx_tx)
{
    wal_pcap_filehdr_stru *pcap_filehdr = (wal_pcap_filehdr_stru *)g_sniffer_st.buffer[g_sniffer_st.cur_file];

    if (sniffer_is_cur_file_empty() == 0) {
        return 0;
    }

    pcap_filehdr->magic = PCAP_PKT_HDR_MAGIC;
    pcap_filehdr->version_major = PCAP_PKT_HDR_VER_MAJ;
    pcap_filehdr->version_minor = PCAP_PKT_HDR_VER_MIN;
    pcap_filehdr->thiszone = PCAP_PKT_HDR_THISZONE;
    pcap_filehdr->sigfigs = PCAP_PKT_HDR_SIGFIGS;
    pcap_filehdr->snaplen = PCAP_PKT_HDR_SNAPLEN;
    pcap_filehdr->linktype = PCAP_PKT_HDR_LINKTYPE_802_11;

    proc_increase_cur_file_pos(sizeof(wal_pcap_filehdr_stru));
    return sizeof(wal_pcap_filehdr_stru);
}

static INLINE__ void proc_update_packet_pcap_header(char *file_buffer, int packet_len)
{
    osal_timeval tv;
    wal_pcap_pkthdr_stru st_pcap_pkthdr = {{0, 0}, 0, 0};

    osal_gettimeofday(&tv);
    st_pcap_pkthdr.time.micro_time = tv.tv_usec;
    st_pcap_pkthdr.time.gmt_time = tv.tv_sec;
    st_pcap_pkthdr.caplen = packet_len;
    st_pcap_pkthdr.len = packet_len;

    if (memcpy_s(file_buffer, sizeof(wal_pcap_pkthdr_stru), &st_pcap_pkthdr, sizeof(wal_pcap_pkthdr_stru)) != EOK) {
        oam_warning_log0(0, OAM_SF_ANY, "{proc_update_packet_pcap_header::memcpy_s err!}");
    }
}

static INLINE__ void proc_set_rx_packet_to_no_proection(int flag_rx_tx, int frame_type, osal_u32 header_len,
    char *dst_buff)
{
    if ((flag_rx_tx == DIRECTION_RX) &&
        (frame_type == IEEE80211_FRAME_TYPE_DATA))
        ((mac_header_control_stru *)dst_buff)->bit_protected_frame = 0;

    if ((flag_rx_tx == DIRECTION_TX) &&
        (frame_type == IEEE80211_FRAME_TYPE_DATA) &&
        (header_len == PCAP_QOS_DATA_PKT_HDR_LEN) &&
        (((mac_header_control_stru *)dst_buff)->bit_from_ds == 1)) {
        ((mac_header_control_stru *)dst_buff)->bit_sub_type = IEEE80211_FRAME_SUBTYPE_QOS;
        // disable amsdu bit; what about NON-QOS data, or 4-address data
        ((mac_ieee80211_qos_stru *)dst_buff)->bit_qc_amsdu = 0;
    }
}

static INLINE__ void proc_write_packet_to_file(char *dst_buff, const char *header_buff,
    int header_len, const char *frame_buff, int frame_len)
{
    osal_s32 ret;

    proc_update_packet_pcap_header(dst_buff, frame_len + header_len);
    if (header_buff != OSAL_NULL) {
        ret = memcpy_s(dst_buff + sizeof(wal_pcap_pkthdr_stru), header_len, header_buff, header_len);
        if (osal_unlikely(ret != EOK)) {
            wifi_printf("proc_write_packet_to_file::copy header failed, ret=%d", ret);
        }
    }

    ret = memcpy_s(dst_buff + sizeof(wal_pcap_pkthdr_stru) + header_len, frame_len, frame_buff, frame_len);
    if (osal_unlikely(ret != EOK)) {
        wifi_printf("proc_write_packet_to_file::copy frame failed, ret=%d", ret);
    }
}

static INLINE__ void record_cur_file_time(const wal_pcap_pkthdr_stru *packet_header)
{
    g_sniffer_st.gmttime[g_sniffer_st.cur_file] = packet_header->time.gmt_time;
    g_sniffer_st.microtime[g_sniffer_st.cur_file] = packet_header->time.micro_time;
}

osal_s32 hmac_sniffer_get_switch(void)
{
    return g_sniffer_st.sniffer_switch;
}

void proc_handle_command_save_file(void)
{
    return;
}

static INLINE__ void proc_init_file_content(int file_index, int file_len)
{
    memset_s(g_sniffer_st.buffer[file_index], file_len, 0, file_len);
    g_sniffer_st.curpos[file_index]    = 0;
    g_sniffer_st.file_full[file_index] = 0;
    g_sniffer_st.gmttime[file_index]   = 0;
    g_sniffer_st.microtime[file_index] = 0;
    g_sniffer_st.packetcount[file_index] = 0;
}

static INLINE__ void proc_init_file_to_save_packet(osal_u32 packet_len)
{
    /* current file is not full, and will be full if write one more packet */
    if  ((!g_sniffer_st.file_full[g_sniffer_st.cur_file] && !sniffer_enough_to_save(packet_len)) ||
         g_sniffer_st.file_full[g_sniffer_st.cur_file]) {
        proc_cur_file_move_to_next();
        proc_init_file_content(g_sniffer_st.cur_file, g_sniffer_st.max_file_len);
    }
}

/* 函数功能: 将当前帧写入sniffer文件 */
osal_u32 proc_sniffer_write_file(const osal_s8 *header_buff, osal_u32 header_len,
    const osal_s8 *frame_buff, osal_u32 frame_len, osal_s32 flag_rx_tx)
{
    osal_s8 *dst_buff = OSAL_NULL;
    osal_s32 frame_type;
    mac_header_control_stru *p_frame_type = OSAL_NULL;

    osal_sem_down(&(g_sniffer_st.main_sem));
    if (g_sniffer_st.sniffer_switch != 1) {
        osal_sem_up(&(g_sniffer_st.main_sem));
        return OAL_FAIL;
    }

    if (frame_buff == OSAL_NULL || (frame_len + header_len) < PCAP_PKT_MIN_LEN) {
        wifi_printf("proc_sniffer_write_file::frame_buff=%p, (frame_len + header_len)=%d",
            frame_buff, (frame_len + header_len));
        osal_sem_up(&(g_sniffer_st.main_sem));
        return OAL_FAIL;
    }

    /* what if a large AMSDU frame? */
    if ((frame_len + header_len) >= PCAP_PKT_MAX_LEN) {
        osal_sem_up(&(g_sniffer_st.main_sem));
        return OAL_FAIL;
    }

    proc_init_file_to_save_packet(sizeof(wal_pcap_pkthdr_stru) + frame_len + header_len);
    proc_write_pcap_hdr_for_empty_file(flag_rx_tx);

    dst_buff = g_sniffer_st.buffer[g_sniffer_st.cur_file] + g_sniffer_st.curpos[g_sniffer_st.cur_file];
    proc_increase_cur_file_pos(sizeof(wal_pcap_pkthdr_stru) + frame_len + header_len);
    proc_write_packet_to_file(dst_buff, header_buff, header_len, frame_buff, frame_len);
    record_cur_file_time((wal_pcap_pkthdr_stru *)dst_buff);
    g_sniffer_st.packetcount[g_sniffer_st.cur_file] += 1;

    if (header_buff == OSAL_NULL) {
        p_frame_type = (mac_header_control_stru *)frame_buff;
    } else {
        p_frame_type = (mac_header_control_stru *)header_buff;
    }
    frame_type = p_frame_type->bit_type;

    // this is to fix some bugs in hybrid sniffer mode
    proc_set_rx_packet_to_no_proection(flag_rx_tx, frame_type, header_len, dst_buff + sizeof(wal_pcap_pkthdr_stru));

    osal_sem_up(&(g_sniffer_st.main_sem));

    return (sizeof(wal_pcap_pkthdr_stru) + frame_len + header_len);
}

/* 上报80211 frame保存在Host本地sniffer文件 */
osal_void hmac_sniffer_80211_frame(const osal_u8 *hdr_addr, osal_u8 hdr_len,
    const osal_u8 *frame_addr, osal_u16 frame_len, osal_u32 msg_id)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    osal_u32 flag_rx_tx;

    if (hmac_sniffer_get_switch() == EXT_SWITCH_OFF || hmac_sniffer_get_switch() == EXT_SWITCH_PAUSE) {
        return;
    }
    if ((msg_id == SOC_DIAG_MSG_ID_WIFI_RX_80211_FRAME) ||
        (msg_id == SOC_DIAG_MSG_ID_WIFI_RX_BEACON)) {
        flag_rx_tx = OAM_OTA_FRAME_DIRECTION_TYPE_RX;
    } else {
        flag_rx_tx = OAM_OTA_FRAME_DIRECTION_TYPE_TX;
    }
    proc_sniffer_write_file(hdr_addr, hdr_len, frame_addr, frame_len, flag_rx_tx);
#endif
    return;
}

osal_u32 hmac_rx_report_80211_frame_etc(oal_netbuf_stru **netbuf, hmac_vap_stru *hmac_vap)
{
    osal_u8 sub_type;
    mac_rx_ctl_stru *mac_rx_cb = OSAL_NULL;

    if (hmac_vap == OAL_PTR_NULL || netbuf == OAL_PTR_NULL || *netbuf == OAL_PTR_NULL) {
        oam_error_log0(0, OAM_SF_RX, "{hmac_rx_report_80211_frame_etc::param null.}");
        return OAL_CONTINUE;
    }

    mac_rx_cb = (mac_rx_ctl_stru *)oal_netbuf_cb(*netbuf);
    sub_type = mac_get_frame_type_and_subtype((osal_u8 *)mac_get_rx_cb_mac_hdr(mac_rx_cb));
    if (sub_type == (WLAN_FC0_SUBTYPE_BEACON | WLAN_FC0_TYPE_MGT)) {
        hmac_sniffer_80211_frame((const osal_u8 *)mac_get_rx_cb_mac_hdr(mac_rx_cb), mac_rx_cb->mac_header_len,
            oal_netbuf_rx_data_const(*netbuf), mac_rx_cb->frame_len, SOC_DIAG_MSG_ID_WIFI_RX_BEACON);
    } else {
        if (hmac_sniffer_get_switch() == EXT_SWITCH_ON) {
            hmac_sniffer_80211_frame((osal_u8 *)mac_get_rx_cb_mac_hdr(mac_rx_cb), mac_rx_cb->mac_header_len,
                oal_netbuf_rx_data_const(*netbuf), mac_rx_cb->frame_len, SOC_DIAG_MSG_ID_WIFI_RX_80211_FRAME);
        }
    }

    return OAL_CONTINUE;
}

osal_s32 proc_sniffer_init_res(void)
{
    static osal_u8 proc_init_flag = 0;

    if (proc_init_flag == 1) {
        return OAL_SUCC;
    }

    osal_sem_init(&(g_sniffer_st.main_sem), 0);

    proc_init_flag = 1;
    osal_sem_up(&(g_sniffer_st.main_sem));
    return OAL_SUCC;
}

hmac_netbuf_hook_stru g_sniffer_data_netbuf_hook = {
    .hooknum = HMAC_FRAME_DATA_RX_EVENT_D2H,
    .priority = HMAC_HOOK_PRI_MIDDLE,
    .hook_func = hmac_rx_report_80211_frame_etc,
};
hmac_netbuf_hook_stru g_sniffer_mgmt_netbuf_hook = {
    .hooknum = HMAC_FRAME_MGMT_RX_EVENT_D2H,
    .priority = HMAC_HOOK_PRI_MIDDLE,
    .hook_func = hmac_rx_report_80211_frame_etc,
};

static INLINE__ osal_void proc_sniffer_free_buff(void)
{
    osal_s32 i;

    for (i = 0; i < PCAP_FILE_NUM; i++) {
        if (g_sniffer_st.buffer[i]) {
            vfree(g_sniffer_st.buffer[i]);
            g_sniffer_st.buffer[i] = OSAL_NULL;
        }
    }
    (osal_void)hmac_unregister_netbuf_hook(&g_sniffer_data_netbuf_hook);
    (osal_void)hmac_unregister_netbuf_hook(&g_sniffer_mgmt_netbuf_hook);
}

static INLINE__ osal_s32 proc_sniffer_reset_buff(osal_s32 old_file_num, osal_s32 new_file_num, osal_s32 new_file_len)
{
    osal_s32 i;

    for (i = 0; i < old_file_num; i++) {
        if (g_sniffer_st.buffer[i]) {
            vfree(g_sniffer_st.buffer[i]);
            g_sniffer_st.buffer[i] = OSAL_NULL;
        }
    }

    for (i = 0; i < new_file_num; i++) {
        g_sniffer_st.buffer[i] = (char *)vmalloc(new_file_len);
        if (!g_sniffer_st.buffer[i]) {
            return -ENOMEM;
        }
        proc_init_file_content(i, new_file_len);
    }

    proc_set_cur_file_index_to_first();
    return 0;
}

static INLINE__ osal_u32 proc_sniffer_init_buff(void)
{
    osal_s32 ret;

    if ((g_sniffer_st.buffer[g_sniffer_st.cur_file] != OSAL_NULL) &&
        (g_sniffer_st.max_file_num == g_sniffer_st.new_file_num) &&
        (g_sniffer_st.max_file_len == g_sniffer_st.new_file_len)) {
        return OAL_SUCC;
    }
    ret = proc_sniffer_reset_buff(g_sniffer_st.max_file_num, g_sniffer_st.new_file_num, g_sniffer_st.new_file_len);
    if (ret) {
        osal_s32 i;
        for (i = 0; i < PCAP_FILE_NUM; i++) {
            if (g_sniffer_st.buffer[i]) {
                vfree(g_sniffer_st.buffer[i]);
                g_sniffer_st.buffer[i] = OSAL_NULL;
            }
        }
        wifi_printf(KERN_ERR "%s ERROR : sniffer_reset error", __func__);
        return OAL_FAIL;
    }
    g_sniffer_st.max_file_num = g_sniffer_st.new_file_num;
    g_sniffer_st.max_file_len = g_sniffer_st.new_file_len;

    ret = hmac_register_netbuf_hook(&g_sniffer_data_netbuf_hook);
    ret |= hmac_register_netbuf_hook(&g_sniffer_mgmt_netbuf_hook);
    return ret;
}

osal_s32 proc_handle_command_sniffer_enable(osal_s32 sniffer_switch)
{
    proc_sniffer_init_res();

    osal_sem_down(&(g_sniffer_st.main_sem));
    if (g_sniffer_st.sniffer_switch == sniffer_switch) {
        osal_sem_up(&(g_sniffer_st.main_sem));
        return OAL_SUCC;
    }

    if (sniffer_switch == 0) {
        proc_sniffer_free_buff();
    } else if (sniffer_switch == 1) {
        if (proc_sniffer_init_buff() == OAL_FAIL) {
            osal_sem_up(&(g_sniffer_st.main_sem));
            wifi_printf("proc_handle_command_sniffer_enable  , %d", __LINE__);
            return OAL_FAIL; // if fail to realloc buffer mem in current os, then no coninue to proc.
        }
        proc_init_file_content(g_sniffer_st.cur_file, g_sniffer_st.max_file_len); // create a new file
    }

    g_sniffer_st.sniffer_switch = sniffer_switch;
    osal_sem_up(&(g_sniffer_st.main_sem));
    return OAL_SUCC;
}

osal_s32 proc_sniffer_reset_file_num_len(osal_s32 new_file_num, osal_s32 new_file_len)
{
    proc_sniffer_init_res();

    osal_sem_down(&(g_sniffer_st.main_sem));
    g_sniffer_st.new_file_num = new_file_num;
    g_sniffer_st.new_file_len = new_file_len;
    osal_sem_up(&(g_sniffer_st.main_sem));
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif

#endif

/**
 ******************************************************************************
 *
 * @file rwnx_platform.c
 *
 * Copyright (C) RivieraWaves 2012-2019
 *
 ******************************************************************************
 */

#include <linux/module.h>
#include <linux/firmware.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>

#include "rwnx_platform.h"
#include "reg_access.h"
#include "hal_desc.h"
#include "rwnx_main.h"
#include "rwnx_pci.h"
#ifndef CONFIG_RWNX_FHOST
#include "ipc_host.h"
#endif /* !CONFIG_RWNX_FHOST */
#include "rwnx_msg_tx.h"

#ifdef AICWF_SDIO_SUPPORT
#include "aicwf_sdio.h"
#endif

#ifdef AICWF_USB_SUPPORT
#include "aicwf_usb.h"
#endif
#include "md5.h"
#include "aicwf_compat_8800dc.h"
#include "aicwf_compat_8800d80.h"
#include "aicwf_compat_8800d80x2.h"
#ifdef CONFIG_USE_FW_REQUEST
#include <linux/firmware.h>
#endif


#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0))
static inline struct inode *file_inode(const struct file *f)
{
        return f->f_dentry->d_inode;
}
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)) */
struct rwnx_plat *g_rwnx_plat = NULL;

#define FW_PATH_MAX_LEN 200
extern char aic_fw_path[FW_PATH_MAX_LEN];

//Parser state
#define INIT 0
#define CMD 1
#define PRINT 2
#define GET_VALUE 3

typedef struct
{
    txpwr_lvl_conf_t txpwr_lvl;
    txpwr_lvl_conf_v2_t txpwr_lvl_v2;
    txpwr_lvl_conf_v3_t txpwr_lvl_v3;
    txpwr_lvl_conf_v4_t txpwr_lvl_v4;
    txpwr_lvl_adj_conf_t txpwr_lvl_adj;
    txpwr_loss_conf_t txpwr_loss;
    txpwr_ofst_conf_t txpwr_ofst;
    txpwr_ofst2x_conf_t txpwr_ofst2x;
    txpwr_ofst2x_conf_v2_t txpwr_ofst2x_v2;
    xtal_cap_conf_t xtal_cap;
} userconfig_info_t;

userconfig_info_t userconfig_info = {
    .txpwr_lvl = {
        .enable           = 1,
        .dsss             = 9,
        .ofdmlowrate_2g4  = 8,
        .ofdm64qam_2g4    = 8,
        .ofdm256qam_2g4   = 8,
        .ofdm1024qam_2g4  = 8,
        .ofdmlowrate_5g   = 11,
        .ofdm64qam_5g     = 10,
        .ofdm256qam_5g    = 9,
        .ofdm1024qam_5g   = 9
    },
    .txpwr_lvl_v2 = {
        .enable             = 1,
        .pwrlvl_11b_11ag_2g4 =
            //1M,   2M,   5M5,  11M,  6M,   9M,   12M,  18M,  24M,  36M,  48M,  54M
            { 20,   20,   20,   20,   20,   20,   20,   20,   18,   18,   16,   16},
        .pwrlvl_11n_11ac_2g4 =
            //MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9
            { 20,   20,   20,   20,   18,   18,   16,   16,   16,   16},
        .pwrlvl_11ax_2g4 =
            //MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9, MCS10,MCS11
            { 20,   20,   20,   20,   18,   18,   16,   16,   16,   16,   15,   15},
    },
    .txpwr_lvl_v3 = {
        .enable             = 1,
        .pwrlvl_11b_11ag_2g4 =
            //1M,   2M,   5M5,  11M,  6M,   9M,   12M,  18M,  24M,  36M,  48M,  54M
            { 20,   20,   20,   20,   20,   20,   20,   20,   18,   18,   16,   16},
        .pwrlvl_11n_11ac_2g4 =
            //MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9
            { 20,   20,   20,   20,   18,   18,   16,   16,   16,   16},
        .pwrlvl_11ax_2g4 =
            //MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9, MCS10,MCS11
            { 20,   20,   20,   20,   18,   18,   16,   16,   16,   16,   15,   15},
         .pwrlvl_11a_5g =
            //NA,   NA,   NA,   NA,   6M,   9M,   12M,  18M,  24M,  36M,  48M,  54M
            { 0x80, 0x80, 0x80, 0x80, 20,   20,   20,   20,   18,   18,   16,   16},
        .pwrlvl_11n_11ac_5g =
            //MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9
            { 20,   20,   20,   20,   18,   18,   16,   16,   16,   15},
        .pwrlvl_11ax_5g =
            //MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9, MCS10,MCS11
            { 20,   20,   20,   20,   18,   18,   16,   16,   16,   15,   14,   14},
    },
    .txpwr_lvl_v4 = {
        .enable             = 1,
        .pwrlvl_11b_11ag_2g4 =
            //1M,   2M,   5M5,  11M,  6M,   9M,   12M,  18M,  24M,  36M,  48M,  54M
            { 20,   20,   20,   20,   20,   20,   20,   20,   18,   18,   16,   16},
        .pwrlvl_11n_11ac_2g4 =
            //MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9
            { 20,   20,   20,   20,   18,   18,   16,   16,   16,   16},
        .pwrlvl_11ax_2g4 =
            //MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9, MCS10,MCS11
            { 20,   20,   20,   20,   18,   18,   16,   16,   16,   16,   15,   15},
        .pwrlvl_11a_5g =
            //6M,   9M,   12M,  18M,  24M,  36M,  48M,  54M
            { 20,   20,   20,   20,   18,   18,   16,   16},
        .pwrlvl_11n_11ac_5g =
            //MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9
            { 20,   20,   20,   20,   18,   18,   16,   16,   16,   15},
        .pwrlvl_11ax_5g =
            //MCS0, MCS1, MCS2, MCS3, MCS4, MCS5, MCS6, MCS7, MCS8, MCS9, MCS10,MCS11
            { 20,   20,   20,   20,   18,   18,   16,   16,   16,   15,   14,   14},
    },
    .txpwr_loss = {
        .loss_enable      = 1,
        .loss_value       = 0,
    },
    .txpwr_ofst = {
        .enable       = 1,
        .chan_1_4     = 0,
        .chan_5_9     = 0,
        .chan_10_13   = 0,
        .chan_36_64   = 0,
        .chan_100_120 = 0,
        .chan_122_140 = 0,
        .chan_142_165 = 0,
    },
    .txpwr_ofst2x = {
        .enable       = 0,
        .pwrofst2x_tbl_2g4 =
        { // ch1-4, ch5-9, ch10-13
            {   0,    0,    0   }, // 11b
            {   0,    0,    0   }, // ofdm_highrate
            {   0,    0,    0   }, // ofdm_lowrate
        },
        .pwrofst2x_tbl_5g =
        { // ch42,  ch58, ch106,ch122,ch138,ch155
            {   0,    0,    0,    0,    0,    0   }, // ofdm_lowrate
            {   0,    0,    0,    0,    0,    0   }, // ofdm_highrate
            {   0,    0,    0,    0,    0,    0   }, // ofdm_midrate
        },
    },
    .txpwr_ofst2x_v2 = {
        .enable        = 0,
        .pwrofst_flags = 0,
        .pwrofst2x_tbl_2g4_ant0 =
        { // 11b, ofdm_highrate, ofdm_lowrate
            {   0,    0,    0   }, // ch1-4
            {   0,    0,    0   }, // ch5-9
            {   0,    0,    0   }, // ch10-13
        },
        .pwrofst2x_tbl_2g4_ant1 =
        { // 11b, ofdm_highrate, ofdm_lowrate
            {   0,    0,    0   }, // ch1-4
            {   0,    0,    0   }, // ch5-9
            {   0,    0,    0   }, // ch10-13
        },
        .pwrofst2x_tbl_5g_ant0 =
        { // ofdm_highrate, ofdm_lowrate, ofdm_midrate
            {   0,    0,    0   }, // ch42
            {   0,    0,    0   }, // ch58
            {   0,    0,    0   }, // ch106
            {   0,    0,    0   }, // ch122
            {   0,    0,    0   }, // ch138
            {   0,    0,    0   }, // ch155
        },
        .pwrofst2x_tbl_5g_ant1 =
        { // ofdm_highrate, ofdm_lowrate, ofdm_midrate
            {   0,    0,    0   }, // ch42
            {   0,    0,    0   }, // ch58
            {   0,    0,    0   }, // ch106
            {   0,    0,    0   }, // ch122
            {   0,    0,    0   }, // ch138
            {   0,    0,    0   }, // ch155
        },
        .pwrofst2x_tbl_6g_ant0 = {   0,   }, // ofdm_highrate: 6e_ch7 ~ 6e_ch229
        .pwrofst2x_tbl_6g_ant1 = {   0,   }, // ofdm_highrate: 6e_ch7 ~ 6e_ch229
    },
    .xtal_cap = {
        .enable        = 0,
        .xtal_cap      = 24,
        .xtal_cap_fine = 31,
    },
};

#ifdef CONFIG_POWER_LIMIT
#define POWER_LIMIT_INVALID_VAL     POWER_LEVEL_INVALID_VAL

#define POWER_LIMIT_CC_MATCHED_BIT  (0x1U << 0)

typedef struct
{
    u8_l ch_cnt_2g4;
    u8_l ch_cnt_5g;
    u8_l ch_num_2g4[MAC_DOMAINCHANNEL_24G_MAX];
    u8_l ch_num_5g[MAC_DOMAINCHANNEL_5G_MAX];
    s8_l max_pwr_2g4[MAC_DOMAINCHANNEL_24G_MAX];
    s8_l max_pwr_5g[MAC_DOMAINCHANNEL_5G_MAX];
} txpwr_lmt_info_t;

typedef struct
{
    u32_l flags;
    txpwr_lmt_info_t txpwr_lmt;
} powerlimit_info_t;

powerlimit_info_t powerlimit_info = {0,};
#endif

#ifndef CONFIG_ROM_PATCH_EN
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0))
static inline struct inode *file_inode(const struct file *f)
{
        return f->f_dentry->d_inode;
}
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)) */


#endif/* !CONFIG_ROM_PATCH_EN */



#ifdef CONFIG_RWNX_TL4
/**
 * rwnx_plat_tl4_fw_upload() - Load the requested FW into embedded side.
 *
 * @rwnx_plat: pointer to platform structure
 * @fw_addr: Virtual address where the fw must be loaded
 * @filename: Name of the fw.
 *
 * Load a fw, stored as a hex file, into the specified address
 */
static int rwnx_plat_tl4_fw_upload(struct rwnx_plat *rwnx_plat, u8* fw_addr,
                                   char *filename)
{
    struct device *dev = rwnx_platform_get_dev(rwnx_plat);
    const struct firmware *fw;
    int err = 0;
    u32 *dst;
    u8 const *file_data;
    char typ0, typ1;
    u32 addr0, addr1;
    u32 dat0, dat1;
    int remain;

    err = request_firmware(&fw, filename, dev);
    if (err) {
        return err;
    }
    file_data = fw->data;
    remain = fw->size;

    /* Copy the file on the Embedded side */
    dev_dbg(dev, "\n### Now copy %s firmware, @ = %p\n", filename, fw_addr);

    /* Walk through all the lines of the configuration file */
    while (remain >= 16) {
        u32 data, offset;

        if (sscanf(file_data, "%c:%08X %04X", &typ0, &addr0, &dat0) != 3)
            break;
        if ((addr0 & 0x01) != 0) {
            addr0 = addr0 - 1;
            dat0 = 0;
        } else {
            file_data += 16;
            remain -= 16;
        }
        if ((remain < 16) ||
            (sscanf(file_data, "%c:%08X %04X", &typ1, &addr1, &dat1) != 3) ||
            (typ1 != typ0) || (addr1 != (addr0 + 1))) {
            typ1 = typ0;
            addr1 = addr0 + 1;
            dat1 = 0;
        } else {
            file_data += 16;
            remain -= 16;
        }

        if (typ0 == 'C') {
            offset = 0x00200000;
            if ((addr1 % 4) == 3)
                offset += 2*(addr1 - 3);
            else
                offset += 2*(addr1 + 1);

            data = dat1 | (dat0 << 16);
        } else {
            offset = 2*(addr1 - 1);
            data = dat0 | (dat1 << 16);
        }
        dst = (u32 *)(fw_addr + offset);
        *dst = data;
    }

    release_firmware(fw);

    return err;
}
#endif

#if 0
/**
 * rwnx_plat_bin_fw_upload() - Load the requested binary FW into embedded side.
 *
 * @rwnx_plat: pointer to platform structure
 * @fw_addr: Virtual address where the fw must be loaded
 * @filename: Name of the fw.
 *
 * Load a fw, stored as a binary file, into the specified address
 */
static int rwnx_plat_bin_fw_upload(struct rwnx_plat *rwnx_plat, u8* fw_addr,
                               char *filename)
{
    const struct firmware *fw;
    struct device *dev = rwnx_platform_get_dev(rwnx_plat);
    int err = 0;
    unsigned int i, size;
    u32 *src, *dst;

    err = request_firmware(&fw, filename, dev);
    if (err) {
        return err;
    }

    /* Copy the file on the Embedded side */
    dev_dbg(dev, "\n### Now copy %s firmware, @ = %p\n", filename, fw_addr);

    src = (u32 *)fw->data;
    dst = (u32 *)fw_addr;
    size = (unsigned int)fw->size;

    /* check potential platform bug on multiple stores vs memcpy */
    for (i = 0; i < size; i += 4) {
        *dst++ = *src++;
    }

    release_firmware(fw);

    return err;
}
#endif

#define MD5(x) x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7],x[8],x[9],x[10],x[11],x[12],x[13],x[14],x[15]
#define MD5PINRT "file md5:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n"

static int rwnx_load_firmware(u32 **fw_buf, const char *name, struct device *device)
{

#ifdef CONFIG_USE_FW_REQUEST
	const struct firmware *fw = NULL;
	u32 *dst = NULL;
	void *buffer=NULL;
	MD5_CTX md5;
	unsigned char decrypt[16];
	int size = 0;
	int ret = 0;

	AICWFDBG(LOGINFO, "%s: request firmware = %s \n", __func__ ,name);

	ret = request_firmware(&fw, name, NULL);

	if (ret < 0) {
		AICWFDBG(LOGERROR, "Load %s fail\n", name);
		release_firmware(fw);
		return -1;
	}

	size = fw->size;
	dst = (u32 *)fw->data;

	if (size <= 0) {
		AICWFDBG(LOGERROR, "wrong size of firmware file\n");
		release_firmware(fw);
		return -1;
	}

	buffer = vmalloc(size);
	memset(buffer, 0, size);
	memcpy(buffer, dst, size);

	*fw_buf = buffer;

	MD5Init(&md5);
	MD5Update(&md5, (unsigned char *)buffer, size);
	MD5Final(&md5, decrypt);
	AICWFDBG(LOGINFO, MD5PINRT, MD5(decrypt));

	release_firmware(fw);

	return size;
#else
    void *buffer = NULL;
    char *path = NULL;
    struct file *fp = NULL;
    int size = 0, len = 0;// i = 0;
    ssize_t rdlen = 0;
    //u32 *src = NULL, *dst = NULL;
	MD5_CTX md5;
	unsigned char decrypt[16];

    /* get the firmware path */
    path = __getname();
    if (!path) {
        *fw_buf = NULL;
        return -1;
    }

	len = snprintf(path, FW_PATH_MAX_LEN, "%s/%s", aic_fw_path, name);

    //len = snprintf(path, FW_PATH_MAX_LEN, "%s", name);
    if (len >= FW_PATH_MAX_LEN) {
        AICWFDBG(LOGERROR, "%s: %s file's path too long\n", __func__, name);
        *fw_buf = NULL;
        __putname(path);
        return -1;
    }

    AICWFDBG(LOGINFO, "%s :firmware path = %s  \n", __func__, path);

    /* open the firmware file */
    fp = filp_open(path, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(fp)) {
        AICWFDBG(LOGERROR, "%s: %s file failed to open\n", __func__, name);
        *fw_buf = NULL;
        __putname(path);
        fp = NULL;
        return -1;
    }

    size = i_size_read(file_inode(fp));
    if (size <= 0) {
        AICWFDBG(LOGERROR, "%s: %s file size invalid %d\n", __func__, name, size);
        *fw_buf = NULL;
        __putname(path);
        filp_close(fp, NULL);
        fp = NULL;
        return -1;
    }

    /* start to read from firmware file */
    buffer = vmalloc(size);
    if (!buffer) {
        *fw_buf = NULL;
        __putname(path);
        filp_close(fp, NULL);
        fp = NULL;
        return -1;
    }

    #if LINUX_VERSION_CODE > KERNEL_VERSION(4, 13, 16)
    rdlen = kernel_read(fp, buffer, size, &fp->f_pos);
    #else
    rdlen = kernel_read(fp, fp->f_pos, buffer, size);
    #endif

    if (size != rdlen) {
        AICWFDBG(LOGERROR, "%s: %s file rdlen invalid %d\n", __func__, name, (int)rdlen);
        *fw_buf = NULL;
        __putname(path);
        filp_close(fp, NULL);
        fp = NULL;
        vfree(buffer);
        buffer = NULL;
        return -1;
    }
    if (rdlen > 0) {
        fp->f_pos += rdlen;
    }

#if 0
    /*start to transform the data format*/
    src = (u32 *)buffer;
    dst = (u32 *)vmalloc(size);

    if (!dst) {
        *fw_buf = NULL;
        __putname(path);
        filp_close(fp, NULL);
        fp = NULL;
        vfree(buffer);
        buffer = NULL;
        return -1;
    }

    for (i = 0; i < (size/4); i++) {
        dst[i] = src[i];
    }
#endif

    __putname(path);
    filp_close(fp, NULL);
    fp = NULL;
    //vfree(buffer);
    //buffer = NULL;
    *fw_buf = (u32*)buffer;

	MD5Init(&md5);
	MD5Update(&md5, (unsigned char *)buffer, size);
	MD5Final(&md5, decrypt);

	AICWFDBG(LOGINFO, MD5PINRT, MD5(decrypt));

    return size;
#endif
}

static void rwnx_restore_firmware(u32 **fw_buf)
{
    vfree(*fw_buf);
    *fw_buf = NULL;
}


/* buffer is allocated by kzalloc */
int rwnx_request_firmware_common(struct rwnx_hw *rwnx_hw, u32** buffer, const char *filename)
{
    int size;

    AICWFDBG(LOGINFO, "### Load file %s\n", filename);

    size = rwnx_load_firmware(buffer, filename, NULL);

    return size;
}

void rwnx_release_firmware_common(u32** buffer)
{
    rwnx_restore_firmware(buffer);
}



/**
 * rwnx_plat_bin_fw_upload_2() - Load the requested binary FW into embedded side.
 *
 * @rwnx_hw: Main driver data
 * @fw_addr: Address where the fw must be loaded
 * @filename: Name of the fw.
 *
 * Load a fw, stored as a binary file, into the specified address
 */

int rwnx_plat_bin_fw_upload_2(struct rwnx_hw *rwnx_hw, u32 fw_addr,
                               char *filename)
{
    int err = 0;
    unsigned int i = 0, size;
//    u32 *src;
	u32 *dst = NULL;

    /* Copy the file on the Embedded side */
    AICWFDBG(LOGINFO, "### Upload %s firmware, @ = %x\n", filename, fw_addr);

    size = rwnx_request_firmware_common(rwnx_hw, &dst, filename);
    if (!dst) {
	    AICWFDBG(LOGERROR, "No such file or directory\n");
	    return -1;
    }
    if (size <= 0) {
            AICWFDBG(LOGERROR, "wrong size of firmware file\n");
            dst = NULL;
            err = -1;
		return -1;
    }

	AICWFDBG(LOGINFO, "size=%d, dst[0]=%x\n", size, dst[0]);
    if (size > 512) {
        for (; i < (size - 512); i += 512) {
            //printk("wr blk 0: %p -> %x\r\n", dst + i / 4, fw_addr + i);
            err = rwnx_send_dbg_mem_block_write_req(rwnx_hw, fw_addr + i, 512, dst + i / 4);
            if (err) {
                AICWFDBG(LOGERROR, "bin upload fail: %x, err:%d\r\n", fw_addr + i, err);
                break;
            }
        }
    }
    if (!err && (i < size)) {
        //printk("wr blk 1: %p -> %x\r\n", dst + i / 4, fw_addr + i);
        err = rwnx_send_dbg_mem_block_write_req(rwnx_hw, fw_addr + i, size - i, dst + i / 4);
        if (err) {
            AICWFDBG(LOGERROR, "bin upload fail: %x, err:%d\r\n", fw_addr + i, err);
        }
    }

    if (dst) {
        rwnx_release_firmware_common(&dst);
    }

    return err;
}



#ifndef CONFIG_ROM_PATCH_EN
#if defined(CONFIG_PLATFORM_ALLWINNER) || defined(CONFIG_NANOPI_M4)
#if 0
static int aic_load_firmware(u32 ** fw_buf, const char *name,
                 struct device *device)
{
        void *buffer=NULL;
        char *path=NULL;
        struct file *fp=NULL;
        int size = 0, len=0, i=0;
        ssize_t rdlen=0;
        u32 *src=NULL, *dst = NULL;
        RWNX_DBG(RWNX_FN_ENTRY_STR);

        /* get the firmware path */
        path = __getname();
        if (!path){
                *fw_buf=NULL;
                return -1;
        }

        len = snprintf(path, FW_PATH_MAX_LEN, "%s/%s",aic_fw_path, name);
        if (len >= FW_PATH_MAX_LEN) {
                printk("%s: %s file's path too long\n", __func__, name);
                *fw_buf=NULL;
                __putname(path);
                return -1;
        }

        printk("%s :firmware path = %s  \n", __func__ ,path);


        /* open the firmware file */
        fp=filp_open(path, O_RDONLY, 0);
        if(IS_ERR(fp) || (!fp)){
	        printk("%s: %s file failed to open\n", __func__, name);
                if(IS_ERR(fp))
			printk("is_Err\n");
		*fw_buf=NULL;
                __putname(path);
                fp=NULL;
                return -1;
        }

        size = i_size_read(file_inode(fp));
        if(size<=0){
                printk("%s: %s file size invalid %d\n", __func__, name, size);
                *fw_buf=NULL;
                __putname(path);
                filp_close(fp,NULL);
                fp=NULL;
                return -1;
	}

        /* start to read from firmware file */
        buffer = kzalloc(size, GFP_KERNEL);
        if(!buffer){
                *fw_buf=NULL;
                __putname(path);
                filp_close(fp,NULL);
                fp=NULL;
                return -1;
        }


        #if LINUX_VERSION_CODE > KERNEL_VERSION(4, 13, 16)
        rdlen = kernel_read(fp, buffer, size, &fp->f_pos);
        #else
        rdlen = kernel_read(fp, fp->f_pos, buffer, size);
        #endif

        if(size != rdlen){
                printk("%s: %s file rdlen invalid %ld\n", __func__, name, (long int)rdlen);
                *fw_buf=NULL;
                __putname(path);
                filp_close(fp,NULL);
                fp=NULL;
                kfree(buffer);
                buffer=NULL;
                return -1;
        }
        if(rdlen > 0){
                fp->f_pos += rdlen;
        }


       /*start to transform the data format*/
        src = (u32*)buffer;
        printk("malloc dst\n");
        dst = (u32*)kzalloc(size,GFP_KERNEL);

        if(!dst){
                *fw_buf=NULL;
                __putname(path);
                filp_close(fp,NULL);
                fp=NULL;
                kfree(buffer);
                buffer=NULL;
                return -1;
        }

        for(i=0;i<(size/4);i++){
                dst[i] = src[i];
        }

        __putname(path);
        filp_close(fp,NULL);
        fp=NULL;
        kfree(buffer);
        buffer=NULL;
        *fw_buf = dst;

        return size;

}
#endif
#endif
#endif


#ifndef CONFIG_ROM_PATCH_EN
#if defined(CONFIG_PLATFORM_ALLWINNER) || defined(CONFIG_NANOPI_M4)
#if 0
static int rwnx_plat_bin_fw_upload_android(struct rwnx_hw *rwnx_hw, u32 fw_addr,
                               char *filename)
{
    struct device *dev = rwnx_platform_get_dev(rwnx_hw->plat);
    unsigned int i=0;
    int size;
    u32 *dst=NULL;
    int err=0;


        /* load aic firmware */
        size = aic_load_firmware(&dst, filename, dev);
        if(size<=0){
                printk("wrong size of firmware file\n");
                kfree(dst);
                dst = NULL;
                return -1;
        }


    /* Copy the file on the Embedded side */
    printk("\n### Upload %s firmware, @ = %x  size=%d\n", filename, fw_addr, size);

    if (size > 1024) {// > 1KB data
        for (i = 0; i < (size - 1024); i += 1024) {//each time write 1KB
            err = rwnx_send_dbg_mem_block_write_req(rwnx_hw, fw_addr + i, 1024, dst + i / 4);
                        if (err) {
                printk("bin upload fail: %x, err:%d\r\n", fw_addr + i, err);
                break;
            }
        }
    }

    if (!err && (i < size)) {// <1KB data
        err = rwnx_send_dbg_mem_block_write_req(rwnx_hw, fw_addr + i, size - i, dst + i / 4);
        if (err) {
            printk("bin upload fail: %x, err:%d\r\n", fw_addr + i, err);
        }
    }

    if (dst) {
        kfree(dst);
        dst = NULL;
    }

    return err;
}
#endif
#endif
#endif



#if 0
#ifndef CONFIG_RWNX_TL4
#define IHEX_REC_DATA           0
#define IHEX_REC_EOF            1
#define IHEX_REC_EXT_SEG_ADD    2
#define IHEX_REC_START_SEG_ADD  3
#define IHEX_REC_EXT_LIN_ADD    4
#define IHEX_REC_START_LIN_ADD  5

/**
 * rwnx_plat_ihex_fw_upload() - Load the requested intel hex 8 FW into embedded side.
 *
 * @rwnx_plat: pointer to platform structure
 * @fw_addr: Virtual address where the fw must be loaded
 * @filename: Name of the fw.
 *
 * Load a fw, stored as a ihex file, into the specified address.
 */
static int rwnx_plat_ihex_fw_upload(struct rwnx_plat *rwnx_plat, u8* fw_addr,
                                    char *filename)
{
    const struct firmware *fw;
    struct device *dev = rwnx_platform_get_dev(rwnx_plat);
    u8 const *src, *end;
    u32 *dst;
    u16 haddr, segaddr, addr;
    u32 hwaddr;
    u8 load_fw, byte_count, checksum, csum, rec_type;
    int err, rec_idx;
    char hex_buff[9];

    err = request_firmware(&fw, filename, dev);
    if (err) {
        return err;
    }

    /* Copy the file on the Embedded side */
    dev_dbg(dev, "\n### Now copy %s firmware, @ = %p\n", filename, fw_addr);

    src = fw->data;
    end = src + (unsigned int)fw->size;
    haddr = 0;
    segaddr = 0;
    load_fw = 1;
    err = -EINVAL;
    rec_idx = 0;
    hwaddr = 0;

#define IHEX_READ8(_val, _cs) {                  \
        hex_buff[2] = 0;                         \
        strncpy(hex_buff, src, 2);               \
        if (kstrtou8(hex_buff, 16, &_val))       \
            goto end;                            \
        src += 2;                                \
        if (_cs)                                 \
            csum += _val;                        \
    }

#define IHEX_READ16(_val) {                        \
        hex_buff[4] = 0;                           \
        strncpy(hex_buff, src, 4);                 \
        if (kstrtou16(hex_buff, 16, &_val))        \
            goto end;                              \
        src += 4;                                  \
        csum += (_val & 0xff) + (_val >> 8);       \
    }

#define IHEX_READ32(_val) {                              \
        hex_buff[8] = 0;                                 \
        strncpy(hex_buff, src, 8);                       \
        if (kstrtouint(hex_buff, 16, &_val))             \
            goto end;                                    \
        src += 8;                                        \
        csum += (_val & 0xff) + ((_val >> 8) & 0xff) +   \
            ((_val >> 16) & 0xff) + (_val >> 24);        \
    }

#define IHEX_READ32_PAD(_val, _nb) {                    \
        memset(hex_buff, '0', 8);                       \
        hex_buff[8] = 0;                                \
        strncpy(hex_buff, src, (2 * _nb));              \
        if (kstrtouint(hex_buff, 16, &_val))            \
            goto end;                                   \
        src += (2 * _nb);                               \
        csum += (_val & 0xff) + ((_val >> 8) & 0xff) +  \
            ((_val >> 16) & 0xff) + (_val >> 24);       \
}

    /* loop until end of file is read*/
    while (load_fw) {
        rec_idx++;
        csum = 0;

        /* Find next colon start code */
        while (*src != ':') {
            src++;
            if ((src + 3) >= end) /* 3 = : + rec_len */
                goto end;
        }
        src++;

        /* Read record len */
        IHEX_READ8(byte_count, 1);
        if ((src + (byte_count * 2) + 8) >= end) /* 8 = rec_addr + rec_type + chksum */
            goto end;

        /* Read record addr */
        IHEX_READ16(addr);

        /* Read record type */
        IHEX_READ8(rec_type, 1);

        switch(rec_type) {
            case IHEX_REC_DATA:
            {
                /* Update destination address */
                dst = (u32 *) (fw_addr + hwaddr + addr);

                while (byte_count) {
                    u32 val;
                    if (byte_count >= 4) {
                        IHEX_READ32(val);
                        byte_count -= 4;
                    } else {
                        IHEX_READ32_PAD(val, byte_count);
                        byte_count = 0;
                    }
                    *dst++ = __swab32(val);
                }
                break;
            }
            case IHEX_REC_EOF:
            {
                load_fw = 0;
                err = 0;
                break;
            }
            case IHEX_REC_EXT_SEG_ADD: /* Extended Segment Address */
            {
                IHEX_READ16(segaddr);
                hwaddr = (haddr << 16) + (segaddr << 4);
                break;
            }
            case IHEX_REC_EXT_LIN_ADD: /* Extended Linear Address */
            {
                IHEX_READ16(haddr);
                hwaddr = (haddr << 16) + (segaddr << 4);
                break;
            }
            case IHEX_REC_START_LIN_ADD: /* Start Linear Address */
            {
                u32 val;
                IHEX_READ32(val); /* need to read for checksum */
                break;
            }
            case IHEX_REC_START_SEG_ADD:
            default:
            {
                dev_err(dev, "ihex: record type %d not supported\n", rec_type);
                load_fw = 0;
            }
        }

        /* Read and compare checksum */
        IHEX_READ8(checksum, 0);
        if (checksum != (u8)(~csum + 1))
            goto end;
    }

#undef IHEX_READ8
#undef IHEX_READ16
#undef IHEX_READ32
#undef IHEX_READ32_PAD

  end:
    release_firmware(fw);

    if (err)
        dev_err(dev, "%s: Invalid ihex record around line %d\n", filename, rec_idx);

    return err;
}
#endif /* CONFIG_RWNX_TL4 */

#ifndef CONFIG_RWNX_SDM
/**
 * rwnx_plat_get_rf() - Retrun the RF used in the platform
 *
 * @rwnx_plat: pointer to platform structure
 */
static u32 rwnx_plat_get_rf(struct rwnx_plat *rwnx_plat)
{
    u32 ver;
    ver = RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM, MDM_HDMCONFIG_ADDR);

    ver = __MDM_PHYCFG_FROM_VERS(ver);
    WARN(((ver != MDM_PHY_CONFIG_TRIDENT) &&
          (ver != MDM_PHY_CONFIG_ELMA) &&
          (ver != MDM_PHY_CONFIG_KARST)),
         "bad phy version 0x%08x\n", ver);

    return ver;
}

/**
 * rwnx_plat_stop_agcfsm() - Stop a AGC state machine
 *
 * @rwnx_plat: pointer to platform structure
 * @agg_reg: Address of the agccntl register (within RWNX_ADDR_SYSTEM)
 * @agcctl: Updated with value of the agccntl rgister before stop
 * @memclk: Updated with value of the clock register before stop
 * @agc_ver: Version of the AGC load procedure
 * @clkctrladdr: Indicates which AGC clock register should be accessed
 */
static void rwnx_plat_stop_agcfsm(struct rwnx_plat *rwnx_plat, int agc_reg,
                                  u32 *agcctl, u32 *memclk, u8 agc_ver,
                                  u32 clkctrladdr)
{
    /* First read agcctnl and clock registers */
    *memclk = RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM, clkctrladdr);

    /* Stop state machine : xxAGCCNTL0[AGCFSMRESET]=1 */
    *agcctl = RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM, agc_reg);
    RWNX_REG_WRITE((*agcctl) | BIT(12), rwnx_plat, RWNX_ADDR_SYSTEM, agc_reg);

    /* Force clock */
    if (agc_ver > 0) {
        /* CLKGATEFCTRL0[AGCCLKFORCE]=1 */
        RWNX_REG_WRITE((*memclk) | BIT(29), rwnx_plat, RWNX_ADDR_SYSTEM,
                       clkctrladdr);
    } else {
        /* MEMCLKCTRL0[AGCMEMCLKCTRL]=0 */
        RWNX_REG_WRITE((*memclk) & ~BIT(3), rwnx_plat, RWNX_ADDR_SYSTEM,
                       clkctrladdr);
    }
}


/**
 * rwnx_plat_start_agcfsm() - Restart a AGC state machine
 *
 * @rwnx_plat: pointer to platform structure
 * @agg_reg: Address of the agccntl register (within RWNX_ADDR_SYSTEM)
 * @agcctl: value of the agccntl register to restore
 * @memclk: value of the clock register to restore
 * @agc_ver: Version of the AGC load procedure
 * @clkctrladdr: Indicates which AGC clock register should be accessed
 */
static void rwnx_plat_start_agcfsm(struct rwnx_plat *rwnx_plat, int agc_reg,
                                   u32 agcctl, u32 memclk, u8 agc_ver,
                                   u32 clkctrladdr)
{

    /* Release clock */
    if (agc_ver > 0)
        /* CLKGATEFCTRL0[AGCCLKFORCE]=0 */
        RWNX_REG_WRITE(memclk & ~BIT(29), rwnx_plat, RWNX_ADDR_SYSTEM,
                       clkctrladdr);
    else
        /* MEMCLKCTRL0[AGCMEMCLKCTRL]=1 */
        RWNX_REG_WRITE(memclk | BIT(3), rwnx_plat, RWNX_ADDR_SYSTEM,
                       clkctrladdr);

    /* Restart state machine: xxAGCCNTL0[AGCFSMRESET]=0 */
    RWNX_REG_WRITE(agcctl & ~BIT(12), rwnx_plat, RWNX_ADDR_SYSTEM, agc_reg);
}
#endif

/**
 * rwnx_plat_fcu_load() - Load FCU (Fith Chain Unit) ucode
 *
 * @rwnx_hw: main driver data
 *
 * c.f Modem UM (AGC/CCA initialization)
 */
static int rwnx_plat_fcu_load(struct rwnx_hw *rwnx_hw)
{
    int ret=0;
#ifndef CONFIG_RWNX_SDM
    struct rwnx_plat *rwnx_plat = rwnx_hw->plat;
    u32 agcctl, memclk;

#ifndef CONFIG_RWNX_FHOST
    /* By default, we consider that there is only one RF in the system */
    rwnx_hw->phy.cnt = 1;
#endif // CONFIG_RWNX_FHOST

    if (rwnx_plat_get_rf(rwnx_plat) != MDM_PHY_CONFIG_ELMA)
        /* No FCU for PHYs other than Elma */
        return 0;

    agcctl = RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM, RIU_RWNXAGCCNTL_ADDR);
    if (!__RIU_FCU_PRESENT(agcctl))
        /* No FCU present in this version */
        return 0;

#ifndef CONFIG_RWNX_FHOST
    /* FCU is present */
	#ifdef USE_5G
    rwnx_hw->phy.cnt = 2;
    rwnx_hw->phy.sec_chan.band = NL80211_BAND_5GHZ;
    rwnx_hw->phy.sec_chan.type = PHY_CHNL_BW_20;
    rwnx_hw->phy.sec_chan.prim20_freq = 5500;
    rwnx_hw->phy.sec_chan.center_freq1 = 5500;
    rwnx_hw->phy.sec_chan.center_freq2 = 0;
	#endif
#endif // CONFIG_RWNX_FHOST

    rwnx_plat_stop_agcfsm(rwnx_plat, FCU_RWNXFCAGCCNTL_ADDR, &agcctl, &memclk, 0,
                          MDM_MEMCLKCTRL0_ADDR);

    ret = rwnx_plat_bin_fw_upload(rwnx_plat,
                              RWNX_ADDR(rwnx_plat, RWNX_ADDR_SYSTEM, PHY_FCU_UCODE_ADDR),
                              RWNX_FCU_FW_NAME);

    rwnx_plat_start_agcfsm(rwnx_plat, FCU_RWNXFCAGCCNTL_ADDR, agcctl, memclk, 0,
                           MDM_MEMCLKCTRL0_ADDR);
#endif

    return ret;
}

/**
 * rwnx_is_new_agc_load() - Return is new agc clock register should be used
 *
 * @rwnx_plat: platform data
 * @rf: rf in used
 *
 * c.f Modem UM (AGC/CCA initialization)
 */
#ifndef CONFIG_RWNX_SDM
static u8 rwnx_get_agc_load_version(struct rwnx_plat *rwnx_plat, u32 rf, u32 *clkctrladdr)
{
    u8 agc_load_ver = 0;
    u32 agc_ver;
    u32 regval;

    /* Trident and Elma PHY use old method */
    if (rf !=  MDM_PHY_CONFIG_KARST) {
        *clkctrladdr = MDM_MEMCLKCTRL0_ADDR;
        return 0;
    }

    /* Get the FPGA signature */
    regval = RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM, SYSCTRL_SIGNATURE_ADDR);

    if (__FPGA_TYPE(regval) == 0xC0CA)
        *clkctrladdr = CRM_CLKGATEFCTRL0_ADDR;
    else
        *clkctrladdr = MDM_CLKGATEFCTRL0_ADDR;

    /* Read RIU version register */
    agc_ver = RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM, RIU_RWNXVERSION_ADDR);
    agc_load_ver = __RIU_AGCLOAD_FROM_VERS(agc_ver);

    return agc_load_ver;
}
#endif /* CONFIG_RWNX_SDM */

/**
 * rwnx_plat_agc_load() - Load AGC ucode
 *
 * @rwnx_plat: platform data
 * c.f Modem UM (AGC/CCA initialization)
 */
static int rwnx_plat_agc_load(struct rwnx_plat *rwnx_plat)
{
    int ret = 0;
#ifndef CONFIG_RWNX_SDM
    u32 agc = 0, agcctl, memclk;
    u32 clkctrladdr;
    u32 rf = rwnx_plat_get_rf(rwnx_plat);
    u8 agc_ver;

    switch (rf) {
        case MDM_PHY_CONFIG_TRIDENT:
            agc = AGC_RWNXAGCCNTL_ADDR;
            break;
        case MDM_PHY_CONFIG_ELMA:
        case MDM_PHY_CONFIG_KARST:
            agc = RIU_RWNXAGCCNTL_ADDR;
            break;
        default:
            return -1;
    }

    agc_ver = rwnx_get_agc_load_version(rwnx_plat, rf, &clkctrladdr);

    rwnx_plat_stop_agcfsm(rwnx_plat, agc, &agcctl, &memclk, agc_ver, clkctrladdr);

    ret = rwnx_plat_bin_fw_upload(rwnx_plat,
                              RWNX_ADDR(rwnx_plat, RWNX_ADDR_SYSTEM, PHY_AGC_UCODE_ADDR),
                              RWNX_AGC_FW_NAME);

    if (!ret && (agc_ver == 1)) {
        /* Run BIST to ensure that the AGC RAM was correctly loaded */
        RWNX_REG_WRITE(BIT(28), rwnx_plat, RWNX_ADDR_SYSTEM,
                       RIU_RWNXDYNAMICCONFIG_ADDR);
        while (RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM,
                             RIU_RWNXDYNAMICCONFIG_ADDR) & BIT(28));

        if (!(RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM,
                            RIU_AGCMEMBISTSTAT_ADDR) & BIT(0))) {
            dev_err(rwnx_platform_get_dev(rwnx_plat),
                    "AGC RAM not loaded correctly 0x%08x\n",
                    RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM,
                                  RIU_AGCMEMSIGNATURESTAT_ADDR));
            ret = -EIO;
        }
    }

    rwnx_plat_start_agcfsm(rwnx_plat, agc, agcctl, memclk, agc_ver, clkctrladdr);

#endif
    return ret;
}

/**
 * rwnx_ldpc_load() - Load LDPC RAM
 *
 * @rwnx_hw: Main driver data
 * c.f Modem UM (LDPC initialization)
 */
static int rwnx_ldpc_load(struct rwnx_hw *rwnx_hw)
{
#ifndef CONFIG_RWNX_SDM
    struct rwnx_plat *rwnx_plat = rwnx_hw->plat;
    u32 rf = rwnx_plat_get_rf(rwnx_plat);
    u32 phy_feat = RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM, MDM_HDMCONFIG_ADDR);

    if ((rf !=  MDM_PHY_CONFIG_KARST) ||
        (phy_feat & (MDM_LDPCDEC_BIT | MDM_LDPCENC_BIT)) !=
        (MDM_LDPCDEC_BIT | MDM_LDPCENC_BIT)) {
        goto disable_ldpc;
    }

    if (rwnx_plat_bin_fw_upload(rwnx_plat,
                            RWNX_ADDR(rwnx_plat, RWNX_ADDR_SYSTEM, PHY_LDPC_RAM_ADDR),
                            RWNX_LDPC_RAM_NAME)) {
        goto disable_ldpc;
    }

    return 0;

  disable_ldpc:
    rwnx_hw->mod_params->ldpc_on = false;

#endif /* CONFIG_RWNX_SDM */
    return 0;
}

/**
 * rwnx_plat_lmac_load() - Load FW code
 *
 * @rwnx_plat: platform data
 */
static int rwnx_plat_lmac_load(struct rwnx_plat *rwnx_plat)
{
    int ret;

    #ifdef CONFIG_RWNX_TL4
    ret = rwnx_plat_tl4_fw_upload(rwnx_plat,
                                  RWNX_ADDR(rwnx_plat, RWNX_ADDR_CPU, RAM_LMAC_FW_ADDR),
                                  RWNX_MAC_FW_NAME);
    #else
    ret = rwnx_plat_ihex_fw_upload(rwnx_plat,
                                   RWNX_ADDR(rwnx_plat, RWNX_ADDR_CPU, RAM_LMAC_FW_ADDR),
                                   RWNX_MAC_FW_NAME);
    if (ret == -ENOENT)
    {
        ret = rwnx_plat_bin_fw_upload(rwnx_plat,
                                      RWNX_ADDR(rwnx_plat, RWNX_ADDR_CPU, RAM_LMAC_FW_ADDR),
                                      RWNX_MAC_FW_NAME2);
    }
    #endif

    return ret;
}
#endif

#ifndef CONFIG_ROM_PATCH_EN
/**
 * rwnx_plat_fmac_load() - Load FW code
 *
 * @rwnx_hw: Main driver data
 */
#if 0
static int rwnx_plat_fmac_load(struct rwnx_hw *rwnx_hw)
{
    int ret;

    RWNX_DBG(RWNX_FN_ENTRY_STR);
    #if defined(CONFIG_NANOPI_M4) || defined(CONFIG_PLATFORM_ALLWINNER)
    ret = rwnx_plat_bin_fw_upload_android(rwnx_hw, RAM_FMAC_FW_ADDR, RWNX_MAC_FW_NAME2);
    #else
    ret = rwnx_plat_bin_fw_upload_2(rwnx_hw,
                                  RAM_FMAC_FW_ADDR,
                                  RWNX_MAC_FW_NAME2);
    #endif
    return ret;
}
#endif
#endif /* !CONFIG_ROM_PATCH_EN */

#if 0
/**
 * rwnx_plat_mpif_sel() - Select the MPIF according to the FPGA signature
 *
 * @rwnx_plat: platform data
 */
static void rwnx_plat_mpif_sel(struct rwnx_plat *rwnx_plat)
{
#ifndef CONFIG_RWNX_SDM
    u32 regval;
    u32 type;

    /* Get the FPGA signature */
    regval = RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM, SYSCTRL_SIGNATURE_ADDR);
    type = __FPGA_TYPE(regval);

    /* Check if we need to switch to the old MPIF or not */
    if ((type != 0xCAFE) && (type != 0XC0CA) && (regval & 0xF) < 0x3)
    {
        /* A old FPGA A is used, so configure the FPGA B to use the old MPIF */
        RWNX_REG_WRITE(0x3, rwnx_plat, RWNX_ADDR_SYSTEM, FPGAB_MPIF_SEL_ADDR);
    }
#endif
}
#endif
#if (defined(CONFIG_DPD) && !defined(CONFIG_FORCE_DPD_CALIB))
int is_file_exist(char* name)
{
    char *path = NULL;
    struct file *fp = NULL;
    int len;

    path = __getname();
    if (!path) {
        AICWFDBG(LOGINFO, "%s getname fail\n", __func__);
        return -1;
    }

    len = snprintf(path, FW_PATH_MAX_LEN, "%s/%s", aic_fw_path, name);

    fp = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        __putname(path);
        fp = NULL;
        return 0;
    } else {
        __putname(path);
        filp_close(fp, NULL);
		fp = NULL;
        return 1;
    }
}
#endif//CONFIG_DPD && !CONFIG_FORCE_DPD_CALIB
/**
 * rwnx_plat_patch_load() - Load patch code
 *
 * @rwnx_hw: Main driver data
 */
#ifdef CONFIG_ROM_PATCH_EN
static int rwnx_plat_patch_load(struct rwnx_hw *rwnx_hw)
{
    int ret = 0;

    RWNX_DBG(RWNX_FN_ENTRY_STR);

    if(rwnx_hw->usbdev->chipid == PRODUCT_ID_AIC8800DC ||
        rwnx_hw->usbdev->chipid == PRODUCT_ID_AIC8800DW){
#ifndef ANDROID_PLATFORM
        sprintf(aic_fw_path, "%s/%s", aic_fw_path, "aic8800DC");
#endif
        AICWFDBG(LOGINFO, "testmode=%d\n", testmode);
        if (chip_sub_id == 0) {
            if (testmode == FW_NORMAL_MODE) {
                ret = aicwf_plat_patch_load_8800dc(rwnx_hw);
                if (ret) {
                    AICWFDBG(LOGINFO, "load patch bin fail: %d\n", ret);
                    return ret;
                }
                aicwf_patch_config_8800dc(rwnx_hw);
            } else if (testmode == FW_RFTEST_MODE) {
                ret = aicwf_plat_rftest_load_8800dc(rwnx_hw);
                if (ret) {
                    AICWFDBG(LOGINFO, "load rftest bin fail: %d\n", ret);
                    return ret;
                }
            }
        } else if (chip_sub_id >= 1) {
            if (testmode == FW_NORMAL_MODE) {
                ret = aicwf_plat_patch_load_8800dc(rwnx_hw);
                if (ret) {
                    AICWFDBG(LOGINFO, "load patch bin fail: %d\n", ret);
                    return ret;
                }
                aicwf_patch_config_8800dc(rwnx_hw);
                #ifdef CONFIG_DPD
                #ifdef CONFIG_FORCE_DPD_CALIB
                if (1) {
                    AICWFDBG(LOGINFO, "dpd calib & write\n");
                    ret = aicwf_dpd_calib_8800dc(rwnx_hw, &dpd_res);
                    if (ret) {
                        AICWFDBG(LOGINFO, "dpd calib fail: %d\n", ret);
                        return ret;
                    }
                }
                #else
                if (is_file_exist(FW_DPDRESULT_NAME_8800DC) == 1) {
                    AICWFDBG(LOGINFO, "dpd bin load\n");
                    ret = aicwf_dpd_result_load_8800dc(rwnx_hw, &dpd_res);
                    if (ret) {
                        AICWFDBG(LOGINFO, "load dpd bin fail: %d\n", ret);
                        return ret;
                    }
                    ret = aicwf_dpd_result_apply_8800dc(rwnx_hw, &dpd_res);
                    if (ret) {
                        AICWFDBG(LOGINFO, "apply dpd bin fail: %d\n", ret);
                        return ret;
                    }
                }
                #endif
                else
                #elif defined(CONFIG_LOFT_CALIB)
                if (1) {
                    AICWFDBG(LOGINFO, "loft calib\n");
                    ret = aicwf_loft_calib_8800dc(rwnx_hw, &loft_res_local);
                    if (ret) {
                        AICWFDBG(LOGINFO, "loft calib fail: %d\n", ret);
                        return ret;
                    }
                } else
                #endif
                {
                    ret = aicwf_misc_ram_init_8800dc(rwnx_hw);
                    if (ret) {
                        AICWFDBG(LOGINFO, "misc ram init fail: %d\n", ret);
                        return ret;
                    }
                }
            } else if (testmode == FW_RFTEST_MODE) {
                #ifdef CONFIG_DPD
                #ifdef CONFIG_FORCE_DPD_CALIB
                if (1) {
                    AICWFDBG(LOGINFO, "patch load\n");
                    ret = aicwf_plat_patch_load_8800dc(rwnx_hw);
                    if (ret) {
                        AICWFDBG(LOGINFO, "load patch bin fail: %d\n", ret);
                        return ret;
                    }
                    //aicwf_patch_config_8800dc(rwnx_hw);
                    AICWFDBG(LOGINFO, "dpd calib & write\n");
                    ret = aicwf_dpd_calib_8800dc(rwnx_hw, &dpd_res);
                    if (ret) {
                        AICWFDBG(LOGINFO, "dpd calib fail: %d\n", ret);
                        return ret;
                    }
                }
                #endif/*CONFIG_FORCE_DPD_CALIB*/
                #elif defined(CONFIG_LOFT_CALIB)
                {
                    AICWFDBG(LOGINFO, "patch load\n");
                    ret = aicwf_plat_patch_load_8800dc(rwnx_hw);
                    if (ret) {
                        AICWFDBG(LOGINFO, "load patch bin fail: %d\n", ret);
                        return ret;
                    }
                    AICWFDBG(LOGINFO, "loft calib\n");
                    ret = aicwf_loft_calib_8800dc(rwnx_hw, &loft_res_local);
                    if (ret) {
                        AICWFDBG(LOGINFO, "loft calib fail: %d\n", ret);
                        return ret;
                    }
                }
                #endif/*CONFIG_DPD*/
                AICWFDBG(LOGINFO, "%s load rftest bin\n", __func__);
                ret = aicwf_plat_rftest_load_8800dc(rwnx_hw);
                if (ret) {
                    AICWFDBG(LOGINFO, "load rftest bin fail: %d\n", ret);
                    return ret;
                }
                /* Note: apply dpd_res after rftest running */
            } else if (testmode == FW_DPDCALIB_MODE) {
                #if (defined(CONFIG_DPD) && !defined(CONFIG_FORCE_DPD_CALIB))
                if (is_file_exist(FW_DPDRESULT_NAME_8800DC) == 0) {
                    AICWFDBG(LOGINFO, "patch load\n");
                    ret = aicwf_plat_patch_load_8800dc(rwnx_hw);
                    if (ret) {
                        AICWFDBG(LOGINFO, "load patch bin fail: %d\n", ret);
                        return ret;
                    }
                    //aicwf_patch_config_8800dc(rwnx_hw);
                    AICWFDBG(LOGINFO, "dpd calib & write\n");
                    ret = aicwf_dpd_calib_8800dc(rwnx_hw, &dpd_res);
                    if (ret) {
                        AICWFDBG(LOGINFO, "dpd calib fail: %d\n", ret);
                        return ret;
                    }
                    ret = aicwf_dpd_result_write_8800dc((void *)&dpd_res, DPD_RESULT_SIZE_8800DC);
                    if (ret) {
                        AICWFDBG(LOGINFO, "file write fail: %d\n", ret);
                        return ret;
                    }
                }
                #endif
                return 1; // exit calib mode
            }
        }
    }

    return ret;
}
#endif


/**
 * rwnx_platform_reset() - Reset the platform
 *
 * @rwnx_plat: platform data
 */
static int rwnx_platform_reset(struct rwnx_plat *rwnx_plat)
{
    u32 regval;

#if defined(AICWF_USB_SUPPORT) || defined(AICWF_SDIO_SUPPORT)
    return 0;
#endif

    /* the doc states that SOFT implies FPGA_B_RESET
     * adding FPGA_B_RESET is clearer */
    RWNX_REG_WRITE(SOFT_RESET | FPGA_B_RESET, rwnx_plat,
                   RWNX_ADDR_SYSTEM, SYSCTRL_MISC_CNTL_ADDR);
    msleep(100);

    regval = RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM, SYSCTRL_MISC_CNTL_ADDR);

    if (regval & SOFT_RESET) {
        dev_err(rwnx_platform_get_dev(rwnx_plat), "reset: failed\n");
        return -EIO;
    }

    RWNX_REG_WRITE(regval & ~FPGA_B_RESET, rwnx_plat,
                   RWNX_ADDR_SYSTEM, SYSCTRL_MISC_CNTL_ADDR);
    msleep(100);
    return 0;
}

/**
 * rwmx_platform_save_config() - Save hardware config before reload
 *
 * @rwnx_plat: Pointer to platform data
 *
 * Return configuration registers values.
 */
static void* rwnx_term_save_config(struct rwnx_plat *rwnx_plat)
{
    const u32 *reg_list;
    u32 *reg_value, *res;
    int i, size = 0;

    if (rwnx_plat->get_config_reg) {
        size = rwnx_plat->get_config_reg(rwnx_plat, &reg_list);
    }

    if (size <= 0)
        return NULL;

    res = kmalloc(sizeof(u32) * size, GFP_KERNEL);
    if (!res)
        return NULL;

    reg_value = res;
    for (i = 0; i < size; i++) {
        *reg_value++ = RWNX_REG_READ(rwnx_plat, RWNX_ADDR_SYSTEM,
                                     *reg_list++);
    }

    return res;
}

#if 0
/**
 * rwmx_platform_restore_config() - Restore hardware config after reload
 *
 * @rwnx_plat: Pointer to platform data
 * @reg_value: Pointer of value to restore
 * (obtained with rwmx_platform_save_config())
 *
 * Restore configuration registers value.
 */
static void rwnx_term_restore_config(struct rwnx_plat *rwnx_plat,
                                     u32 *reg_value)
{
    const u32 *reg_list;
    int i, size = 0;

    if (!reg_value || !rwnx_plat->get_config_reg)
        return;

    size = rwnx_plat->get_config_reg(rwnx_plat, &reg_list);

    for (i = 0; i < size; i++) {
        RWNX_REG_WRITE(*reg_value++, rwnx_plat, RWNX_ADDR_SYSTEM,
                       *reg_list++);
    }
}
#endif

#ifndef CONFIG_RWNX_FHOST
#if 0
static int rwnx_check_fw_compatibility(struct rwnx_hw *rwnx_hw)
{
    struct ipc_shared_env_tag *shared = rwnx_hw->ipc_env->shared;
    #ifdef CONFIG_RWNX_FULLMAC
    struct wiphy *wiphy = rwnx_hw->wiphy;
    #endif //CONFIG_RWNX_FULLMAC
    #ifdef CONFIG_RWNX_OLD_IPC
    int ipc_shared_version = 10;
    #else //CONFIG_RWNX_OLD_IPC
    int ipc_shared_version = 11;
    #endif //CONFIG_RWNX_OLD_IPC
    int res = 0;

    if(shared->comp_info.ipc_shared_version != ipc_shared_version)
    {
        wiphy_err(wiphy, "Different versions of IPC shared version between driver and FW (%d != %d)\n ",
                  ipc_shared_version, shared->comp_info.ipc_shared_version);
        res = -1;
    }

    if(shared->comp_info.radarbuf_cnt != IPC_RADARBUF_CNT)
    {
        wiphy_err(wiphy, "Different number of host buffers available for Radar events handling "\
                  "between driver and FW (%d != %d)\n", IPC_RADARBUF_CNT,
                  shared->comp_info.radarbuf_cnt);
        res = -1;
    }

    if(shared->comp_info.unsuprxvecbuf_cnt != IPC_UNSUPRXVECBUF_CNT)
    {
        wiphy_err(wiphy, "Different number of host buffers available for unsupported Rx vectors "\
                  "handling between driver and FW (%d != %d)\n", IPC_UNSUPRXVECBUF_CNT,
                  shared->comp_info.unsuprxvecbuf_cnt);
        res = -1;
    }

    #ifdef CONFIG_RWNX_FULLMAC
    if(shared->comp_info.rxdesc_cnt != IPC_RXDESC_CNT)
    {
        wiphy_err(wiphy, "Different number of shared descriptors available for Data RX handling "\
                  "between driver and FW (%d != %d)\n", IPC_RXDESC_CNT,
                  shared->comp_info.rxdesc_cnt);
        res = -1;
    }
    #endif /* CONFIG_RWNX_FULLMAC */

    if(shared->comp_info.rxbuf_cnt != IPC_RXBUF_CNT)
    {
        wiphy_err(wiphy, "Different number of host buffers available for Data Rx handling "\
                  "between driver and FW (%d != %d)\n", IPC_RXBUF_CNT,
                  shared->comp_info.rxbuf_cnt);
        res = -1;
    }

    if(shared->comp_info.msge2a_buf_cnt != IPC_MSGE2A_BUF_CNT)
    {
        wiphy_err(wiphy, "Different number of host buffers available for Emb->App MSGs "\
                  "sending between driver and FW (%d != %d)\n", IPC_MSGE2A_BUF_CNT,
                  shared->comp_info.msge2a_buf_cnt);
        res = -1;
    }

    if(shared->comp_info.dbgbuf_cnt != IPC_DBGBUF_CNT)
    {
        wiphy_err(wiphy, "Different number of host buffers available for debug messages "\
                  "sending between driver and FW (%d != %d)\n", IPC_DBGBUF_CNT,
                  shared->comp_info.dbgbuf_cnt);
        res = -1;
    }

    if(shared->comp_info.bk_txq != NX_TXDESC_CNT0)
    {
        wiphy_err(wiphy, "Driver and FW have different sizes of BK TX queue (%d != %d)\n",
                  NX_TXDESC_CNT0, shared->comp_info.bk_txq);
        res = -1;
    }

    if(shared->comp_info.be_txq != NX_TXDESC_CNT1)
    {
        wiphy_err(wiphy, "Driver and FW have different sizes of BE TX queue (%d != %d)\n",
                  NX_TXDESC_CNT1, shared->comp_info.be_txq);
        res = -1;
    }

    if(shared->comp_info.vi_txq != NX_TXDESC_CNT2)
    {
        wiphy_err(wiphy, "Driver and FW have different sizes of VI TX queue (%d != %d)\n",
                  NX_TXDESC_CNT2, shared->comp_info.vi_txq);
        res = -1;
    }

    if(shared->comp_info.vo_txq != NX_TXDESC_CNT3)
    {
        wiphy_err(wiphy, "Driver and FW have different sizes of VO TX queue (%d != %d)\n",
                  NX_TXDESC_CNT3, shared->comp_info.vo_txq);
        res = -1;
    }

    #if NX_TXQ_CNT == 5
    if(shared->comp_info.bcn_txq != NX_TXDESC_CNT4)
    {
        wiphy_err(wiphy, "Driver and FW have different sizes of BCN TX queue (%d != %d)\n",
                NX_TXDESC_CNT4, shared->comp_info.bcn_txq);
        res = -1;
    }
    #else
    if (shared->comp_info.bcn_txq > 0)
    {
        wiphy_err(wiphy, "BCMC enabled in firmware but disabled in driver\n");
        res = -1;
    }
    #endif /* NX_TXQ_CNT == 5 */

    if(shared->comp_info.ipc_shared_size != sizeof(ipc_shared_env))
    {
        wiphy_err(wiphy, "Different sizes of IPC shared between driver and FW (%zd != %d)\n",
                  sizeof(ipc_shared_env), shared->comp_info.ipc_shared_size);
        res = -1;
    }

    if(shared->comp_info.msg_api != MSG_API_VER)
    {
        wiphy_warn(wiphy, "WARNING: Different supported message API versions between "\
                   "driver and FW (%d != %d)\n", MSG_API_VER, shared->comp_info.msg_api);
    }

    return res;
}
#endif
#endif /* !CONFIG_RWNX_FHOST */

int rwnx_atoi2(char *value, int c_len)
{
    int len = 0;
    int i = 0;
    int result = 0;
    int flag = 1;

    if (value[0] == '-') {
        flag = -1;
        value++;
    }
    len = c_len;

    for (i = 0;i < len ;i++) {
        result = result * 10;
        if (value[i] >= 48 && value[i] <= 57) {
            result += value[i] - 48;
        } else {
            result = 0;
            break;
        }
    }

    return result * flag;
}


int rwnx_atoi(char *value)
{
    int len = 0;
    int i = 0;
    int result = 0;
    int flag = 1;

    if (value[0] == '-') {
        flag = -1;
        value++;
    }
    len = strlen(value);

    for (i = 0;i < len ;i++) {
        result = result * 10;
        if (value[i] >= 48 && value[i] <= 57) {
            result += value[i] - 48;
        } else {
            result = 0;
            break;
        }
    }

    return result * flag;
}

void get_userconfig_txpwr_lvl_in_fdrv(txpwr_lvl_conf_t *txpwr_lvl)
{
    txpwr_lvl->enable           = userconfig_info.txpwr_lvl.enable;
    txpwr_lvl->dsss             = userconfig_info.txpwr_lvl.dsss;
    txpwr_lvl->ofdmlowrate_2g4  = userconfig_info.txpwr_lvl.ofdmlowrate_2g4;
    txpwr_lvl->ofdm64qam_2g4    = userconfig_info.txpwr_lvl.ofdm64qam_2g4;
    txpwr_lvl->ofdm256qam_2g4   = userconfig_info.txpwr_lvl.ofdm256qam_2g4;
    txpwr_lvl->ofdm1024qam_2g4  = userconfig_info.txpwr_lvl.ofdm1024qam_2g4;
    txpwr_lvl->ofdmlowrate_5g   = userconfig_info.txpwr_lvl.ofdmlowrate_5g;
    txpwr_lvl->ofdm64qam_5g     = userconfig_info.txpwr_lvl.ofdm64qam_5g;
    txpwr_lvl->ofdm256qam_5g    = userconfig_info.txpwr_lvl.ofdm256qam_5g;
    txpwr_lvl->ofdm1024qam_5g   = userconfig_info.txpwr_lvl.ofdm1024qam_5g;

    AICWFDBG(LOGINFO, "%s:enable:%d\r\n",          __func__, txpwr_lvl->enable);
    AICWFDBG(LOGINFO, "%s:dsss:%d\r\n",            __func__, txpwr_lvl->dsss);
    AICWFDBG(LOGINFO, "%s:ofdmlowrate_2g4:%d\r\n", __func__, txpwr_lvl->ofdmlowrate_2g4);
    AICWFDBG(LOGINFO, "%s:ofdm64qam_2g4:%d\r\n",   __func__, txpwr_lvl->ofdm64qam_2g4);
    AICWFDBG(LOGINFO, "%s:ofdm256qam_2g4:%d\r\n",  __func__, txpwr_lvl->ofdm256qam_2g4);
    AICWFDBG(LOGINFO, "%s:ofdm1024qam_2g4:%d\r\n", __func__, txpwr_lvl->ofdm1024qam_2g4);
    AICWFDBG(LOGINFO, "%s:ofdmlowrate_5g:%d\r\n",  __func__, txpwr_lvl->ofdmlowrate_5g);
    AICWFDBG(LOGINFO, "%s:ofdm64qam_5g:%d\r\n",    __func__, txpwr_lvl->ofdm64qam_5g);
    AICWFDBG(LOGINFO, "%s:ofdm256qam_5g:%d\r\n",   __func__, txpwr_lvl->ofdm256qam_5g);
    AICWFDBG(LOGINFO, "%s:ofdm1024qam_5g:%d\r\n",  __func__, txpwr_lvl->ofdm1024qam_5g);
}

void get_userconfig_txpwr_lvl_v2_in_fdrv(txpwr_lvl_conf_v2_t *txpwr_lvl_v2)
{
    *txpwr_lvl_v2 = userconfig_info.txpwr_lvl_v2;

    AICWFDBG(LOGINFO, "%s:enable:%d\r\n",               __func__, txpwr_lvl_v2->enable);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_1m_2g4:%d\r\n",  __func__, txpwr_lvl_v2->pwrlvl_11b_11ag_2g4[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_2m_2g4:%d\r\n",  __func__, txpwr_lvl_v2->pwrlvl_11b_11ag_2g4[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_5m5_2g4:%d\r\n", __func__, txpwr_lvl_v2->pwrlvl_11b_11ag_2g4[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_11m_2g4:%d\r\n", __func__, txpwr_lvl_v2->pwrlvl_11b_11ag_2g4[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_6m_2g4:%d\r\n",  __func__, txpwr_lvl_v2->pwrlvl_11b_11ag_2g4[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_9m_2g4:%d\r\n",  __func__, txpwr_lvl_v2->pwrlvl_11b_11ag_2g4[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_12m_2g4:%d\r\n", __func__, txpwr_lvl_v2->pwrlvl_11b_11ag_2g4[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_18m_2g4:%d\r\n", __func__, txpwr_lvl_v2->pwrlvl_11b_11ag_2g4[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_24m_2g4:%d\r\n", __func__, txpwr_lvl_v2->pwrlvl_11b_11ag_2g4[8]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_36m_2g4:%d\r\n", __func__, txpwr_lvl_v2->pwrlvl_11b_11ag_2g4[9]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_48m_2g4:%d\r\n", __func__, txpwr_lvl_v2->pwrlvl_11b_11ag_2g4[10]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_54m_2g4:%d\r\n", __func__, txpwr_lvl_v2->pwrlvl_11b_11ag_2g4[11]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs0_2g4:%d\r\n",__func__, txpwr_lvl_v2->pwrlvl_11n_11ac_2g4[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs1_2g4:%d\r\n",__func__, txpwr_lvl_v2->pwrlvl_11n_11ac_2g4[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs2_2g4:%d\r\n",__func__, txpwr_lvl_v2->pwrlvl_11n_11ac_2g4[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs3_2g4:%d\r\n",__func__, txpwr_lvl_v2->pwrlvl_11n_11ac_2g4[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs4_2g4:%d\r\n",__func__, txpwr_lvl_v2->pwrlvl_11n_11ac_2g4[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs5_2g4:%d\r\n",__func__, txpwr_lvl_v2->pwrlvl_11n_11ac_2g4[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs6_2g4:%d\r\n",__func__, txpwr_lvl_v2->pwrlvl_11n_11ac_2g4[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs7_2g4:%d\r\n",__func__, txpwr_lvl_v2->pwrlvl_11n_11ac_2g4[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs8_2g4:%d\r\n",__func__, txpwr_lvl_v2->pwrlvl_11n_11ac_2g4[8]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs9_2g4:%d\r\n",__func__, txpwr_lvl_v2->pwrlvl_11n_11ac_2g4[9]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs0_2g4:%d\r\n",    __func__, txpwr_lvl_v2->pwrlvl_11ax_2g4[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs1_2g4:%d\r\n",    __func__, txpwr_lvl_v2->pwrlvl_11ax_2g4[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs2_2g4:%d\r\n",    __func__, txpwr_lvl_v2->pwrlvl_11ax_2g4[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs3_2g4:%d\r\n",    __func__, txpwr_lvl_v2->pwrlvl_11ax_2g4[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs4_2g4:%d\r\n",    __func__, txpwr_lvl_v2->pwrlvl_11ax_2g4[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs5_2g4:%d\r\n",    __func__, txpwr_lvl_v2->pwrlvl_11ax_2g4[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs6_2g4:%d\r\n",    __func__, txpwr_lvl_v2->pwrlvl_11ax_2g4[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs7_2g4:%d\r\n",    __func__, txpwr_lvl_v2->pwrlvl_11ax_2g4[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs8_2g4:%d\r\n",    __func__, txpwr_lvl_v2->pwrlvl_11ax_2g4[8]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs9_2g4:%d\r\n",    __func__, txpwr_lvl_v2->pwrlvl_11ax_2g4[9]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs10_2g4:%d\r\n",   __func__, txpwr_lvl_v2->pwrlvl_11ax_2g4[10]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs11_2g4:%d\r\n",   __func__, txpwr_lvl_v2->pwrlvl_11ax_2g4[11]);
}

void get_userconfig_txpwr_lvl_v3_in_fdrv(txpwr_lvl_conf_v3_t *txpwr_lvl_v3)
{
    *txpwr_lvl_v3 = userconfig_info.txpwr_lvl_v3;

    AICWFDBG(LOGINFO, "%s:enable:%d\r\n",               __func__, txpwr_lvl_v3->enable);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_1m_2g4:%d\r\n",  __func__, txpwr_lvl_v3->pwrlvl_11b_11ag_2g4[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_2m_2g4:%d\r\n",  __func__, txpwr_lvl_v3->pwrlvl_11b_11ag_2g4[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_5m5_2g4:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11b_11ag_2g4[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_11m_2g4:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11b_11ag_2g4[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_6m_2g4:%d\r\n",  __func__, txpwr_lvl_v3->pwrlvl_11b_11ag_2g4[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_9m_2g4:%d\r\n",  __func__, txpwr_lvl_v3->pwrlvl_11b_11ag_2g4[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_12m_2g4:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11b_11ag_2g4[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_18m_2g4:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11b_11ag_2g4[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_24m_2g4:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11b_11ag_2g4[8]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_36m_2g4:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11b_11ag_2g4[9]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_48m_2g4:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11b_11ag_2g4[10]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_54m_2g4:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11b_11ag_2g4[11]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs0_2g4:%d\r\n",__func__, txpwr_lvl_v3->pwrlvl_11n_11ac_2g4[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs1_2g4:%d\r\n",__func__, txpwr_lvl_v3->pwrlvl_11n_11ac_2g4[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs2_2g4:%d\r\n",__func__, txpwr_lvl_v3->pwrlvl_11n_11ac_2g4[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs3_2g4:%d\r\n",__func__, txpwr_lvl_v3->pwrlvl_11n_11ac_2g4[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs4_2g4:%d\r\n",__func__, txpwr_lvl_v3->pwrlvl_11n_11ac_2g4[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs5_2g4:%d\r\n",__func__, txpwr_lvl_v3->pwrlvl_11n_11ac_2g4[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs6_2g4:%d\r\n",__func__, txpwr_lvl_v3->pwrlvl_11n_11ac_2g4[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs7_2g4:%d\r\n",__func__, txpwr_lvl_v3->pwrlvl_11n_11ac_2g4[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs8_2g4:%d\r\n",__func__, txpwr_lvl_v3->pwrlvl_11n_11ac_2g4[8]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs9_2g4:%d\r\n",__func__, txpwr_lvl_v3->pwrlvl_11n_11ac_2g4[9]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs0_2g4:%d\r\n",    __func__, txpwr_lvl_v3->pwrlvl_11ax_2g4[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs1_2g4:%d\r\n",    __func__, txpwr_lvl_v3->pwrlvl_11ax_2g4[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs2_2g4:%d\r\n",    __func__, txpwr_lvl_v3->pwrlvl_11ax_2g4[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs3_2g4:%d\r\n",    __func__, txpwr_lvl_v3->pwrlvl_11ax_2g4[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs4_2g4:%d\r\n",    __func__, txpwr_lvl_v3->pwrlvl_11ax_2g4[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs5_2g4:%d\r\n",    __func__, txpwr_lvl_v3->pwrlvl_11ax_2g4[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs6_2g4:%d\r\n",    __func__, txpwr_lvl_v3->pwrlvl_11ax_2g4[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs7_2g4:%d\r\n",    __func__, txpwr_lvl_v3->pwrlvl_11ax_2g4[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs8_2g4:%d\r\n",    __func__, txpwr_lvl_v3->pwrlvl_11ax_2g4[8]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs9_2g4:%d\r\n",    __func__, txpwr_lvl_v3->pwrlvl_11ax_2g4[9]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs10_2g4:%d\r\n",   __func__, txpwr_lvl_v3->pwrlvl_11ax_2g4[10]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs11_2g4:%d\r\n",   __func__, txpwr_lvl_v3->pwrlvl_11ax_2g4[11]);

    AICWFDBG(LOGINFO, "%s:lvl_11a_1m_5g:%d\r\n",        __func__, txpwr_lvl_v3->pwrlvl_11a_5g[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_2m_5g:%d\r\n",        __func__, txpwr_lvl_v3->pwrlvl_11a_5g[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_5m5_5g:%d\r\n",       __func__, txpwr_lvl_v3->pwrlvl_11a_5g[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_11m_5g:%d\r\n",       __func__, txpwr_lvl_v3->pwrlvl_11a_5g[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_6m_5g:%d\r\n",        __func__, txpwr_lvl_v3->pwrlvl_11a_5g[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_9m_5g:%d\r\n",        __func__, txpwr_lvl_v3->pwrlvl_11a_5g[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_12m_5g:%d\r\n",       __func__, txpwr_lvl_v3->pwrlvl_11a_5g[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_18m_5g:%d\r\n",       __func__, txpwr_lvl_v3->pwrlvl_11a_5g[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_24m_5g:%d\r\n",       __func__, txpwr_lvl_v3->pwrlvl_11a_5g[8]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_36m_5g:%d\r\n",       __func__, txpwr_lvl_v3->pwrlvl_11a_5g[9]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_48m_5g:%d\r\n",       __func__, txpwr_lvl_v3->pwrlvl_11a_5g[10]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_54m_5g:%d\r\n",       __func__, txpwr_lvl_v3->pwrlvl_11a_5g[11]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs0_5g:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11n_11ac_5g[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs1_5g:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11n_11ac_5g[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs2_5g:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11n_11ac_5g[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs3_5g:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11n_11ac_5g[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs4_5g:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11n_11ac_5g[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs5_5g:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11n_11ac_5g[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs6_5g:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11n_11ac_5g[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs7_5g:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11n_11ac_5g[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs8_5g:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11n_11ac_5g[8]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs9_5g:%d\r\n", __func__, txpwr_lvl_v3->pwrlvl_11n_11ac_5g[9]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs0_5g:%d\r\n",     __func__, txpwr_lvl_v3->pwrlvl_11ax_5g[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs1_5g:%d\r\n",     __func__, txpwr_lvl_v3->pwrlvl_11ax_5g[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs2_5g:%d\r\n",     __func__, txpwr_lvl_v3->pwrlvl_11ax_5g[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs3_5g:%d\r\n",     __func__, txpwr_lvl_v3->pwrlvl_11ax_5g[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs4_5g:%d\r\n",     __func__, txpwr_lvl_v3->pwrlvl_11ax_5g[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs5_5g:%d\r\n",     __func__, txpwr_lvl_v3->pwrlvl_11ax_5g[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs6_5g:%d\r\n",     __func__, txpwr_lvl_v3->pwrlvl_11ax_5g[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs7_5g:%d\r\n",     __func__, txpwr_lvl_v3->pwrlvl_11ax_5g[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs8_5g:%d\r\n",     __func__, txpwr_lvl_v3->pwrlvl_11ax_5g[8]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs9_5g:%d\r\n",     __func__, txpwr_lvl_v3->pwrlvl_11ax_5g[9]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs10_5g:%d\r\n",    __func__, txpwr_lvl_v3->pwrlvl_11ax_5g[10]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs11_5g:%d\r\n",    __func__, txpwr_lvl_v3->pwrlvl_11ax_5g[11]);
}

void get_userconfig_txpwr_lvl_v4_in_fdrv(txpwr_lvl_conf_v4_t *txpwr_lvl_v4)
{
    *txpwr_lvl_v4 = userconfig_info.txpwr_lvl_v4;

    AICWFDBG(LOGINFO, "%s:enable:%d\r\n",               __func__, txpwr_lvl_v4->enable);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_1m_2g4:%d\r\n",  __func__, txpwr_lvl_v4->pwrlvl_11b_11ag_2g4[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_2m_2g4:%d\r\n",  __func__, txpwr_lvl_v4->pwrlvl_11b_11ag_2g4[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_5m5_2g4:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11b_11ag_2g4[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_11m_2g4:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11b_11ag_2g4[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_6m_2g4:%d\r\n",  __func__, txpwr_lvl_v4->pwrlvl_11b_11ag_2g4[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_9m_2g4:%d\r\n",  __func__, txpwr_lvl_v4->pwrlvl_11b_11ag_2g4[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_12m_2g4:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11b_11ag_2g4[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_18m_2g4:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11b_11ag_2g4[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_24m_2g4:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11b_11ag_2g4[8]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_36m_2g4:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11b_11ag_2g4[9]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_48m_2g4:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11b_11ag_2g4[10]);
    AICWFDBG(LOGINFO, "%s:lvl_11b_11ag_54m_2g4:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11b_11ag_2g4[11]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs0_2g4:%d\r\n",__func__, txpwr_lvl_v4->pwrlvl_11n_11ac_2g4[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs1_2g4:%d\r\n",__func__, txpwr_lvl_v4->pwrlvl_11n_11ac_2g4[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs2_2g4:%d\r\n",__func__, txpwr_lvl_v4->pwrlvl_11n_11ac_2g4[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs3_2g4:%d\r\n",__func__, txpwr_lvl_v4->pwrlvl_11n_11ac_2g4[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs4_2g4:%d\r\n",__func__, txpwr_lvl_v4->pwrlvl_11n_11ac_2g4[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs5_2g4:%d\r\n",__func__, txpwr_lvl_v4->pwrlvl_11n_11ac_2g4[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs6_2g4:%d\r\n",__func__, txpwr_lvl_v4->pwrlvl_11n_11ac_2g4[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs7_2g4:%d\r\n",__func__, txpwr_lvl_v4->pwrlvl_11n_11ac_2g4[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs8_2g4:%d\r\n",__func__, txpwr_lvl_v4->pwrlvl_11n_11ac_2g4[8]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs9_2g4:%d\r\n",__func__, txpwr_lvl_v4->pwrlvl_11n_11ac_2g4[9]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs0_2g4:%d\r\n",    __func__, txpwr_lvl_v4->pwrlvl_11ax_2g4[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs1_2g4:%d\r\n",    __func__, txpwr_lvl_v4->pwrlvl_11ax_2g4[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs2_2g4:%d\r\n",    __func__, txpwr_lvl_v4->pwrlvl_11ax_2g4[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs3_2g4:%d\r\n",    __func__, txpwr_lvl_v4->pwrlvl_11ax_2g4[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs4_2g4:%d\r\n",    __func__, txpwr_lvl_v4->pwrlvl_11ax_2g4[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs5_2g4:%d\r\n",    __func__, txpwr_lvl_v4->pwrlvl_11ax_2g4[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs6_2g4:%d\r\n",    __func__, txpwr_lvl_v4->pwrlvl_11ax_2g4[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs7_2g4:%d\r\n",    __func__, txpwr_lvl_v4->pwrlvl_11ax_2g4[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs8_2g4:%d\r\n",    __func__, txpwr_lvl_v4->pwrlvl_11ax_2g4[8]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs9_2g4:%d\r\n",    __func__, txpwr_lvl_v4->pwrlvl_11ax_2g4[9]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs10_2g4:%d\r\n",   __func__, txpwr_lvl_v4->pwrlvl_11ax_2g4[10]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs11_2g4:%d\r\n",   __func__, txpwr_lvl_v4->pwrlvl_11ax_2g4[11]);

    AICWFDBG(LOGINFO, "%s:lvl_11a_6m_5g:%d\r\n",        __func__, txpwr_lvl_v4->pwrlvl_11a_5g[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_9m_5g:%d\r\n",        __func__, txpwr_lvl_v4->pwrlvl_11a_5g[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_12m_5g:%d\r\n",       __func__, txpwr_lvl_v4->pwrlvl_11a_5g[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_18m_5g:%d\r\n",       __func__, txpwr_lvl_v4->pwrlvl_11a_5g[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_24m_5g:%d\r\n",       __func__, txpwr_lvl_v4->pwrlvl_11a_5g[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_36m_5g:%d\r\n",       __func__, txpwr_lvl_v4->pwrlvl_11a_5g[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_48m_5g:%d\r\n",       __func__, txpwr_lvl_v4->pwrlvl_11a_5g[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11a_54m_5g:%d\r\n",       __func__, txpwr_lvl_v4->pwrlvl_11a_5g[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs0_5g:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11n_11ac_5g[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs1_5g:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11n_11ac_5g[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs2_5g:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11n_11ac_5g[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs3_5g:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11n_11ac_5g[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs4_5g:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11n_11ac_5g[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs5_5g:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11n_11ac_5g[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs6_5g:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11n_11ac_5g[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs7_5g:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11n_11ac_5g[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs8_5g:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11n_11ac_5g[8]);
    AICWFDBG(LOGINFO, "%s:lvl_11n_11ac_mcs9_5g:%d\r\n", __func__, txpwr_lvl_v4->pwrlvl_11n_11ac_5g[9]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs0_5g:%d\r\n",     __func__, txpwr_lvl_v4->pwrlvl_11ax_5g[0]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs1_5g:%d\r\n",     __func__, txpwr_lvl_v4->pwrlvl_11ax_5g[1]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs2_5g:%d\r\n",     __func__, txpwr_lvl_v4->pwrlvl_11ax_5g[2]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs3_5g:%d\r\n",     __func__, txpwr_lvl_v4->pwrlvl_11ax_5g[3]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs4_5g:%d\r\n",     __func__, txpwr_lvl_v4->pwrlvl_11ax_5g[4]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs5_5g:%d\r\n",     __func__, txpwr_lvl_v4->pwrlvl_11ax_5g[5]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs6_5g:%d\r\n",     __func__, txpwr_lvl_v4->pwrlvl_11ax_5g[6]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs7_5g:%d\r\n",     __func__, txpwr_lvl_v4->pwrlvl_11ax_5g[7]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs8_5g:%d\r\n",     __func__, txpwr_lvl_v4->pwrlvl_11ax_5g[8]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs9_5g:%d\r\n",     __func__, txpwr_lvl_v4->pwrlvl_11ax_5g[9]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs10_5g:%d\r\n",    __func__, txpwr_lvl_v4->pwrlvl_11ax_5g[10]);
    AICWFDBG(LOGINFO, "%s:lvl_11ax_mcs11_5g:%d\r\n",    __func__, txpwr_lvl_v4->pwrlvl_11ax_5g[11]);
}

void get_userconfig_txpwr_lvl_adj_in_fdrv(txpwr_lvl_adj_conf_t *txpwr_lvl_adj)
{
    *txpwr_lvl_adj = userconfig_info.txpwr_lvl_adj;

    AICWFDBG(LOGINFO, "%s:enable:%d\r\n",                   __func__, txpwr_lvl_adj->enable);
    AICWFDBG(LOGINFO, "%s:lvl_adj_2g4_chan_1_4:%d\r\n",     __func__, txpwr_lvl_adj->pwrlvl_adj_tbl_2g4[0]);
    AICWFDBG(LOGINFO, "%s:lvl_adj_2g4_chan_5_9:%d\r\n",     __func__, txpwr_lvl_adj->pwrlvl_adj_tbl_2g4[1]);
    AICWFDBG(LOGINFO, "%s:lvl_adj_2g4_chan_10_13:%d\r\n",   __func__, txpwr_lvl_adj->pwrlvl_adj_tbl_2g4[2]);

    AICWFDBG(LOGINFO, "%s:lvl_adj_5g_chan_42:%d\r\n",       __func__, txpwr_lvl_adj->pwrlvl_adj_tbl_5g[0]);
    AICWFDBG(LOGINFO, "%s:lvl_adj_5g_chan_58:%d\r\n",       __func__, txpwr_lvl_adj->pwrlvl_adj_tbl_5g[1]);
    AICWFDBG(LOGINFO, "%s:lvl_adj_5g_chan_106:%d\r\n",      __func__, txpwr_lvl_adj->pwrlvl_adj_tbl_5g[2]);
    AICWFDBG(LOGINFO, "%s:lvl_adj_5g_chan_122:%d\r\n",      __func__, txpwr_lvl_adj->pwrlvl_adj_tbl_5g[3]);
    AICWFDBG(LOGINFO, "%s:lvl_adj_5g_chan_138:%d\r\n",      __func__, txpwr_lvl_adj->pwrlvl_adj_tbl_5g[4]);
    AICWFDBG(LOGINFO, "%s:lvl_adj_5g_chan_155:%d\r\n",      __func__, txpwr_lvl_adj->pwrlvl_adj_tbl_5g[5]);
}


void get_userconfig_txpwr_ofst_in_fdrv(txpwr_ofst_conf_t *txpwr_ofst)
{
    txpwr_ofst->enable       = userconfig_info.txpwr_ofst.enable;
    txpwr_ofst->chan_1_4     = userconfig_info.txpwr_ofst.chan_1_4;
    txpwr_ofst->chan_5_9     = userconfig_info.txpwr_ofst.chan_5_9;
    txpwr_ofst->chan_10_13   = userconfig_info.txpwr_ofst.chan_10_13;
    txpwr_ofst->chan_36_64   = userconfig_info.txpwr_ofst.chan_36_64;
    txpwr_ofst->chan_100_120 = userconfig_info.txpwr_ofst.chan_100_120;
    txpwr_ofst->chan_122_140 = userconfig_info.txpwr_ofst.chan_122_140;
    txpwr_ofst->chan_142_165 = userconfig_info.txpwr_ofst.chan_142_165;

    AICWFDBG(LOGINFO, "%s:enable      :%d\r\n", __func__, txpwr_ofst->enable);
    AICWFDBG(LOGINFO, "%s:chan_1_4    :%d\r\n", __func__, txpwr_ofst->chan_1_4);
    AICWFDBG(LOGINFO, "%s:chan_5_9    :%d\r\n", __func__, txpwr_ofst->chan_5_9);
    AICWFDBG(LOGINFO, "%s:chan_10_13  :%d\r\n", __func__, txpwr_ofst->chan_10_13);
    AICWFDBG(LOGINFO, "%s:chan_36_64  :%d\r\n", __func__, txpwr_ofst->chan_36_64);
    AICWFDBG(LOGINFO, "%s:chan_100_120:%d\r\n", __func__, txpwr_ofst->chan_100_120);
    AICWFDBG(LOGINFO, "%s:chan_122_140:%d\r\n", __func__, txpwr_ofst->chan_122_140);
    AICWFDBG(LOGINFO, "%s:chan_142_165:%d\r\n", __func__, txpwr_ofst->chan_142_165);
}

void get_userconfig_txpwr_ofst2x_in_fdrv(txpwr_ofst2x_conf_t *txpwr_ofst2x)
{
    int type, ch_grp;
    *txpwr_ofst2x = userconfig_info.txpwr_ofst2x;
    AICWFDBG(LOGINFO, "%s:enable      :%d\r\n", __func__, txpwr_ofst2x->enable);
    AICWFDBG(LOGINFO, "pwrofst2x 2.4g: [0]:11b, [1]:ofdm_highrate, [2]:ofdm_lowrate\n"
        "  chan=" "\t1-4" "\t5-9" "\t10-13");
    for (type = 0; type < 3; type++) {
        AICWFDBG(LOGINFO, "\n  [%d] =", type);
        for (ch_grp = 0; ch_grp < 3; ch_grp++) {
            AICWFDBG(LOGINFO, "\t%d", txpwr_ofst2x->pwrofst2x_tbl_2g4[type][ch_grp]);
        }
    }
    AICWFDBG(LOGINFO, "\npwrofst2x 5g: [0]:ofdm_lowrate, [1]:ofdm_highrate, [2]:ofdm_midrate\n"
        "  chan=" "\t36-50" "\t51-64" "\t98-114" "\t115-130" "\t131-146" "\t147-166");
    for (type = 0; type < 3; type++) {
        AICWFDBG(LOGINFO, "\n  [%d] =", type);
        for (ch_grp = 0; ch_grp < 6; ch_grp++) {
            AICWFDBG(LOGINFO, "\t%d", txpwr_ofst2x->pwrofst2x_tbl_5g[type][ch_grp]);
        }
    }
    AICWFDBG(LOGINFO, "\n");
}

void get_userconfig_txpwr_ofst2x_v2_in_fdrv(txpwr_ofst2x_conf_v2_t *txpwr_ofst2x_v2)
{
    int type, ch_grp;
    *txpwr_ofst2x_v2 = userconfig_info.txpwr_ofst2x_v2;
    AICWFDBG(LOGINFO, "%s:enable      :%d\r\n", __func__, txpwr_ofst2x_v2->enable);
    AICWFDBG(LOGINFO, "pwrofst2x 2.4g(ant0/ant1): [0]:11b, [1]:ofdm_highrate\n"
        "  chan=" "\t1-4" "\t5-9" "\t10-13");
    for (type = 0; type < 2; type++) {
        AICWFDBG(LOGINFO, "\n  [%d] =", type);
        for (ch_grp = 0; ch_grp < 3; ch_grp++) {
            AICWFDBG(LOGINFO, "\t%d/%d", txpwr_ofst2x_v2->pwrofst2x_tbl_2g4_ant0[ch_grp][type], txpwr_ofst2x_v2->pwrofst2x_tbl_2g4_ant1[ch_grp][type]);
        }
    }
    AICWFDBG(LOGINFO, "\npwrofst2x 5g: [0]:ofdm_highrate\n"
        "  chan=" "\t36-50" "\t51-64" "\t98-114" "\t115-130" "\t131-146" "\t147-166");
    for (type = 0; type < 1; type++) {
        AICWFDBG(LOGINFO, "\n  [%d] =", type);
        for (ch_grp = 0; ch_grp < 6; ch_grp++) {
            AICWFDBG(LOGINFO, "\t%d/%d", txpwr_ofst2x_v2->pwrofst2x_tbl_5g_ant0[ch_grp][type], txpwr_ofst2x_v2->pwrofst2x_tbl_5g_ant1[ch_grp][type]);
        }
    }
    AICWFDBG(LOGINFO, "\n");
}
void get_userconfig_txpwr_loss(txpwr_loss_conf_t *txpwr_loss)
{
    txpwr_loss->loss_enable      = userconfig_info.txpwr_loss.loss_enable;
    txpwr_loss->loss_value       = userconfig_info.txpwr_loss.loss_value;

    AICWFDBG(LOGINFO, "%s:loss_enable:%d\r\n",     __func__, txpwr_loss->loss_enable);
    AICWFDBG(LOGINFO, "%s:loss_value:%d\r\n",      __func__, txpwr_loss->loss_value);
}

void get_userconfig_xtal_cap(xtal_cap_conf_t *xtal_cap)
{
    *xtal_cap = userconfig_info.xtal_cap;

    AICWFDBG(LOGINFO, "%s:enable       :%d\r\n", __func__, xtal_cap->enable);
    AICWFDBG(LOGINFO, "%s:xtal_cap     :%d\r\n", __func__, xtal_cap->xtal_cap);
    AICWFDBG(LOGINFO, "%s:xtal_cap_fine:%d\r\n", __func__, xtal_cap->xtal_cap_fine);
}

void rwnx_plat_nvram_set_value(char *command, char *value)
{
    //TODO send command
    AICWFDBG(LOGINFO, "%s:command=%s value=%s\n", __func__, command, value);
    if (!strcmp(command, "enable")) {
        userconfig_info.txpwr_lvl.enable = rwnx_atoi(value);
        userconfig_info.txpwr_lvl_v2.enable = rwnx_atoi(value);
	userconfig_info.txpwr_lvl_v3.enable = rwnx_atoi(value);
    } else if (!strcmp(command, "dsss")) {
        userconfig_info.txpwr_lvl.dsss = rwnx_atoi(value);
    } else if (!strcmp(command, "ofdmlowrate_2g4")) {
        userconfig_info.txpwr_lvl.ofdmlowrate_2g4 = rwnx_atoi(value);
    } else if (!strcmp(command, "ofdm64qam_2g4")) {
        userconfig_info.txpwr_lvl.ofdm64qam_2g4 = rwnx_atoi(value);
    } else if (!strcmp(command, "ofdm256qam_2g4")) {
        userconfig_info.txpwr_lvl.ofdm256qam_2g4 = rwnx_atoi(value);
    } else if (!strcmp(command, "ofdm1024qam_2g4")) {
        userconfig_info.txpwr_lvl.ofdm1024qam_2g4 = rwnx_atoi(value);
    } else if (!strcmp(command, "ofdmlowrate_5g")) {
        userconfig_info.txpwr_lvl.ofdmlowrate_5g = rwnx_atoi(value);
    } else if (!strcmp(command, "ofdm64qam_5g")) {
        userconfig_info.txpwr_lvl.ofdm64qam_5g = rwnx_atoi(value);
    } else if (!strcmp(command, "ofdm256qam_5g")) {
        userconfig_info.txpwr_lvl.ofdm256qam_5g = rwnx_atoi(value);
    } else if (!strcmp(command, "ofdm1024qam_5g")) {
        userconfig_info.txpwr_lvl.ofdm1024qam_5g = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_1m_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11b_11ag_2g4[0] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[0] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_2m_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11b_11ag_2g4[1] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[1] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_5m5_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11b_11ag_2g4[2] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[2] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_11m_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11b_11ag_2g4[3] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[3] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_6m_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11b_11ag_2g4[4] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[4] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_9m_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11b_11ag_2g4[5] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[5] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_12m_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11b_11ag_2g4[6] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[6] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_18m_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11b_11ag_2g4[7] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[7] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_24m_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11b_11ag_2g4[8] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[8] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_36m_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11b_11ag_2g4[9] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[9] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_48m_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11b_11ag_2g4[10] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[10] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_54m_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11b_11ag_2g4[11] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11b_11ag_2g4[11] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs0_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11n_11ac_2g4[0] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_2g4[0] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs1_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11n_11ac_2g4[1] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_2g4[1] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs2_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11n_11ac_2g4[2] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_2g4[2] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs3_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11n_11ac_2g4[3] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_2g4[3] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs4_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11n_11ac_2g4[4] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_2g4[4] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs5_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11n_11ac_2g4[5] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_2g4[5] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs6_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11n_11ac_2g4[6] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_2g4[6] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs7_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11n_11ac_2g4[7] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_2g4[7] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs8_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11n_11ac_2g4[8] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_2g4[8] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs9_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11n_11ac_2g4[9] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_2g4[9] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs0_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11ax_2g4[0] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[0] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs1_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11ax_2g4[1] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[1] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs2_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11ax_2g4[2] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[2] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs3_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11ax_2g4[3] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[3] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs4_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11ax_2g4[4] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[4] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs5_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11ax_2g4[5] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[5] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs6_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11ax_2g4[6] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[6] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs7_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11ax_2g4[7] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[7] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs8_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11ax_2g4[8] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[8] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs9_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11ax_2g4[9] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[9] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs10_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11ax_2g4[10] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[10] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs11_2g4")) {
        userconfig_info.txpwr_lvl_v2.pwrlvl_11ax_2g4[11] = rwnx_atoi(value);
		userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_2g4[11] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_1m_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11a_5g[0] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_2m_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11a_5g[1] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_5m5_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11a_5g[2] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_11m_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11a_5g[3] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_6m_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11a_5g[4] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_9m_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11a_5g[5] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_12m_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11a_5g[6] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_18m_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11a_5g[7] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_24m_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11a_5g[8] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_36m_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11a_5g[9] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_48m_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11a_5g[10] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_54m_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11a_5g[11] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs0_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_5g[0] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs1_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_5g[1] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs2_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_5g[2] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs3_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_5g[3] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs4_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_5g[4] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs5_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_5g[5] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs6_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_5g[6] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs7_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_5g[7] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs8_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_5g[8] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs9_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11n_11ac_5g[9] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs0_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_5g[0] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs1_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_5g[1] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs2_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_5g[2] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs3_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_5g[3] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs4_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_5g[4] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs5_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_5g[5] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs6_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_5g[6] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs7_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_5g[7] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs8_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_5g[8] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs9_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_5g[9] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs10_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_5g[10] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs11_5g")) {
        userconfig_info.txpwr_lvl_v3.pwrlvl_11ax_5g[11] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_enable")) {
        userconfig_info.txpwr_lvl_adj.enable = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_2g4_chan_1_4")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_2g4[0] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_2g4_chan_5_9")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_2g4[1] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_2g4_chan_10_13")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_2g4[2] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_5g_chan_42")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_5g[0] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_5g_chan_58")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_5g[1] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_5g_chan_106")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_5g[2] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_5g_chan_122")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_5g[3] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_5g_chan_138")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_5g[4] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_5g_chan_155")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_5g[5] = rwnx_atoi(value);
    } else if (!strcmp(command, "loss_enable")) {
        userconfig_info.txpwr_loss.loss_enable = rwnx_atoi(value);
    } else if (!strcmp(command, "loss_value")) {
        userconfig_info.txpwr_loss.loss_value = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_enable")) {
        userconfig_info.txpwr_ofst.enable = rwnx_atoi(value);
        userconfig_info.txpwr_ofst2x.enable = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_chan_1_4")) {
        userconfig_info.txpwr_ofst.chan_1_4 = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_chan_5_9")) {
        userconfig_info.txpwr_ofst.chan_5_9 = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_chan_10_13")) {
        userconfig_info.txpwr_ofst.chan_10_13 = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_chan_36_64")) {
        userconfig_info.txpwr_ofst.chan_36_64 = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_chan_100_120")) {
        userconfig_info.txpwr_ofst.chan_100_120 = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_chan_122_140")) {
        userconfig_info.txpwr_ofst.chan_122_140 = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_chan_142_165")) {
        userconfig_info.txpwr_ofst.chan_142_165 = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_11b_chan_1_4")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_2g4[0][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_11b_chan_5_9")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_2g4[0][1] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_11b_chan_10_13")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_2g4[0][2] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ofdm_highrate_chan_1_4")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_2g4[1][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ofdm_highrate_chan_5_9")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_2g4[1][1] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ofdm_highrate_chan_10_13")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_2g4[1][2] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ofdm_lowrate_chan_1_4")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_2g4[2][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ofdm_lowrate_chan_5_9")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_2g4[2][1] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ofdm_lowrate_chan_10_13")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_2g4[2][2] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_lowrate_chan_42")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[0][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_lowrate_chan_58")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[0][1] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_lowrate_chan_106")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[0][2] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_lowrate_chan_122")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[0][3] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_lowrate_chan_138")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[0][4] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_lowrate_chan_155")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[0][5] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_highrate_chan_42")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[1][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_highrate_chan_58")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[1][1] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_highrate_chan_106")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[1][2] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_highrate_chan_122")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[1][3] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_highrate_chan_138")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[1][4] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_highrate_chan_155")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[1][5] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_midrate_chan_42")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[2][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_midrate_chan_58")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[2][1] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_midrate_chan_106")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[2][2] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_midrate_chan_122")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[2][3] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_midrate_chan_138")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[2][4] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ofdm_midrate_chan_155")) {
        userconfig_info.txpwr_ofst2x.pwrofst2x_tbl_5g[2][5] = rwnx_atoi(value);
    } else if (!strcmp(command, "xtal_enable")) {
        userconfig_info.xtal_cap.enable = rwnx_atoi(value);
    } else if (!strcmp(command, "xtal_cap")) {
        userconfig_info.xtal_cap.xtal_cap = rwnx_atoi(value);
    } else if (!strcmp(command, "xtal_cap_fine")) {
        userconfig_info.xtal_cap.xtal_cap_fine = rwnx_atoi(value);
    } else {
        AICWFDBG(LOGERROR, "invalid cmd: %s\n", command);
    }

}
void rwnx_plat_nvram_set_value_8800d80x2(char *command, char *value)
{
    //TODO send command
    AICWFDBG(LOGINFO, "%s:command=%s value=%s\n", __func__, command, value);
    if (!strcmp(command, "enable")) {
        userconfig_info.txpwr_lvl_v4.enable = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_1m_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[0] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_2m_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[1] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_5m5_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[2] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_11m_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[3] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_6m_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[4] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_9m_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[5] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_12m_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[6] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_18m_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[7] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_24m_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[8] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_36m_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[9] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_48m_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[10] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11b_11ag_54m_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11b_11ag_2g4[11] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs0_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_2g4[0] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs1_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_2g4[1] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs2_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_2g4[2] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs3_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_2g4[3] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs4_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_2g4[4] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs5_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_2g4[5] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs6_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_2g4[6] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs7_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_2g4[7] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs8_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_2g4[8] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs9_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_2g4[9] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs0_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[0] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs1_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[1] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs2_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[2] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs3_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[3] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs4_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[4] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs5_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[5] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs6_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[6] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs7_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[7] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs8_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[8] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs9_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[9] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs10_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[10] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs11_2g4")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_2g4[11] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_6m_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11a_5g[0] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_9m_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11a_5g[1] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_12m_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11a_5g[2] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_18m_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11a_5g[3] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_24m_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11a_5g[4] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_36m_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11a_5g[5] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_48m_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11a_5g[6] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11a_54m_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11a_5g[7] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs0_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_5g[0] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs1_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_5g[1] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs2_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_5g[2] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs3_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_5g[3] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs4_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_5g[4] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs5_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_5g[5] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs6_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_5g[6] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs7_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_5g[7] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs8_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_5g[8] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11n_11ac_mcs9_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11n_11ac_5g[9] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs0_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_5g[0] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs1_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_5g[1] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs2_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_5g[2] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs3_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_5g[3] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs4_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_5g[4] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs5_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_5g[5] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs6_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_5g[6] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs7_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_5g[7] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs8_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_5g[8] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs9_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_5g[9] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs10_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_5g[10] = rwnx_atoi(value);
    } else if (!strcmp(command,     "lvl_11ax_mcs11_5g")) {
        userconfig_info.txpwr_lvl_v4.pwrlvl_11ax_5g[11] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_enable")) {
        userconfig_info.txpwr_lvl_adj.enable = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_2g4_chan_1_4")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_2g4[0] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_2g4_chan_5_9")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_2g4[1] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_2g4_chan_10_13")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_2g4[2] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_5g_chan_42")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_5g[0] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_5g_chan_58")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_5g[1] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_5g_chan_106")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_5g[2] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_5g_chan_122")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_5g[3] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_5g_chan_138")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_5g[4] = rwnx_atoi(value);
    } else if (!strcmp(command, "lvl_adj_5g_chan_155")) {
        userconfig_info.txpwr_lvl_adj.pwrlvl_adj_tbl_5g[5] = rwnx_atoi(value);
    } else if (!strcmp(command, "loss_enable")) {
        userconfig_info.txpwr_loss.loss_enable = rwnx_atoi(value);
    } else if (!strcmp(command, "loss_value")) {
        userconfig_info.txpwr_loss.loss_value = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_enable")) {
		userconfig_info.txpwr_ofst2x_v2.enable = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ant0_11b_chan_1_4")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_2g4_ant0[0][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ant0_11b_chan_5_9")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_2g4_ant0[1][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ant0_11b_chan_10_13")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_2g4_ant0[2][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ant0_ofdm_highrate_chan_1_4")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_2g4_ant0[0][1] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ant0_ofdm_highrate_chan_5_9")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_2g4_ant0[1][1] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ant0_ofdm_highrate_chan_10_13")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_2g4_ant0[2][1] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ant1_11b_chan_1_4")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_2g4_ant1[0][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ant1_11b_chan_5_9")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_2g4_ant1[1][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ant1_11b_chan_10_13")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_2g4_ant1[2][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ant1_ofdm_highrate_chan_1_4")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_2g4_ant1[0][1] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ant1_ofdm_highrate_chan_5_9")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_2g4_ant1[1][1] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_2g4_ant1_ofdm_highrate_chan_10_13")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_2g4_ant1[2][1] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ant0_ofdm_highrate_chan_42")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_5g_ant0[0][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ant0_ofdm_highrate_chan_58")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_5g_ant0[1][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ant0_ofdm_highrate_chan_106")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_5g_ant0[2][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ant0_ofdm_highrate_chan_122")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_5g_ant0[3][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ant0_ofdm_highrate_chan_138")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_5g_ant0[4][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ant0_ofdm_highrate_chan_155")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_5g_ant0[5][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ant1_ofdm_highrate_chan_42")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_5g_ant1[0][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ant1_ofdm_highrate_chan_58")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_5g_ant1[1][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ant1_ofdm_highrate_chan_106")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_5g_ant1[2][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ant1_ofdm_highrate_chan_122")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_5g_ant1[3][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ant1_ofdm_highrate_chan_138")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_5g_ant1[4][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "ofst_5g_ant1_ofdm_highrate_chan_155")) {
        userconfig_info.txpwr_ofst2x_v2.pwrofst2x_tbl_5g_ant1[5][0] = rwnx_atoi(value);
    } else if (!strcmp(command, "xtal_enable")) {
        userconfig_info.xtal_cap.enable = rwnx_atoi(value);
    } else if (!strcmp(command, "xtal_cap")) {
        userconfig_info.xtal_cap.xtal_cap = rwnx_atoi(value);
    } else if (!strcmp(command, "xtal_cap_fine")) {
        userconfig_info.xtal_cap.xtal_cap_fine = rwnx_atoi(value);
    } else {
        AICWFDBG(LOGERROR, "invalid cmd: %s\n", command);
    }
}

void rwnx_plat_userconfig_parsing_8800d80x2(char *buffer, int size)
{
    int i = 0;
    int parse_state = 0;
    char command[64];
    char value[100];
    int char_counter = 0;

    memset(command, 0, 64);
    memset(value, 0, 100);

    for (i = 0; i < size; i++) {
        //Send command or print nvram log when char is \r or \n
        if (buffer[i] == 0x0a || buffer[i] == 0x0d) {
            if (command[0] != 0 && value[0] != 0) {
                if (parse_state == PRINT) {
                    AICWFDBG(LOGINFO, "%s:%s\r\n", __func__, value);
                } else if (parse_state == GET_VALUE) {
                    rwnx_plat_nvram_set_value_8800d80x2(command, value);
                }
            }
            //Reset command value and char_counter
            memset(command, 0, 64);
            memset(value, 0, 100);
            char_counter = 0;
            parse_state = INIT;
            continue;
        }

        //Switch parser state
        if (parse_state == INIT) {
            if (buffer[i] == '#') {
                parse_state = PRINT;
                continue;
            } else if (buffer[i] == 0x0a || buffer[i] == 0x0d) {
                parse_state = INIT;
                continue;
            } else {
                parse_state = CMD;
            }
        }

        //Fill data to command and value
        if (parse_state == PRINT) {
            command[0] = 0x01;
            value[char_counter] = buffer[i];
            char_counter++;
        } else if (parse_state == CMD) {
            if (command[0] != 0 && buffer[i] == '=') {
                parse_state = GET_VALUE;
                char_counter = 0;
                continue;
            }
            command[char_counter] = buffer[i];
            char_counter++;
        } else if (parse_state == GET_VALUE) {
            if(buffer[i] != 0x2D && (buffer[i] < 0x30 || buffer[i] > 0x39)) {
                continue;
            }
            value[char_counter] = buffer[i];
            char_counter++;
        }
    }
}

void rwnx_plat_userconfig_parsing(char *buffer, int size)
{
    int i = 0;
    int parse_state = 0;
    char command[64];
    char value[100];
    int char_counter = 0;

    memset(command, 0, 64);
    memset(value, 0, 100);

    for (i = 0; i < size; i++) {
        //Send command or print nvram log when char is \r or \n
        if (buffer[i] == 0x0a || buffer[i] == 0x0d) {
            if (command[0] != 0 && value[0] != 0) {
                if (parse_state == PRINT) {
                    AICWFDBG(LOGINFO, "%s:%s\r\n", __func__, value);
                } else if (parse_state == GET_VALUE) {
                    rwnx_plat_nvram_set_value(command, value);
                }
            }
            //Reset command value and char_counter
            memset(command, 0, 64);
            memset(value, 0, 100);
            char_counter = 0;
            parse_state = INIT;
            continue;
        }

        //Switch parser state
        if (parse_state == INIT) {
            if (buffer[i] == '#') {
                parse_state = PRINT;
                continue;
            } else if (buffer[i] == 0x0a || buffer[i] == 0x0d) {
                parse_state = INIT;
                continue;
            } else {
                parse_state = CMD;
            }
        }

        //Fill data to command and value
        if (parse_state == PRINT) {
            command[0] = 0x01;
            value[char_counter] = buffer[i];
            char_counter++;
        } else if (parse_state == CMD) {
            if (command[0] != 0 && buffer[i] == '=') {
                parse_state = GET_VALUE;
                char_counter = 0;
                continue;
            }
            command[char_counter] = buffer[i];
            char_counter++;
        } else if (parse_state == GET_VALUE) {
            if(buffer[i] != 0x2D && (buffer[i] < 0x30 || buffer[i] > 0x39)) {
		continue;
            }
            value[char_counter] = buffer[i];
            char_counter++;
        }
    }
}

#ifdef CONFIG_POWER_LIMIT
#define GetLineFromBuffer(buffer)   strsep(&buffer, "\n")

int isAllSpaceOrTab(uint8_t *data, uint8_t size)
{
    uint8_t cnt = 0, NumOfSpaceAndTab = 0;
    while (size > cnt) {
        if (data[cnt] == ' ' || data[cnt] == '\t' || data[cnt] == '\0')
            ++NumOfSpaceAndTab;
        ++cnt;
    }
    return size == NumOfSpaceAndTab;
}

int IsCommentString(char *szStr)
{
    if (*szStr == '#' && *(szStr + 1) == ' ')
        return 1;
    else
        return 0;
}

int ParseQualifiedString(char *In, u32 *Start, char *Out, char LeftQualifier, char RightQualifier)
{
    u32 i = 0, j = 0;
    char c = In[(*Start)++];
    if (c != LeftQualifier)
        return 0;
    i = (*Start);
    c = In[(*Start)++];
    while (c != RightQualifier && c != '\0')
        c = In[(*Start)++];
    if (c == '\0')
        return 0;
    j = (*Start) - 2;
    strncpy((char *)Out, (const char *)(In + i), j - i + 1);
    return 1;
}

int GetU1ByteIntegerFromStringInDecimal(char *Str, u8 *pInt)
{
    u16 i = 0;
    *pInt = 0;
    while (Str[i] != '\0') {
        if (Str[i] >= '0' && Str[i] <= '9') {
            *pInt *= 10;
            *pInt += (Str[i] - '0');
        } else
            return 0;
        ++i;
    }
    return 1;
}
int GetS1ByteIntegerFromStringInDecimal(char *str, s8 *val)
{
    u8 negative = 0;
    u16 i = 0;
    *val = 0;
    while (str[i] != '\0') {
        if (i == 0 && (str[i] == '+' || str[i] == '-')) {
            if (str[i] == '-')
                negative = 1;
        } else if (str[i] >= '0' && str[i] <= '9') {
            *val *= 10;
            *val += (str[i] - '0');
        } else
            return 0;
        ++i;
    }
    if (negative)
        *val = -*val;
    return 1;
}

void rwnx_plat_powerlimit_parsing(char *buffer, int size, char *cc)
{
#define LD_STAGE_EXC_MAPPING    0
#define LD_STAGE_TAB_DEFINE     1
#define LD_STAGE_TAB_START      2
#define LD_STAGE_COLUMN_DEFINE  3
#define LD_STAGE_CH_ROW         4

    uint8_t loadingStage = LD_STAGE_EXC_MAPPING;
    uint32_t i = 0, forCnt = 0;
    uint32_t i_cc;
    char *szLine, *ptmp;
    char band[10], colNumBuf[10];
    uint8_t colNum = 0, colNum_cc = 255, band_cc = 0;
    // clear powerlimit info at first
    memset((void *)&powerlimit_info, 0, sizeof(powerlimit_info_t));
    ptmp = buffer;
    for (szLine = GetLineFromBuffer(ptmp); szLine != NULL; szLine = GetLineFromBuffer(ptmp)) {
        if (isAllSpaceOrTab(szLine, sizeof(*szLine)))
            continue;
        if (IsCommentString(szLine))
            continue;

        if (loadingStage == LD_STAGE_EXC_MAPPING) {
            if (szLine[0] == '#' || szLine[1] == '#') {
                loadingStage = LD_STAGE_TAB_DEFINE;
            } else {
                continue;
            }
        }

        if (loadingStage == LD_STAGE_TAB_DEFINE) {
            /* read "##	2.4G" */
            if (szLine[0] != '#' || szLine[1] != '#')
                continue;

            /* skip the space */
            i = 2;
            while (szLine[i] == ' ' || szLine[i] == '\t')
                ++i;

            szLine[--i] = ' '; /* return the space in front of the regulation info */

            /* Parse the label of the table */
            memset((void *)band, 0, 10);
            if (!ParseQualifiedString(szLine, &i, band, ' ', ',')) {
                AICWFDBG(LOGERROR, "Fail to parse band!\n");
                goto exit;
            }
            if (strncmp(band, "2.4G", 4) == 0) {
                band_cc = PHY_BAND_2G4;
            } else if (strncmp(band, "5G", 2) == 0) {
                band_cc = PHY_BAND_5G;
            }
            memset((void *) colNumBuf, 0, 10);
            if (!ParseQualifiedString(szLine, &i, colNumBuf, '#', '#')) {
                AICWFDBG(LOGERROR, "Fail to parse column number!\n");
                goto exit;
            }
            if (!GetU1ByteIntegerFromStringInDecimal(colNumBuf, &colNum)) {
                AICWFDBG(LOGERROR, "Column number \"%s\" is not unsigned decimal\n", colNumBuf);
                goto exit;
            }
            if (colNum == 0) {
                AICWFDBG(LOGERROR, "Column number is 0\n");
                goto exit;
            }

            AICWFDBG(LOGINFO, "band=%s(%d)\n", band, band_cc);
            loadingStage = LD_STAGE_TAB_START;
        } else if (loadingStage == LD_STAGE_TAB_START) {
            /* read "##	START" */
            if (szLine[0] != '#' || szLine[1] != '#')
                continue;

            /* skip the space */
            i = 2;
            while (szLine[i] == ' ' || szLine[i] == '\t')
                ++i;

            if (strncmp((u8 *)(szLine + i), "START", 5)) {
                AICWFDBG(LOGERROR, "Missing \"##   START\" label\n");
                goto exit;
            }

            loadingStage = LD_STAGE_COLUMN_DEFINE;
        } else if (loadingStage == LD_STAGE_COLUMN_DEFINE) {
            /* read "##	CN	US" */
            if (szLine[0] != '#' || szLine[1] != '#')
                continue;

            /* skip the space */
            i = 2;
            while (szLine[i] == ' ' || szLine[i] == '\t')
                ++i;

            for (forCnt = 0; forCnt < colNum; forCnt++) {
                /* skip the space */
                while (szLine[i] == ' ' || szLine[i] == '\t')
                    i++;
                i_cc = i;

                while (szLine[i] != ' ' && szLine[i] != '\t' && szLine[i] != '\0')
                    i++;

                if ((i - i_cc) != 2) {
                    AICWFDBG(LOGERROR, "CC len err\n");
                    goto exit;
                } else if ((szLine[i_cc] == cc[0]) && (szLine[i_cc + 1] == cc[1])) {
                    AICWFDBG(LOGINFO, "CC matched: %s, col=%d\n", cc, forCnt);
                    colNum_cc = forCnt;
                    powerlimit_info.flags |= POWER_LIMIT_CC_MATCHED_BIT;
                    break;
                }
            }

            loadingStage = LD_STAGE_CH_ROW;
        } else if (loadingStage == LD_STAGE_CH_ROW) {
            char channel[10] = {0}, powerLimit[10] = {0};
            u8 channel_num, powerLimit_val, cnt = 0;

            /* the table ends */
            if (szLine[0] == '#' && szLine[1] == '#') {
                i = 2;
                while (szLine[i] == ' ' || szLine[i] == '\t')
                    ++i;

                if (strncmp((u8 *)(szLine + i), "END", 3) == 0) {
                    loadingStage = LD_STAGE_TAB_DEFINE;
                    colNum = 0;
                    continue;
                } else {
                    AICWFDBG(LOGERROR, "Missing \"##   END\" label\n");
                    goto exit;
                }
            }

            if ((szLine[0] != 'c' && szLine[0] != 'C') ||
                (szLine[1] != 'h' && szLine[1] != 'H')
            ) {
                AICWFDBG(LOGERROR, "Wrong channel prefix: '%c','%c'(%d,%d)\n", szLine[0], szLine[1], szLine[0], szLine[1]);
                continue;
            }
            i = 2;/* move to the  location behind 'h' */

            /* load the channel number */
            cnt = 0;
            while (szLine[i] >= '0' && szLine[i] <= '9') {
                channel[cnt] = szLine[i];
                ++cnt;
                ++i;
            }

            for (forCnt = 0; forCnt < colNum; ++forCnt) {
                /* skip the space between channel number and the power limit value */
                while (szLine[i] == ' ' || szLine[i] == '\t')
                    ++i;

                /* load the power limit value */
                memset((void *)powerLimit, 0, 10);

                if (szLine[i] == 'N' && szLine[i + 1] == 'A') {
                    /*
                    * means channel not available
                    */
                    sprintf(powerLimit, "%d", POWER_LIMIT_INVALID_VAL);
                    i += 2;
                } else if ((szLine[i] >= '0' && szLine[i] <= '9')
                    || szLine[i] == '+' || szLine[i] == '-'
                ) {
                    /* case of dBm value */
                    cnt = 0;
                    while ((szLine[i] >= '0' && szLine[i] <= '9')
                        || szLine[i] == '+' || szLine[i] == '-'
                    ) {
                        powerLimit[cnt] = szLine[i];
                        ++cnt;
                        ++i;
                    }
                } else {
                    AICWFDBG(LOGERROR, "Wrong limit expression \"%c%c\"(%d, %d)\n"
                        , szLine[i], szLine[i + 1], szLine[i], szLine[i + 1]);
                    goto exit;
                }

                if (forCnt == colNum_cc) {
                    /* store the power limit value */
                    if (GetU1ByteIntegerFromStringInDecimal((char *)channel, &channel_num) == 0
                        || GetS1ByteIntegerFromStringInDecimal((char *)powerLimit, &powerLimit_val) == 0
                    ) {
                        AICWFDBG(LOGERROR, "Illegal index of power limit table [ch %s][val %s]\n", channel, powerLimit);
                        goto exit;
                    }

                    if (band_cc == PHY_BAND_2G4) {
                        uint8_t cur_idx = powerlimit_info.txpwr_lmt.ch_cnt_2g4;
                        AICWFDBG(LOGINFO, "[%d]: ch=%s, pwr=%s\n", cur_idx, channel, powerLimit);
                        if (cur_idx < MAC_DOMAINCHANNEL_24G_MAX) {
                            powerlimit_info.txpwr_lmt.ch_num_2g4[cur_idx] = channel_num;
                            powerlimit_info.txpwr_lmt.max_pwr_2g4[cur_idx] = powerLimit_val;
                            powerlimit_info.txpwr_lmt.ch_cnt_2g4++;
                        } else {
                            AICWFDBG(LOGERROR, "band %d chan_cnt reached %d\n", band_cc, MAC_DOMAINCHANNEL_24G_MAX);
                            AICWFDBG(LOGERROR, "channel=%s(%d) powerLimit=%s(%d)\n", channel, channel_num, powerLimit, powerLimit_val);
                        }
                    } else if (band_cc == PHY_BAND_5G) {
                        uint8_t cur_idx = powerlimit_info.txpwr_lmt.ch_cnt_5g;
                        AICWFDBG(LOGINFO, "[%d]: ch=%s, pwr=%s\n", cur_idx, channel, powerLimit);
                        if (cur_idx < MAC_DOMAINCHANNEL_5G_MAX) {
                            powerlimit_info.txpwr_lmt.ch_num_5g[cur_idx] = channel_num;
                            powerlimit_info.txpwr_lmt.max_pwr_5g[cur_idx] = powerLimit_val;
                            powerlimit_info.txpwr_lmt.ch_cnt_5g++;
                        } else {
                            AICWFDBG(LOGERROR, "band %d chan_cnt reached %d\n", band_cc, MAC_DOMAINCHANNEL_5G_MAX);
                            AICWFDBG(LOGERROR, "channel=%s(%d) powerLimit=%s(%d)\n", channel, channel_num, powerLimit, powerLimit_val);
                        }
                    }
                    break;
                }
            }
        }
    }
exit:
    return;
}

/// 5G lower bound freq
#define PHY_FREQ_5G 5000

uint16_t phy_channel_to_freq(uint8_t band, int channel)
{
    if ((band == PHY_BAND_2G4) && (channel >= 1) && (channel <= 14)) {
        if (channel == 14)
            return 2484;
        else
            return 2407 + channel * 5;
    } else if ((band == PHY_BAND_5G) && (channel >= 1) && (channel <= 165)) {
        return PHY_FREQ_5G + channel * 5;
    }
    return 0;
}

int8_t get_powerlimit_by_freq(uint8_t band, uint16_t freq)
{
    int8_t ret = POWER_LIMIT_INVALID_VAL;
    uint8_t idx;
    if (powerlimit_info.flags & POWER_LIMIT_CC_MATCHED_BIT) {
        if (band == PHY_BAND_2G4) {
            uint8_t idx_cnt = powerlimit_info.txpwr_lmt.ch_cnt_2g4;
            for (idx = 0; idx < idx_cnt; idx++) {
                int ch_num = powerlimit_info.txpwr_lmt.ch_num_2g4[idx];
                uint16_t freq_tmp = phy_channel_to_freq(PHY_BAND_2G4, ch_num);
                if (freq == freq_tmp) {
                    ret = powerlimit_info.txpwr_lmt.max_pwr_2g4[idx];
                    //AICWFDBG(LOGINFO, "[%d]: ch=%d(freq=%d), pwr=%d\n", idx, ch_num, freq, ret);
                    break;
                }
            }
            if (idx == idx_cnt) {
                AICWFDBG(LOGERROR, "powerlimit search failed: band=%d freq=%d\n", band, freq);
            }
        } else if (band == PHY_BAND_5G) {
            uint8_t idx_cnt = powerlimit_info.txpwr_lmt.ch_cnt_5g;
            for (idx = 0; idx < idx_cnt; idx++) {
                int ch_num = powerlimit_info.txpwr_lmt.ch_num_5g[idx];
                uint16_t freq_tmp = phy_channel_to_freq(PHY_BAND_5G, ch_num);
                if (freq == freq_tmp) {
                    ret = powerlimit_info.txpwr_lmt.max_pwr_5g[idx];
                    //AICWFDBG(LOGINFO, "[%d]: ch=%d(freq=%d), pwr=%d\n", idx, ch_num, freq, ret);
                    break;
                }
            }
            if (idx == idx_cnt) {
                AICWFDBG(LOGERROR, "powerlimit search failed: band=%d freq=%d\n", band, freq);
            }
        }
    }
    return ret;
}

int8_t get_powerlimit_by_chnum(uint8_t chnum)
{
    int8_t ret = POWER_LIMIT_INVALID_VAL;
    uint8_t idx;
    if (powerlimit_info.flags & POWER_LIMIT_CC_MATCHED_BIT) {
        if (chnum <= 14) {
            uint8_t idx_cnt = powerlimit_info.txpwr_lmt.ch_cnt_2g4;
            for (idx = 0; idx < idx_cnt; idx++) {
                uint8_t ch_num = powerlimit_info.txpwr_lmt.ch_num_2g4[idx];
                if (chnum == ch_num) {
                    ret = powerlimit_info.txpwr_lmt.max_pwr_2g4[idx];
                    //AICWFDBG(LOGINFO, "[%d]: ch=%d, pwr=%d\n", idx, ch_num, ret);
                    break;
                }
            }
            if (idx == idx_cnt) {
                AICWFDBG(LOGERROR, "powerlimit search failed: chnum=%d\n", chnum);
            }
        } else if (chnum <= 165) {
            uint8_t idx_cnt = powerlimit_info.txpwr_lmt.ch_cnt_5g;
            for (idx = 0; idx < idx_cnt; idx++) {
                int ch_num = powerlimit_info.txpwr_lmt.ch_num_5g[idx];
                if (chnum == ch_num) {
                    ret = powerlimit_info.txpwr_lmt.max_pwr_5g[idx];
                    //AICWFDBG(LOGINFO, "[%d]: ch=%d, pwr=%d\n", idx, ch_num, ret);
                    break;
                }
            }
            if (idx == idx_cnt) {
                AICWFDBG(LOGERROR, "powerlimit search failed: chnum=%d\n", chnum);
            }
        }
    }
    return ret;
}
#endif

/**
 * rwnx_plat_userconfig_load  ---Load aic_userconfig.txt
 *@filename name of config
*/
static int rwnx_plat_userconfig_load(struct rwnx_hw *rwnx_hw) {

	if(rwnx_hw->usbdev->chipid == PRODUCT_ID_AIC8800DC){
		rwnx_plat_userconfig_load_8800dc(rwnx_hw);
        #ifdef CONFIG_POWER_LIMIT
        rwnx_plat_powerlimit_load_8800dcdw(rwnx_hw, PRODUCT_ID_AIC8800DC);
        #endif
	}else if(rwnx_hw->usbdev->chipid == PRODUCT_ID_AIC8800DW){
        rwnx_plat_userconfig_load_8800dw(rwnx_hw);
        #ifdef CONFIG_POWER_LIMIT
        rwnx_plat_powerlimit_load_8800dcdw(rwnx_hw, PRODUCT_ID_AIC8800DW);
        #endif
    }else if(rwnx_hw->usbdev->chipid == PRODUCT_ID_AIC8800D81){
        rwnx_plat_userconfig_load_8800d80(rwnx_hw);
    }else if(rwnx_hw->usbdev->chipid == PRODUCT_ID_AIC8800D81X2 ||
        rwnx_hw->usbdev->chipid == PRODUCT_ID_AIC8800D89X2){
        rwnx_plat_userconfig_load_8800d80x2(rwnx_hw);
    }

	return 0;
}


/**
 * rwnx_platform_on() - Start the platform
 *
 * @rwnx_hw: Main driver data
 * @config: Config to restore (NULL if nothing to restore)
 *
 * It starts the platform :
 * - load fw and ucodes
 * - initialize IPC
 * - boot the fw
 * - enable link communication/IRQ
 *
 * Called by 802.11 part
 */
int rwnx_platform_on(struct rwnx_hw *rwnx_hw, void *config)
{
    #if 0
    u8 *shared_ram;
    #endif
#ifdef CONFIG_ROM_PATCH_EN
    int ret = 0;
#endif

    struct rwnx_plat *rwnx_plat = rwnx_hw->plat;

    RWNX_DBG(RWNX_FN_ENTRY_STR);

    if (rwnx_plat->enabled)
        return 0;

    #if 0
    if (rwnx_platform_reset(rwnx_plat))
        return -1;

    rwnx_plat_mpif_sel(rwnx_plat);

    if ((ret = rwnx_plat_fcu_load(rwnx_hw)))
        return ret;
    if ((ret = rwnx_plat_agc_load(rwnx_plat)))
        return ret;
    if ((ret = rwnx_ldpc_load(rwnx_hw)))
        return ret;
    if ((ret = rwnx_plat_lmac_load(rwnx_plat)))
        return ret;

    shared_ram = RWNX_ADDR(rwnx_plat, RWNX_ADDR_SYSTEM, SHARED_RAM_START_ADDR);
    if ((ret = rwnx_ipc_init(rwnx_hw, shared_ram)))
        return ret;

    if ((ret = rwnx_plat->enable(rwnx_hw)))
        return ret;
    RWNX_REG_WRITE(BOOTROM_ENABLE, rwnx_plat,
                   RWNX_ADDR_SYSTEM, SYSCTRL_MISC_CNTL_ADDR);

	#if 0
    if ((ret = rwnx_fw_trace_config_filters(rwnx_get_shared_trace_buf(rwnx_hw),
                                            rwnx_ipc_fw_trace_desc_get(rwnx_hw),
                                            rwnx_hw->mod_params->ftl)))
	#endif

    #ifndef CONFIG_RWNX_FHOST
    if ((ret = rwnx_check_fw_compatibility(rwnx_hw)))
    {
        rwnx_hw->plat->disable(rwnx_hw);
        tasklet_kill(&rwnx_hw->task);
        rwnx_ipc_deinit(rwnx_hw);
        return ret;
    }
    #endif /* !CONFIG_RWNX_FHOST */

    if (config)
        rwnx_term_restore_config(rwnx_plat, config);

    rwnx_ipc_start(rwnx_hw);
    #else
    #ifndef CONFIG_ROM_PATCH_EN
    #ifdef CONFIG_DOWNLOAD_FW
    if ((ret = rwnx_plat_fmac_load(rwnx_hw)))
        return ret;
    #endif /* !CONFIG_ROM_PATCH_EN */
    #endif
    #endif

#ifdef CONFIG_ROM_PATCH_EN
    ret = rwnx_plat_patch_load(rwnx_hw);
    if (ret) {
        return ret;
    }
#endif

    rwnx_plat_userconfig_load(rwnx_hw);


    //rwnx_plat->enabled = true;

    return 0;
}

/**
 * rwnx_platform_off() - Stop the platform
 *
 * @rwnx_hw: Main driver data
 * @config: Updated with pointer to config, to be able to restore it with
 * rwnx_platform_on(). It's up to the caller to free the config. Set to NULL
 * if configuration is not needed.
 *
 * Called by 802.11 part
 */
void rwnx_platform_off(struct rwnx_hw *rwnx_hw, void **config)
{
#if defined(AICWF_USB_SUPPORT) || defined(AICWF_SDIO_SUPPORT)
		tasklet_kill(&rwnx_hw->task);
        rwnx_hw->plat->enabled = false;
        return ;
#endif

    if (!rwnx_hw->plat->enabled) {
        if (config)
            *config = NULL;
        return;
    }

#ifdef AICWF_PCIE_SUPPORT
    rwnx_ipc_stop(rwnx_hw);
#endif

    if (config)
        *config = rwnx_term_save_config(rwnx_hw->plat);

    rwnx_hw->plat->disable(rwnx_hw);

    tasklet_kill(&rwnx_hw->task);

#ifdef AICWF_PCIE_SUPPORT
    rwnx_ipc_deinit(rwnx_hw);
#endif


    rwnx_platform_reset(rwnx_hw->plat);

    rwnx_hw->plat->enabled = false;
}

/**
 * rwnx_platform_init() - Initialize the platform
 *
 * @rwnx_plat: platform data (already updated by platform driver)
 * @platform_data: Pointer to store the main driver data pointer (aka rwnx_hw)
 *                That will be set as driver data for the platform driver
 * Return: 0 on success, < 0 otherwise
 *
 * Called by the platform driver after it has been probed
 */
int rwnx_platform_init(struct rwnx_plat *rwnx_plat, void **platform_data)
{
    int ret = 0;
    RWNX_DBG(RWNX_FN_ENTRY_STR);

    rwnx_plat->enabled = false;
    rwnx_plat->wait_disconnect_cb = false;
    g_rwnx_plat = rwnx_plat;

#if defined CONFIG_RWNX_FULLMAC
    AICWFDBG(LOGINFO, "%s rwnx_cfg80211_init enter \r\n", __func__);
    ret = rwnx_cfg80211_init(rwnx_plat, platform_data);
    AICWFDBG(LOGINFO, "%s rwnx_cfg80211_init exit \r\n", __func__);
#if defined(AICWF_USB_SUPPORT) && defined(CONFIG_VENDOR_GPIO)
    // initialize gpiob2, gpiob3, gpiob5 to output mode and set output to 0
    rwnx_send_dbg_gpio_init_req(rwnx_plat->usbdev->rwnx_hw, 2, 1, 0);//gpiob 2 = 0
    rwnx_send_dbg_gpio_init_req(rwnx_plat->usbdev->rwnx_hw, 3, 1, 0);//gpiob 3 = 0
    rwnx_send_dbg_gpio_init_req(rwnx_plat->usbdev->rwnx_hw, 5, 1, 0);//gpiob 5 = 0

    // read gpiob2
    //struct dbg_gpio_read_cfm gpio_rd_cfm;
    //rwnx_send_dbg_gpio_read_req(rwnx_plat->usbdev->rwnx_hw, 2, &gpio_rd_cfm);
    //AICWFDBG(LOGINFO, "gpio_rd_cfm idx:%d val:%d\n", gpio_rd_cfm.gpio_idx, gpio_rd_cfm.gpio_val);

    // set gpiob2 output to 1
    //rwnx_send_dbg_gpio_write_req(rwnx_plat->usbdev->rwnx_hw, 2, 1);
#endif

    return ret;
#elif defined CONFIG_RWNX_FHOST
    return rwnx_fhost_init(rwnx_plat, platform_data);
#endif
}

/**
 * rwnx_platform_deinit() - Deinitialize the platform
 *
 * @rwnx_hw: main driver data
 *
 * Called by the platform driver after it is removed
 */
void rwnx_platform_deinit(struct rwnx_hw *rwnx_hw)
{
    RWNX_DBG(RWNX_FN_ENTRY_STR);

#if defined CONFIG_RWNX_FULLMAC
    rwnx_cfg80211_deinit(rwnx_hw);
#elif defined CONFIG_RWNX_FHOST
    rwnx_fhost_deinit(rwnx_hw);
#endif
}

/**
 * rwnx_platform_register_drv() - Register all possible platform drivers
 */
int rwnx_platform_register_drv(void)
{
    return rwnx_pci_register_drv();
}


/**
 * rwnx_platform_unregister_drv() - Unegister all platform drivers
 */
void rwnx_platform_unregister_drv(void)
{
    return rwnx_pci_unregister_drv();
}

struct device *rwnx_platform_get_dev(struct rwnx_plat *rwnx_plat)
{
#ifdef AICWF_SDIO_SUPPORT
	return rwnx_plat->sdiodev->dev;
#endif
#ifdef AICWF_USB_SUPPORT
    return rwnx_plat->usbdev->dev;
#endif
    return &(rwnx_plat->pci_dev->dev);
}


#ifndef CONFIG_RWNX_SDM
MODULE_FIRMWARE(RWNX_AGC_FW_NAME);
MODULE_FIRMWARE(RWNX_FCU_FW_NAME);
MODULE_FIRMWARE(RWNX_LDPC_RAM_NAME);
#endif
MODULE_FIRMWARE(RWNX_MAC_FW_NAME);
#ifndef CONFIG_RWNX_TL4
MODULE_FIRMWARE(RWNX_MAC_FW_NAME2);
#endif



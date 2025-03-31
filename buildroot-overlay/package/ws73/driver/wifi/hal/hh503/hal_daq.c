/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: hal_daq.c
 * Create: 2023-01-16
 */
#ifdef _PRE_WLAN_FEATURE_DAQ
#include "hal_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID DIAG_FILE_ID_WIFI_HOST_HAL_DAQ_C

#undef THIS_MOD_ID
#define THIS_MOD_ID DIAG_MOD_ID_WIFI_HOST


#define HAL_MAC_DIAG_MATCH_ADDR_MASK  (0xFF)       // 匹配地址0xFF [7-0]
#define HAL_MAC_DIAG_MATCH_FRM_MASK   (0x3F)       // 匹配帧类型0x3F [5-0]

static osal_void hal_set_mac_diag_soc_init(const mac_cfg_mct_set_diag_stru *event_set_diag,
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device)
{
    u_cfg_monitor_clock diag_clock;
    u_cfg_high_clk_mode clk_mode;
    u_cpu_mem_share_cfg share_cken;
    u_bank_ram3_use_cfg ram3_use;
    u_cfg_sample_sel sample_sel;
    u_cfg_sample_node_sel sample_node_sel;
    u_cfg_sample_mode sample_mode;

    // 使用ram3的作为数采空间 ws73V100 DIAG_CTL维测功能使用手册.docx 信息表 DIAG_RAM 地址 当前给定32K
    osal_u32 len = (event_set_diag->value <= HAL_MAC_DIAG_SEL_PKT_RAM_16KB) ?
        ((1 + event_set_diag->value) * 512) : 0x2000; // ((2kB+2*i)*1024)/4 = (1+i)*512 word 4B  0x2000--32kb
    osal_u32 sample_start_addr = 0x680000; // 0x680000 diag_ram起始地址
    osal_u32 sample_end_addr = (event_set_diag->value <= HAL_MAC_DIAG_SEL_PKT_RAM_16KB) ?
        (sample_start_addr + 4 * len - 1) : 0x687FFF;

    /* step1  节点频率超过240Mhz，配置为1，否则配置为0 */
    clk_mode.u32 = hal_reg_read(DIAG_CTL_RB_CFG_HIGH_CLK_MODE_REG);
    clk_mode.bits.cfg_diag_high_mode = 0;
    hal_reg_write(DIAG_CTL_RB_CFG_HIGH_CLK_MODE_REG, clk_mode.u32);

    /* step2  Cpu_mem_share_cken复用ram0-ram5前关闭需要切换对应RAM的时钟使能 */
    share_cken.u32 = hal_reg_read(CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG);
    share_cken.bits.cpu_mem_share_memcken &= 0x37; // ram3 clk en
    hal_reg_write(CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG, share_cken.u32);

    hal_reg_write(CLDO_CTL_REG_BANK_RAM3_START_ADDR_REG, sample_start_addr); // 参考phy数采先设置起始地址

    /* step3  复用ram0-ram5配置 */
    ram3_use.u32 = hal_reg_read(CLDO_CTL_REG_BANK_RAM3_USE_CFG_REG);
    ram3_use.bits.bank_ram3_use_cfg = 0x2; // 2’b01---PKT(MAC) 2’b10---DIAG
    hal_reg_write(CLDO_CTL_REG_BANK_RAM3_USE_CFG_REG, ram3_use.u32);

    /* step4  Cpu_mem_share_cken复用ram0-ram5后打开对应RAM的时钟使能 */
    share_cken.u32 = hal_reg_read(CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG);
    share_cken.bits.cpu_mem_share_memcken |= 0x8; // ram3 clk en
    hal_reg_write(CLDO_CTL_REG_CPU_MEM_SHARE_CFG_REG, share_cken.u32);

    /* step14 配置数据采集模式同步使能寄存器 在配置前12步骤时保持前置0，配置完成后置1 */
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_GEN_SYNC_REG, 0x0);

    /* step0  配置数采时钟门控，打开clk_monitor时钟 */
    diag_clock.u32 = hal_reg_read(DIAG_CTL_RB_CFG_MONITOR_CLOCK_REG);
    diag_clock.bits.cfg_monitor_clken = 1;
    diag_clock.bits.cfg_monitor_rd_src_clken = 1;
    diag_clock.bits.cfg_monitor_src_clken = 1;
    hal_reg_write(DIAG_CTL_RB_CFG_MONITOR_CLOCK_REG, diag_clock.u32);

    /* step5  步骤1把CFG_HIGH_CLK_MODE配置为1的时候才需要配置 */
    /* step6  如果选择采集CPU数据：配置数据采集模式选择寄存器 */

    /* step7  如果数采检测APB总线：使能检测CPU访问WLMAC的APB接口4'd7 : WIFI-APB(配合CFG_APB_DIAG_EN寄存器) */

    /* step8 配置数据采集源选择4'd5 : WLMAC */
    sample_sel.u32 = hal_reg_read(DIAG_CTL_RB_CFG_SAMPLE_SEL_REG);
    sample_sel.bits.cfg_sample_sel = hal_mac_test_device->diag_param.diag_source; // 0x5: WLMAC; 0xB: COEX
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_SEL_REG, sample_sel.u32);

    /* step9  配置数据采集源选择源头内部节点选择 */
    sample_node_sel.u32 = hal_reg_read(DIAG_CTL_RB_CFG_SAMPLE_NODE_SEL_REG); // sample_node_sel.bits.cfg_sample_sel

    /* step10 11 12  配置采集长度和地址是最大地址还是实际地址?? */
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_LENGTH_REG, len); // MAC采集长度 单位为WORD 4B 4*0x1000=        16KB的RAM
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_START_ADDR_REG, sample_start_addr);
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_END_ADDR_REG, sample_end_addr);

    /* step13 配置数据采集模式:定长模式'2'b00或循环模式2'b01 */
    sample_mode.u32 = 0;
    sample_mode.bits.cfg_sample_mode = event_set_diag->mode;
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_MODE_REG, sample_mode.u32);

    /* step14 配置数据采集模式同步使能寄存器 在配置前12步骤时保持前置0，配置完成后置1 */
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_GEN_SYNC_REG, 0x1);

    /* step15 循环采数配置：采数计数N个数据后中断上报CPU */

    /* step16 配置维测中断上报使能 */

    /* step17 配置维测BUS监测地址 */

    /* step18 配置维测BUS监测地址数据掩位 */

    /* step19 配置维测BUS监测地址数据 */

    /* step20 配置维测BUS监测使能。1’b1:使能地址监测 */

    /* step21 配置数据采集使能寄存器 电平拉高后开始采集数据 */
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_EN_REG, 0x1);
}

static osal_void hal_set_mac_diag_soc_clear(hal_to_dmac_device_mac_test_stru *hal_mac_test_device)
{
    hal_reg_write(DIAG_CTL_RB_CFG_MONITOR_CLOCK_REG, 0x0);        // 数采时钟门控 CLDO+330
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_GEN_SYNC_REG, 0x0);      // 数采模式同步使能 diag+228
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_EN_REG, 0x0);            // 数采使能 高电平开始采集  diag+22C
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_MODE_REG, 0x0);          // 数采模式配置 定长或循环  diag+224
    hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_NODE_SEL_REG, 0x0);      // 数采源节点选择   diag+214
    hal_mac_test_device->diag_param.diag_mode = 0;
    hal_mac_test_device->diag_param.diag_basic_start_addr = 0;
    hal_mac_test_device->diag_param.diag_basic_end_addr = 0;
    hal_mac_test_device->diag_param.diag_read_addr = 0;
    hal_mac_test_device->diag_param.diag_num = 0;
    hal_mac_test_device->diag_param.diag_source = 0;
}

osal_void hal_set_mac_diag_init(osal_u8 vap_id, const mac_cfg_mct_set_diag_stru *event_set_diag,
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device)
{
    u_diag_control1 diag_ctrl1;
    u_diag_en diag_en;

    hal_set_mac_diag_soc_init(event_set_diag, hal_mac_test_device);

    hal_mac_test_device->diag_param.diag_basic_start_addr = hal_reg_read(DIAG_CTL_RB_CFG_SAMPLE_START_ADDR_REG);
    hal_mac_test_device->diag_param.diag_basic_end_addr =  hal_reg_read(DIAG_CTL_RB_CFG_SAMPLE_END_ADDR_REG);

    diag_ctrl1.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x18C);
    diag_en.u32 = hal_reg_read(HH503_MAC_VLD_BANK_BASE_0x78);

    diag_ctrl1.bits.xCt2O4lz2ORstOSolROsmx_ = 1; // 选择输出的TSF单位 0:us 1:4us 2:16us 3:32us
    diag_ctrl1.bits.xCt2O4lz2ORstOsmx_ = vap_id; // 0：vap0 1：vap1 2：vap2 3：vap3

    diag_en.bits.xCt2O4lz2rOmo_ = 0;
    diag_en.bits.xCt2O4lz2KOmo_ = 0;
    diag_en.bits.xCt2O4lz2nOmo_ = 0;
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x78, diag_en.u32);

    if (event_set_diag->mode == OSAL_FALSE) {
        diag_ctrl1.bits.x6NgwhiwidLudLwWh_ = 0; // mac 循环采数模式使能
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, diag_ctrl1.u32);
        /* 0:2kB 1:4kB 2:6kB 3:8kB 4:10kB 5:12kB 6:14kB 7:16kB */
        if (event_set_diag->value >= HAL_MAC_DIAG_SEL_PKT_RAM_2KB &&
            event_set_diag->value <= HAL_MAC_DIAG_SEL_PKT_RAM_16KB) {
            diag_ctrl1.bits.xCt2OGdROySttmwOsmx_ = event_set_diag->value;
            hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, diag_ctrl1.u32);
        }

        /* soc step13 配置数据采集模式:定长模式或循环模式 */
        hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_MODE_REG, 0x0); // '2'b00: 片内固定长度采集（默认）2'b01: 片内循环采数

        hal_mac_test_device->diag_param.diag_cycle = 0;
        wifi_printf("{hal_set_mac_diag_init:mode:%d cfg_pkt_buffer_sel:%d\r\n", event_set_diag->mode,
            event_set_diag->value);
    } else {
        diag_ctrl1.bits.x6NgwhiwidLudLwWh_ = 1; // mac 循环采数模式使能
        // 循环数采触发源配置
        diag_ctrl1.bits.x6NgwFBGgwL_BgwSiFWwSdP_ = 0x1; // cfg_diag_trig_mode_mux[0]=1 待定??
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, diag_ctrl1.u32);
        /* soc step13 配置数据采集模式:定长模式或循环模式 */
        hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_MODE_REG, 0x1); // '2'b00: 片内固定长度采集（默认）2'b01: 片内循环采数

        hal_mac_test_device->diag_param.diag_cycle = 1;
        wifi_printf("{hal_set_mac_diag_init:mode:%d cfg_no_output_sel:%d\r\n", event_set_diag->mode,
            event_set_diag->value);
    }
}

osal_void hal_mac_diag_match_addr_cfg(osal_u32 match_type, osal_u32 match_en, osal_u32 cfg_value)
{
    osal_u32 val_reg;
    osal_u32 match_en_bit[HAL_MAC_TX_RX_DIAG_MATCH_NUM] = {BIT_OFFSET_21, BIT_OFFSET_20, BIT_OFFSET_17,
        BIT_OFFSET_19, BIT_OFFSET_18, BIT_OFFSET_16};
    osal_u32 match_addr_bit[HAL_MAC_TX_RX_DIAG_MATCH_NUM] = {BIT_OFFSET_0, BIT_OFFSET_8, BIT_OFFSET_0,
        BIT_OFFSET_16, BIT_OFFSET_24, BIT_OFFSET_6};
    if (match_en) {
        val_reg = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x18C);
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, val_reg | (0x1 << match_en_bit[match_type]));

        val_reg = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x198);
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x198, val_reg | ((cfg_value << match_addr_bit[match_type]) &
            (HAL_MAC_DIAG_MATCH_ADDR_MASK << match_addr_bit[match_type])));
    } else {
        val_reg = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x18C);
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, val_reg & (~(0x1 << match_en_bit[match_type])));

        val_reg = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x198);
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x198, val_reg &
            (~(HAL_MAC_DIAG_MATCH_ADDR_MASK << match_addr_bit[match_type])));
    }
}

osal_void hal_mac_diag_match_frm_type_cfg(osal_u32 match_type, osal_u32 match_en, osal_u32 cfg_value)
{
    osal_u32 val_reg;
    osal_u32 match_en_bit[HAL_MAC_TX_RX_DIAG_MATCH_NUM] = {BIT_OFFSET_21, BIT_OFFSET_20, BIT_OFFSET_17,
        BIT_OFFSET_19, BIT_OFFSET_18, BIT_OFFSET_16};
    osal_u32 match_addr_bit[HAL_MAC_TX_RX_DIAG_MATCH_NUM] = {BIT_OFFSET_0, BIT_OFFSET_8, BIT_OFFSET_0,
        BIT_OFFSET_16, BIT_OFFSET_24, BIT_OFFSET_6};
    if (match_en) {
        val_reg = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x18C);
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, val_reg | (0x1 << match_en_bit[match_type]));

        val_reg = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x19C);
        hal_reg_write((HH503_MAC_CTRL0_BANK_BASE_0x19C), val_reg | ((cfg_value << match_addr_bit[match_type]) &
            (HAL_MAC_DIAG_MATCH_FRM_MASK << match_addr_bit[match_type])));
    } else {
        val_reg = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x18C);
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, val_reg & (~(0x1 << match_en_bit[match_type])));

        val_reg = hal_reg_read((HH503_MAC_CTRL0_BANK_BASE_0x19C));
        hal_reg_write((HH503_MAC_CTRL0_BANK_BASE_0x19C), val_reg &
            (~(HAL_MAC_DIAG_MATCH_FRM_MASK << match_addr_bit[match_type])));
    }
}

osal_void hal_mac_diag_match_proc(osal_u32 match_type, osal_u32 match_en, osal_u32 cfg_value)
{
    switch (match_type) {
        case HAL_MAC_TX_DIAG_MATCH_RA_ADDR:
            hal_mac_diag_match_addr_cfg(match_type, match_en, cfg_value);
            break;
        case HAL_MAC_TX_DIAG_MATCH_TA_ADDR:
            hal_mac_diag_match_addr_cfg(match_type, match_en, cfg_value);
            break;
        case HAL_MAC_TX_DIAG_MATCH_FRM_TYPE:
            hal_mac_diag_match_frm_type_cfg(match_type, match_en, cfg_value);
            break;
        case HAL_MAC_RX_DIAG_MATCH_RA_ADDR:
            hal_mac_diag_match_addr_cfg(match_type, match_en, cfg_value);
            break;
        case HAL_MAC_RX_DIAG_MATCH_TA_ADDR:
            hal_mac_diag_match_addr_cfg(match_type, match_en, cfg_value);
            break;
        case HAL_MAC_RX_DIAG_MATCH_FRM_TYPE:
            hal_mac_diag_match_frm_type_cfg(match_type, match_en, cfg_value);
            break;
        default:
            oam_error_log1(0, OAM_SF_ANY, "hal_mac_diag_match_proc: invalid match_type:%d", match_type);
            break;
    }
}

osal_void hal_set_mac_diag_test_query(hal_to_dmac_device_mac_test_stru *hal_mac_test_device)
{
    osal_u32 diag_end_en;
    osal_u32 diag_end_addr;
    u_diag_control1 diag_ctrl1;

    diag_end_en = hal_reg_read(DIAG_CTL_RB_SAMPLE_DONE_REG);
    if (diag_end_en & 0x1) {
        diag_end_addr = hal_reg_read(DIAG_CTL_RB_PKE_MEM_OBS_SAMPLE_ADDR_REG);
        wifi_printf("hal_set_mac_diag_test_query: done_addr:0x%x cur_addr:0x%x success!\r\n",
            hal_reg_read(DIAG_CTL_RB_SAMPLE_DONE_ADDR_REG), diag_end_addr);

        diag_ctrl1.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x18C);
        diag_ctrl1.bits.xCt2O4lz2OCyyOmo_ = 0; // MAC、PHY、WIFI和SOC数采使能 使能MAC输出维测主控信号
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, diag_ctrl1.u32);
        hal_reg_write(DIAG_CTL_RB_CFG_SAMPLE_EN_REG, 0x0); // 配置soc数据采集使能寄存

        hal_mac_test_device->diag_param.diag_sample_addr = diag_end_addr + 0x8;
        if (hal_mac_test_device->diag_param.diag_cycle == 1) {
            hal_mac_test_device->diag_param.diag_read_addr = hal_mac_test_device->diag_param.diag_sample_addr;
        } else {
            hal_mac_test_device->diag_param.diag_read_addr =
                hal_mac_test_device->diag_param.diag_basic_start_addr;
        }
    } else {
        hal_mac_test_device->diag_param.diag_read_addr =
                hal_mac_test_device->diag_param.diag_basic_start_addr;
        diag_end_addr = hal_reg_read(DIAG_CTL_RB_PKE_MEM_OBS_SAMPLE_ADDR_REG);
        wifi_printf("hal_set_mac_diag_test_query: done_addr:0x%x cur_addr:0x%x sampling!\r\n",
            hal_reg_read(DIAG_CTL_RB_SAMPLE_DONE_ADDR_REG), diag_end_addr);
    }
}

osal_void hal_set_mac_diag_test_stop(hal_to_dmac_device_mac_test_stru *hal_mac_test_device, osal_u32 mode)
{
    osal_u32 diag_end_addr;
    osal_u32 diag_end_num;
    u_diag_en diag_en;
    u_diag_control1 diag_ctrl1;

    diag_en.u32 = hal_reg_read(HH503_MAC_VLD_BANK_BASE_0x78);

    diag_en.bits.xCt2O4lz2rOmo_ = 0;
    diag_en.bits.xCt2O4lz2KOmo_ = 0;
    diag_en.bits.xCt2O4lz2nOmo_ = 0;
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x78, diag_en.u32);
    if (mode != 0) {
        diag_ctrl1.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x18C);
        diag_ctrl1.bits.x6NgwhiwidLudLwCTwL_Bg_ = 0; // 循环采集停止
        hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, diag_ctrl1.u32);

        diag_end_addr = hal_reg_read(DIAG_CTL_RB_PKE_MEM_OBS_SAMPLE_ADDR_REG);
        diag_end_num = hal_reg_read(HH503_MAC_RD0_BANK_BASE_0x104) & 0x0FFFF;
        wifi_printf("hal_set_mac_diag_test_stop: repeat sample end addr %#x end num %#x(%d)\n",
            diag_end_addr, diag_end_num, diag_end_num & 0xffff);
    }
    hal_set_mac_diag_soc_clear(hal_mac_test_device);
}

osal_void hal_set_mac_diag_param(osal_u8 vap_id, mac_cfg_mct_set_diag_stru *event_set_diag,
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device)
{
    osal_u32 diag_match_type;
    osal_u32 diag_match_en;

    switch (event_set_diag->function_index) {
        case HAL_MAC_DIAG_TEST_INIT:
            hal_set_mac_diag_init(vap_id, event_set_diag, hal_mac_test_device);
            break;
        case HAL_MAC_DIAG_TEST_MATCH: // tx rx匹配
            diag_match_en = event_set_diag->mode & 0x1; // 0x1 取bit0 使能位
            diag_match_type = (event_set_diag->mode >> 1) & 0x7; // 右移1位取3bit
            hal_mac_diag_match_proc(diag_match_type, diag_match_en, event_set_diag->value);
            wifi_printf("{hal_set_mac_diag_param: cfg_match_en:0x%x cfg_match_addr_frm:0x%x\r\n",
                event_set_diag->mode, event_set_diag->value);
            break;
        case HAL_MAC_DIAG_TEST_QUERY: // 11查询采样是否结束
            hal_set_mac_diag_test_query(hal_mac_test_device);
            break;
        case HAL_MAC_DIAG_TEST_STOP: // 12 停止采样
            hal_set_mac_diag_test_stop(hal_mac_test_device, event_set_diag->mode);
            break;
        default:
            wifi_printf("hal_set_mac_diag_param: invalid index:%d\r\n", event_set_diag->function_index);
            break;
    }

    return;
}

osal_void hal_set_mac_diag_mode_deal_dug(osal_void)
{
    osal_u32 val_reg;
    u_diag_en diag_en;
    u_diag_control1 diag_ctrl1;

    diag_ctrl1.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x18C);
    diag_en.u32 = hal_reg_read(HH503_MAC_VLD_BANK_BASE_0x78);

    diag_ctrl1.bits.x6NgwFBGgwqGowCWo_ = 0; // 1维测业务内容 0测试数据
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, diag_ctrl1.u32);

    diag_en.bits.xCt2O4lz2KOmo_ = 1;
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x78, diag_en.u32);

    val_reg = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x19C);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x19C, val_reg | 0xF000); // 0xF000--dword1[15:12]=15

    diag_ctrl1.bits.xCt2O4lz2OCyyOmo_ = 1; // MAC、PHY、WIFI和SOC数采使能 使能MAC输出维测主控信号
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, diag_ctrl1.u32);
}

osal_void hal_set_mac_diag_mode_deal_diag1_test(osal_u32 cfg_value)
{
    u_diag_en diag_en;
    u_diag_control1 diag_ctrl1;
    unref_param(cfg_value);
    diag_ctrl1.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x18C);
    diag_en.u32 = hal_reg_read(HH503_MAC_VLD_BANK_BASE_0x78);

    diag_en.bits.xCt2O4lz2rOmo_ = 1;
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x78, diag_en.u32);

    diag_ctrl1.bits.x6NgwFBGgwqGowCWo_ = 1; // 1维测业务内容 0测试数据

    diag_ctrl1.bits.x6NgwLPwFBGgwWh_ = 1; // 发送上报使能
    diag_ctrl1.bits.x6Ngw_PwFBGgwWh_ = 1; // 接收上报使能
    diag_ctrl1.bits.xCt2O4lz2OIzCOzGyOIv4mOmo_ = 0; // MAC_APB写操作上报维测使能
    diag_ctrl1.bits.xCt2O4lz2OwsslOIv4mOmo_ = 0;    // MAC RSSI模式上报维测使能
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, diag_ctrl1.u32);

    diag_ctrl1.bits.xCt2O4lz2OCyyOmo_ = 1; // MAC、PHY、WIFI和SOC数采使能 使能MAC输出维测主控信号
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, diag_ctrl1.u32);
}

osal_void hal_set_mac_diag_mode_deal_diag4_test(osal_u32 cfg_value)
{
    osal_u32 val_reg;
    u_diag_en diag_en;
    u_diag_control1 diag_ctrl1;

    if (cfg_value < HAL_MAC_DIAG4_NODE_DBAC || cfg_value > HAL_MAC_DIAG4_NODE_TSF_VAP3) {
        wifi_printf("===hal_set_mac_diag_mode_deal_diag4_test test_node[%d] err===\r\n", cfg_value);
        return;
    }
    diag_ctrl1.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x18C);
    diag_en.u32 = hal_reg_read(HH503_MAC_VLD_BANK_BASE_0x78);

    diag_en.bits.xCt2O4lz2KOmo_ = 1;
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x78, diag_en.u32);

    val_reg = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x19C);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x19C, val_reg | 0x3000); // 0x3000--dword1[15:12]=3

    diag_ctrl1.bits.x6NgwFBGgwqGowCWo_ = 1; // 1维测业务内容 0测试数据
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, diag_ctrl1.u32);

    // 特性测试编号选择及特性详细信号mask值
    val_reg = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x1A0);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x1A0, val_reg | cfg_value); // bit[4:0]--0-31
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x1B8, 0x1 << cfg_value);    // 对应bit[31:0]

    diag_ctrl1.bits.xCt2O4lz2OCyyOmo_ = 1; // MAC、PHY、WIFI和SOC数采使能 使能MAC输出维测主控信号
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, diag_ctrl1.u32);
    wifi_printf("===hal_set_mac_diag_mode_deal_diag4_test test_node[%d]===\r\n", cfg_value);
}

osal_void hal_set_mac_diag_mode_deal_diag8_test(osal_u32 cfg_value)
{
    osal_u32 val_reg;
    u_diag_en diag_en;
    u_diag_control1 diag_ctrl1;

    diag_ctrl1.u32 = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x18C);
    diag_en.u32 = hal_reg_read(HH503_MAC_VLD_BANK_BASE_0x78);

    if (cfg_value < HAL_MAC_DIAG8_NODE_H || cfg_value > HAL_MAC_DIAG8_NODE_PHI_PSI) {
        wifi_printf("===hal_set_mac_diag_mode_deal_diag8_test test_node[%d] err===\r\n", cfg_value);
        return;
    }
    diag_en.bits.xCt2O4lz2nOmo_ = 1;
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x78, diag_en.u32);

    val_reg = hal_reg_read(HH503_MAC_CTRL0_BANK_BASE_0x19C);
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x19C, val_reg | 0x7000); // 0x7000--dword1[15:12]=7

    diag_ctrl1.bits.x6NgwFBGgwqGowCWo_ = 1; // 1维测业务内容 0测试数据

    // 待观测的节点编号
    diag_ctrl1.bits.xCt2O4lz2nOov4mOsmx_ = cfg_value; // 0：H内容   1：HV内容  2：SNR内容 3：{PHI_PSI}
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, diag_ctrl1.u32);

    diag_ctrl1.bits.xCt2O4lz2OCyyOmo_ = 1; // MAC、PHY、WIFI和SOC数采使能 使能MAC输出维测主控信号
    hal_reg_write(HH503_MAC_CTRL0_BANK_BASE_0x18C, diag_ctrl1.u32);
    wifi_printf("===hal_set_mac_diag_mode_deal_diag8_test test_node[%d]===\r\n", cfg_value);
}

osal_void hal_set_mac_diag_mode(mac_cfg_mct_set_diag_stru *event_set_diag,
    hal_to_dmac_device_mac_test_stru *hal_mac_test_device)
{
    u_diag_en diag_en;

    diag_en.u32 = hal_reg_read(HH503_MAC_VLD_BANK_BASE_0x78);

    diag_en.bits.xCt2O4lz2rOmo_ = 0;
    diag_en.bits.xCt2O4lz2KOmo_ = 0;
    diag_en.bits.xCt2O4lz2nOmo_ = 0;
    hal_reg_write(HH503_MAC_VLD_BANK_BASE_0x78, diag_en.u32);

    hal_mac_test_device->diag_param.diag_mode = event_set_diag->function_index;

    hal_mac_test_device->diag_param.diag_num = 1;

    switch (event_set_diag->function_index) {
        case HAL_MAC_DIAG_DUG: // 0对应Debug模式 测试维测通路
            hal_set_mac_diag_mode_deal_dug();
            break;
        case HAL_MAC_DIAG_TEST1: // 1对应维测1
            hal_set_mac_diag_mode_deal_diag1_test(event_set_diag->value);
            break;
        case HAL_MAC_DIAG_TEST4: // 4对应维测4
            hal_set_mac_diag_mode_deal_diag4_test(event_set_diag->value);
            break;
        case HAL_MAC_DIAG_TEST8: // 8对应维测8
            hal_set_mac_diag_mode_deal_diag8_test(event_set_diag->value);
            break;
        default:
            oam_error_log1(0, OAM_SF_ANY, "hal_set_mac_diag_mode::invalid index: %d", event_set_diag->function_index);
            break;
    }

    return;
}
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

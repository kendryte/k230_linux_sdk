#ifndef NOC_BANDWIDTH_H
#define NOC_BANDWIDTH_H
//#include <stdint.h>
#include <linux/types.h>


/*
数据通路参考：

Field Type    : Bit(s)
------------------------
InitFlow      : 21 .. 17
TargFlow      : 16 .. 12
Targ SubRange : 11 .. 9


InitFlow field:
~~~~~~~~~~~~~~~
value (0x) : flow
--------------------------------------------------
0          : Flow:/Specification/ai_axi0/I/0
1          : Flow:/Specification/ai_axi1/I/0
2          : Flow:/Specification/cpu0_axi/I/0
3          : Flow:/Specification/cpu1_axi/I/0
4          : Flow:/Specification/display_axi/I/0
5          : Flow:/Specification/dpu_axi/I/0
6          : Flow:/Specification/g2p5d_axi/I/0
7          : Flow:/Specification/hi_sd_axi/I/0
8          : Flow:/Specification/hi_ssi0_axi/I/0
9          : Flow:/Specification/hi_ssi1_2_axi/I/0
A          : Flow:/Specification/hi_usb_ahb/I/0
B          : Flow:/Specification/isp_3dnr/I/0
C          : Flow:/Specification/isp_dwe/I/0
D          : Flow:/Specification/isp_hdr/I/0
E          : Flow:/Specification/isp_mp_mcm/I/0
F          : Flow:/Specification/isp_scal/I/0
10         : Flow:/Specification/sec_axi/I/0
11         : Flow:/Specification/spi2axi_axi/I/0
12         : Flow:/Specification/stor_axi/I/0
13         : Flow:/Specification/vpu_axi/I/0


TargFlow field:
~~~~~~~~~~~~~~~
value (0x) : flow
-----------------------------------------------------------
0          : Flow:/Specification/isp_ahb/T/dw
1          : Flow:/Specification/isp_ahb/T/isp
2          : Flow:/Specification/isp_ahb/T/vi
3          : RESERVED
4          : Flow:/Specification/display_2p5d_ahb/T/display
5          : Flow:/Specification/display_2p5d_ahb/T/g2p5d
6          : Flow:/Specification/sec_apb/T/bootrom
7          : Flow:/Specification/sec_apb/T/sec
8          : Flow:/Specification/ai_cfg/T/0
9          : Flow:/Specification/dpu_apb/T/0
A          : Flow:/Specification/hi_ahb/T/0
B          : Flow:/Specification/ls_apb/T/0
C          : Flow:/Specification/mctl_cfg/T/0
D          : Flow:/Specification/mctl_p0_axi/T/0
E          : Flow:/Specification/mctl_p1_axi/T/0
F          : Flow:/Specification/mctl_p2_axi/T/0
10         : Flow:/Specification/mctl_p3_axi/T/0
11         : Flow:/Specification/mctl_p4_axi/T/0
12         : Flow:/Specification/pmu_apb/T/0
13         : Flow:/Specification/service/T/0
14         : Flow:/Specification/sram_axi0/T/0
15         : Flow:/Specification/sram_axi1/T/0
16         : Flow:/Specification/stor_cfg/T/0
17         : Flow:/Specification/sysctl_apb/T/0
18         : Flow:/Specification/vpu_apb/T/0


Init flow         Targ flow                  Targ subrange : Init mapping , Init localAddress , Targ mapping , Targ localAddress
ai_axi0/I/0       mctl_p3_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
ai_axi0/I/0       mctl_p3_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
ai_axi1/I/0       mctl_p3_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
ai_axi1/I/0       mctl_p3_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
cpu0_axi/I/0      mctl_p4_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
cpu0_axi/I/0      mctl_p4_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
cpu1_axi/I/0      mctl_p0_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
cpu1_axi/I/0      mctl_p0_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
display_axi/I/0   mctl_p2_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
display_axi/I/0   mctl_p2_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
dpu_axi/I/0       mctl_p2_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
dpu_axi/I/0       mctl_p2_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
g2p5d_axi/I/0     mctl_p2_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
g2p5d_axi/I/0     mctl_p2_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
hi_sd_axi/I/0     mctl_p0_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
hi_sd_axi/I/0     mctl_p0_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
hi_ssi0_axi/I/0   mctl_p0_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
hi_ssi0_axi/I/0   mctl_p0_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
hi_ssi1_2_axi/I/0 mctl_p0_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
hi_ssi1_2_axi/I/0 mctl_p0_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
hi_usb_ahb/I/0    mctl_p0_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
hi_usb_ahb/I/0    mctl_p0_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
isp_3dnr/I/0      mctl_p1_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
isp_3dnr/I/0      mctl_p1_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
isp_dwe/I/0       mctl_p1_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
isp_dwe/I/0       mctl_p1_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
isp_hdr/I/0       mctl_p1_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
isp_hdr/I/0       mctl_p1_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
isp_mp_mcm/I/0    mctl_p1_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
isp_mp_mcm/I/0    mctl_p1_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
isp_scal/I/0      mctl_p1_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
isp_scal/I/0      mctl_p1_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
sec_axi/I/0       mctl_p0_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
sec_axi/I/0       mctl_p0_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
stor_axi/I/0      mctl_p4_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
stor_axi/I/0      mctl_p4_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0
vpu_axi/I/0       mctl_p2_axi/T/0            0x0           : 0            , 0x0               , 0            , 0x0
vpu_axi/I/0       mctl_p2_axi/T/0            0x1           : NA           , 0x0               , NA           , 0x0

*/
#define PROBE_MCTLPX_FILER_NUM_MAX 4
typedef enum PROBE_MCTLPX_IDX
{
    PROBE_MCTLP0		= 0x0,
    PROBE_MCTLP1		= 0x1,
    PROBE_MCTLP2		= 0x2,
    PROBE_MCTLP3		= 0x3,
    PROBE_MCTLP4		= 0x4,
    PROBE_MCTLP5		= 0x5,
    PROBE_MCTLP6		= 0x6,
    PROBE_MCTLP_MAX		= 0x7
} probe_mctlpx_idx_t;

typedef struct mctlpx_filters {
    volatile uint32_t Filters_x_RouteIdBase;  //0x80
    volatile uint32_t Filters_x_RouteIdMask; //0x84
    volatile uint32_t Filters_x_AddrBase_Low; //0x88
    volatile uint32_t Reserved_01[1]; //0x8c
    volatile uint32_t Filters_x_WindowSize; //0x90
    volatile uint32_t Reserved_02_03[2];
    volatile uint32_t Filters_x_Opcode; //0x9c
    volatile uint32_t Filters_x_Status; //0xa0
    volatile uint32_t Filters_x_Length; //0xa4
    volatile uint32_t Filters_x_Urgency; // 0xa8
    volatile uint32_t Reserved_20_32[13]; //0xac--0xdc
} mctlpx_filters_t; //0x60

typedef struct mctlpx_counters {
    volatile uint32_t  Reserved_0; //0x200
    volatile uint32_t Counters_x_Src; //0x204
    volatile uint32_t Counters_x_AlarmMode; //0x208
    volatile uint32_t Counters_x_Val; //0x20c
} mctlpx_counters_t; //0x10


typedef struct probe_mctlpx_main_probe {
    volatile uint32_t Id_CoreId;		/* 0x0000 */
    volatile uint32_t Id_RevisionId;		/* 0x0004 */
    volatile uint32_t MainCtl;			/* 0x0008 */
    volatile uint32_t CfgCtl;			/* 0x000c */
    volatile uint32_t Reserved_00[1];		/* 0x0010 */
    volatile uint32_t FilterLut;		/* 0x0014 */
    volatile uint32_t TraceAlarmEn;		/* 0x0018 */
    volatile uint32_t TraceAlarmStatus;		/* 0x001c */
    volatile uint32_t TraceAlarmClr;		/* 0x0020 */
    volatile uint32_t StatPeriod;		/* 0x0024 */
    volatile uint32_t StatGo;			/* 0x0028 */
    volatile uint32_t StatAlarmMin;		/* 0x002c */
    volatile uint32_t Reserved_01; /*0x30*/
    volatile uint32_t StatAlarmMax;		/* 0x0034 */
    volatile uint32_t Reserved_02; /*0x38*/
    volatile uint32_t StatAlarmStatus;		/* 0x003c */
    volatile uint32_t StatAlarmClr;		/* 0x0040 */
    volatile uint32_t StatAlarmEn;		/* 0x0044 */
    volatile uint32_t Reserved_03_16[14];		/* 0x0048--0x7c */
    mctlpx_filters_t filters_0_3[4];		/* 0x0080 0x01fc */ //4*0x60
    // volatile uint32_t Reserved_22[1];		/* x200 */
    mctlpx_counters_t counters_0_7[8];		/* 0x200 0x01D4 */ //0x10*8
	volatile uint32_t reserver_89_184[96];  //0x280-0x3fc
} probe_mctlpx_main_probe_t;//0x400

struct noc_qos_st{//size=0x80
	volatile uint32_t Id_CoreId;  //0x0
	volatile uint32_t Id_RevisionId; //0x4
	volatile uint32_t Priority; //0x8
	volatile uint32_t Mode;//0xc
	volatile uint32_t Bandwidth;//0x10
	volatile uint32_t Saturation;//0x14
	volatile uint32_t ExtControl; //0x18
	volatile uint32_t reserved_00_24[25];//0x1c--0x7c
};

struct noc_rate_st{//size = 0x80
    volatile uint32_t Id_CoreId;
    volatile uint32_t Id_RevisionId;
    volatile uint32_t Rate;
    volatile uint32_t Bypass;
    volatile uint32_t Reserved_00_27[28];
};

struct noc_reg_st{
    struct probe_mctlpx_main_probe mctlpx_main_Probe[7]; //0x0-0x1c00 ;0x400*7
    char reserver[0x100];//0x1c00-0x1d00 observer_main_ErrorLogger_0  observer_main_ErrorLogger_1
	struct noc_qos_st ai_axi0_I_main_QosGenerator;//0x000--0x080
	struct noc_qos_st ai_axi1_I_main_QosGenerator;//0x000--0x080
	struct noc_qos_st cpu0_axi_I_main_QosGenerator;//0x000--0x080
	struct noc_qos_st cpu1_axi_I_main_QosGenerator;//0x000--0x080
	struct noc_qos_st display_axi_I_main_QosGenerator;//0x000--0x080
	struct noc_qos_st g2p5d_axi_I_main_QosGenerator;//0x000--0x080
	struct noc_qos_st isp_3dnr_I_main_QosGenerator;//0x000--0x080
	struct noc_qos_st isp_dwe_I_main_QosGenerator;//0x000--0x080
	struct noc_qos_st isp_hdr_I_main_QosGenerator;//0x000--0x080
	struct noc_qos_st isp_mp_mcm_I_main_QosGenerator;//0x000--0x080
	struct noc_qos_st isp_scal_I_main_QosGenerator;//0x000--0x080
    struct noc_qos_st sec_axi_I_main_QosGenerator;
    struct noc_qos_st stor_axi_I_main_QosGenerator;
    struct noc_qos_st vpu_axi_I_main_QosGenerator;
    struct noc_rate_st main_RateAdapter[6];//Link2_main_RateAdapter---Link9_main_RateAdapter //0x2400-0x2700
    char reserver_1[0]; //-Error---Firewall_ospi_main_Firewall--error
};

typedef enum InitFlow
{
    AI_AXI0 = 0,//     0          : Flow:/Specification/ai_axi0/I/0
    AI_AXI1 = 1,// 1          : Flow:/Specification/ai_axi1/I/0
    CPU0_AXI = 2,// 2          : Flow:/Specification/cpu0_axi/I/0
    CPU1_AXI = 3,// 3          : Flow:/Specification/cpu1_axi/I/0
    DISPLAY_AXI = 4,// 4          : Flow:/Specification/display_axi/I/0
    DPU_AXI = 5,// 5          : Flow:/Specification/dpu_axi/I/0
    G2DP5D_AXI = 6,// 6          : Flow:/Specification/g2p5d_axi/I/0
    HI_SD_AXI = 7,// 7          : Flow:/Specification/hi_sd_axi/I/0
    // 8          : Flow:/Specification/hi_ssi0_axi/I/0
    // 9          : Flow:/Specification/hi_ssi1_2_axi/I/0
    // A          : Flow:/Specification/hi_usb_ahb/I/0
    // B          : Flow:/Specification/isp_3dnr/I/0
    // C          : Flow:/Specification/isp_dwe/I/0
    // D          : Flow:/Specification/isp_hdr/I/0
    // E          : Flow:/Specification/isp_mp_mcm/I/0
    // F          : Flow:/Specification/isp_scal/I/0
    // 10         : Flow:/Specification/sec_axi/I/0
    // 11         : Flow:/Specification/spi2axi_axi/I/0
    STORE_AXI = 12,// 12         : Flow:/Specification/stor_axi/I/0
    // 13         : Flow:/Specification/vpu_axi/I/0
    MCTLP0_ALL			= 0x1C,           // MCTLP0_ALL
    MCTLP1_ALL			= 0x1D,           // MCTLP0_ALL
    MCTLP2_ALL			= 0x1E,           // MCTLP0_ALL
    MCTLP3_ALL			= 0x1F,           // MCTLP0_ALL


} InitFlow_t;

typedef enum TargFlow
{
    // 0          : Flow:/Specification/isp_ahb/T/dw
    // 1          : Flow:/Specification/isp_ahb/T/isp
    // 2          : Flow:/Specification/isp_ahb/T/vi
    // 3          : RESERVED
    // 4          : Flow:/Specification/display_2p5d_ahb/T/display
    // 5          : Flow:/Specification/display_2p5d_ahb/T/g2p5d
    // 6          : Flow:/Specification/sec_apb/T/bootrom
    // 7          : Flow:/Specification/sec_apb/T/sec
    // 8          : Flow:/Specification/ai_cfg/T/0
    // 9          : Flow:/Specification/dpu_apb/T/0
    // A          : Flow:/Specification/hi_ahb/T/0
    // B          : Flow:/Specification/ls_apb/T/0
    // C          : Flow:/Specification/mctl_cfg/T/0
    MCTL_P0_AXI = 0XD,// D          : Flow:/Specification/mctl_p0_axi/T/0
    MCTL_P1_AXI = 0XE,// E          : Flow:/Specification/mctl_p1_axi/T/0
    MCTL_P2_AXI = 0XF,// F          : Flow:/Specification/mctl_p2_axi/T/0
    MCTL_P3_AXI = 0X10,// 10         : Flow:/Specification/mctl_p3_axi/T/0
    MCTL_P4_AXI = 0X11,// 11         : Flow:/Specification/mctl_p4_axi/T/0
    // 12         : Flow:/Specification/pmu_apb/T/0
    // 13         : Flow:/Specification/service/T/0
    // 14         : Flow:/Specification/sram_axi0/T/0
    // 15         : Flow:/Specification/sram_axi1/T/0
    // 16         : Flow:/Specification/stor_cfg/T/0
    // 17         : Flow:/Specification/sysctl_apb/T/0
    // 18         : Flow:/Specification/vpu_apb/T/0


} TargFlow_t;

#define PROBE_MCTLP0_ID_COREID		0x04C9DF06
#define PROBE_MCTLP0_ID_REVISIONID	0xB085B900
#define PROBE_MCTLP1_ID_COREID		0x59669906
#define PROBE_MCTLP1_ID_REVISIONID	0xB085B900
#define PROBE_MCTLP2_ID_COREID		0xDD60C606
#define PROBE_MCTLP2_ID_REVISIONID	0xB085B900
#define PROBE_MCTLP3_ID_COREID		0xC2B1C606
#define PROBE_MCTLP3_ID_REVISIONID	0xB085B900

#define MAINCTL_ERREN				0x1
#define MAINCTL_TRACEEN				(0x1 << 1)
#define MAINCTL_PAYLOADEN			(0x1 << 2)
#define MAINCTL_STATEN				(0x1 << 3)
#define MAINCTL_ALARMEN				(0x1 << 4)
#define MAINCTL_STATCONDDIMP			(0x1 << 5)
#define MAINCTL_INTRUSIVEMODE			(0x1 << 6)
#define MAINCTL_FILTBYTEALWAYSCHAINABLEEN	(0x1 << 7)

#define CFGCTL_GLOBALEN				0x1
#define CFGCTL_ACTIVE				(0x1 << 1)

#define FILTERS_X_OPCODE_RDEN			0x1
#define FILTERS_X_OPCODE_WREN			(0x1 << 1)
#define FILTERS_X_OPCODE_LOCKEN			(0x1 << 2)
#define FILTERS_X_OPCODE_URGEN			(0x1 << 3)

#define FILTERS_X_STATUS_REQEN			0x1
#define FILTERS_X_STATUS_RSPEN			(0x1 << 1)


typedef enum Counters_M_Src
{
    OFF				= 0x0,		/* Counter disabled */
    CYCLE			= 0x1,		/* Probe clock cycles */
    IDLE			= 0x2,		/* Idle cycles during which no packet data is observed */
    XFER			= 0x3,		/* Transfer cycles during which packet data is transferred */
    BUSY			= 0x4,		/* Busy cycles during which the packet data is made available by the
						   transmitting agent but the receiving agent is not ready to receive it */
    WAIT			= 0x5,		/* Wait cycles during a packet in which the transmitting agent suspends
						   the transfer of packet data */
    PKT				= 0x6,		/* Packets */
    LUT				= 0x7,		/* Packets selected by the LUT */
    BYTE			= 0x8,		/* Total number of payload bytes */
    PRESS0			= 0x9,		/* Clock cycles with pressure level > 0 */
    PRESS1			= 0xA,		/* Clock cycles with pressure level > 1 */
    PRESS2			= 0xB,		/* Clock cycles with pressure level > 2 */
    FILT0			= 0xC,		/* Packets selected by Filter 0 */
    FILT1			= 0xD,		/* Packets selected by Filter 1 */
    FILT2			= 0xE,		/* Packets selected by Filter 2 */
    FILT3			= 0xF,		/* Packets selected by Filter 3 */
    CHAIN			= 0x10,		/* Carry from counter 2m to counter 2m + 1 */
    LUT_BYTE_EN			= 0x11,		/* Enabled payload byte in packets selected by the LUT */
    LUT_BYTE			= 0x12,		/* Total number of payload bytes in packets selected by the LUT */
    FILT_BYTE_EN		= 0x13,		/* Enabled payload byte in packets selected by the associated filter */
    FILT_BYTE			= 0x14		/* Total number of payload bytes in packets selected by the associated
						   filter */
} Counters_M_Src_t;

#define SEQID_OFFSET			0
#define TARG_SUBRANGE_OFFSET		9
#define TARG_FLOW_OFFSET		12
#define INIT_FLOW_OFFSET		17


#define FILTERS_M_ROUTEIDMASK		0xFFF
#define FILTERS_M_ROUTEIDMASK_OFFSET	TARG_SUBRANGE_OFFSET

#define FILTER_M_LENGTH			0xF

#define FILTER_0_INUSE 0x1
#define FILTER_1_INUSE (0x1 << 1)
#define FILTER_2_INUSE (0x1 << 2)
#define FILTER_3_INUSE (0x1 << 3)


typedef enum PROBE_STATUS {
        PROBE_IDLE = 0,
        PROBE_INUSE = 1,
        PROBE_ERROR = 2
}probe_status_t;

struct probe_mctlpx_stat {
        probe_status_t probe_stat;
        unsigned int filters_inuse;

        unsigned int clk_freq;
        unsigned long cpu_cycles;  //not use
        //unsigned int counters[8];
		unsigned long port_max_count;
		//unsigned long port_max_counth;
		unsigned long filter_max_count[PROBE_MCTLPX_FILER_NUM_MAX];
        unsigned int stat_period;
        unsigned long filter_RouteIdBase[PROBE_MCTLPX_FILER_NUM_MAX];
};
struct noc_plat {
    //void __iomem    *reg;   /* virtual address */
	struct noc_reg_st *noc_reg;
    struct probe_mctlpx_stat  probe_run_status[PROBE_MCTLP_MAX];
	unsigned long all_probe_max_count;
    struct task_struct *my_thread;
	//unsigned long max_counth;
};



//extern struct probe_mctlpx_stat probe_mctlpx_status[4];

/* configure probe_mctlpx_main_probe according to the initflow parameter.
 *
 * Please note: if you configure more than 1 flow in one mctl port, for example:
 * at first you configure DISPLAYAXIM,  and then configure ISPAXIM_R2K, both of the
 * operation will use the probe_mctlp2_main_probe,  the parameters such as
 * startaddr, windowsize and stat_period will be configured two times, and therefor,
 * only the last time you configured for ISPAXIM_R2K will be applied when you enable it.
 *
 * @initflow:           initflow to set, InitFlow_t type, invalid value is not accepted
 * @startaddr:          start address to set for the probe to monitor
 * @windowsize:         windowsize to set for the probe to minotor, with the startaddr
 * 			parameter to specify which address access will be monitored
 * @stat_period:        period to statistics
 *
 * return value:        mctl port index(value from 0 to 3) for success, -1 for failure
 *
 * */
extern probe_mctlpx_idx_t noc_probe_mctl_cfg(struct noc_plat  *noc, \
						InitFlow_t initflow,  unsigned int startaddr, unsigned int windowsize, unsigned int stat_period);



/* enable probe_mctlpx_main_probe to start statistics.
 *
 * @port_idx:        mctl port index, which is return by noc_probe_mctl_cfg.
 *
 * */
extern void noc_probe_mctlpx_enable(struct noc_plat *noc, probe_mctlpx_idx_t port_idx);


/* disable probe_mctlpx_main_probe to stop statistics.
 *
 * @port_idx:        mctl port index, which is return by noc_probe_mctl_cfg.
 *
 * */
extern void noc_probe_mctlpx_disable(struct noc_plat *noc, probe_mctlpx_idx_t port_idx);


/* read counters for all mctl ports, if the counter for high 16bits warps, print the counters value and
 * calculate the bandwidth.
 * can be called in in loop with short sleep, for example usleep(10), for accuracy.
 *
 * */
//extern void noc_probe_mctl_bandwidth_show(struct noc_plat *noc);


/* get counters value according to the port index and the filter index.
 *
 * @port_idx:           mctl port index, which is return by noc_probe_mctl_cfg.
 * @filter_idx:         filter index,
 *
 * return value:        return value of total bytes of the filter
 *
 * */
extern unsigned int noc_probe_get_counter_val(probe_mctlpx_idx_t port_idx, unsigned int filter_idx);


#endif

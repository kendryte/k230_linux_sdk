//#include <math.h>
#include <linux/types.h>

#include <linux/module.h>
#include <linux/seq_file.h>
//#include <k230.h>
//#include <sysctl_clk.h>
#include "noc.h"
#include <linux/printk.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/bitops.h>

/*
modprobe noc.ko;  #安装驱动
echo kpu > /sys/devices/platform/k230-noc/probe  #使能统计
cat /sys/devices/platform/k230-noc/probe  #查看结果
*/

struct initflow_config_st
{
	unsigned int initflow;
	char * str;
	int targ_flow;
	char *targ_str;
	unsigned int startaddr;
	int port_id;
	int port_have_filter_num; //probe 有几个filter
	unsigned int clk;
};

extern char *next_arg(char *args, char **param, char **val);
static void noc_probe_store_help(void);


static int noc_kthread_function_read_count(struct noc_plat *noc)
{
	int probe_i,filter_j;
	int fs = 0, fl = 0;
	unsigned long p_count=0;
	unsigned long allp_count = 0;


	//printk(KERN_INFO "Kernel thread is running. Loop count: %d\n", loop_count);
	// 线程休眠 2 秒
	for(probe_i=0; probe_i< PROBE_MCTLPX_FILER_NUM_MAX; probe_i++){ //probe
		struct probe_mctlpx_stat *port_sta =  &noc->probe_run_status[probe_i];
		probe_mctlpx_main_probe_t * probe_base = &noc->noc_reg->mctlpx_main_Probe[probe_i];
		unsigned long temp;

		if((port_sta->probe_stat != PROBE_INUSE) || (port_sta->filters_inuse == 0))
			continue;

		fs = ffs(port_sta->filters_inuse);
		fl = fls(port_sta->filters_inuse);
		p_count = 0;

		for(filter_j=fs-1; filter_j<fl; filter_j++){ //filter
			//struct initflow_config_st info;
			unsigned long  count_low, count_h;
			count_low = probe_base->counters_0_7[2*filter_j].Counters_x_Val;
			count_h = probe_base->counters_0_7[(2*filter_j)+1].Counters_x_Val;
			if((count_low > 0xffff) && (count_h > 0xffff)){
				//printk("error all fff =%lx %lx ,PORT=%d filter=%d \n", count_low, count_h, probe_i, filter_j);
				//port_sta->probe_stat = PROBE_ERROR;
				continue;
			}
			temp = count_h<<16 | count_low;
			if(temp > port_sta->filter_max_count[filter_j]){
				port_sta->filter_max_count[filter_j] = temp;
			}
			p_count += temp;
		}
		if(p_count > port_sta->port_max_count){
			port_sta->port_max_count = p_count;
		}
		allp_count += p_count;
	}
	if(allp_count > noc->all_probe_max_count){
		noc->all_probe_max_count = allp_count;
	}
	return 0;
}
// 定义内核线程函数
static int noc_kthread_function(void *data)
{
    int loop_count = 0;

	struct noc_plat *noc = (struct noc_plat *)data;
    // 检查内核线程是否被要求停止
    while (!kthread_should_stop()) {
		msleep(30);
		loop_count++;
		noc_kthread_function_read_count(noc);
    }
    printk(KERN_INFO "Kernel thread is exiting. loop_count=%x\n", loop_count);
    return 0;
}


static int  noc_probe_get_initfow_config_info(InitFlow_t initflow, int display_flag, struct initflow_config_st *info)
{
	int i=0;

	struct initflow_config_st array[]={
		{AI_AXI0,   "ai_axi0/I/0",  MCTL_P3_AXI, "mctl_p3_axi/T/0", 0x00000000,  PROBE_MCTLP1, 4, 400000000},//ddrc_p3_clk,比较奇怪 mctl p3要用main_probe_p1
		{AI_AXI1,   "ai_axi0/I/0",  MCTL_P3_AXI,"mctl_p3_axi/T/0", 0x00000000,  PROBE_MCTLP1, 4, 400000000},
		{STORE_AXI,   "stor_axi/I/0",  MCTL_P4_AXI,"mctl_p4_axi/T/0", 0x00000000,  PROBE_MCTLP4, 4, 0x1dcd6500}, //ddrc_p4_clk,范俊涛使用的这个例子；
	};

	for(i=0;i<ARRAY_SIZE(array);i++) {
		if(display_flag){
			pr_info("init [0x%02x %13.13s->0x%02x %-9.9s] port=%d filter=%d \n", \
				array[i].initflow, array[i].str, array[i].targ_flow,array[i].targ_str, array[i].port_id, array[i].port_have_filter_num);
		}
		else if( array[i].initflow == initflow ){
			if(info)
				memcpy(info, &array[i], sizeof(*info));
			return 0;
		}
	}
	return 1;
}





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
 *                      parameter to specify which address access will be monitored
 * @stat_period:        period to statistics
 *
 * return value:        mctl port index(value from 0 to 3) for success, -1 for failure
 *
 * */
probe_mctlpx_idx_t noc_probe_mctl_cfg(struct noc_plat *noc,InitFlow_t initflow, unsigned int startaddr, unsigned int windowsize, unsigned int stat_period)
{
	probe_mctlpx_main_probe_t *noc_probe_reg = NULL;
	struct probe_mctlpx_stat *run_status =NULL;
	mctlpx_filters_t *filter_base;
	mctlpx_counters_t *counter_low;
	mctlpx_counters_t *counter_high;
	unsigned int data;
	unsigned int filter_idx = 0;

	struct initflow_config_st config;
	if(noc_probe_get_initfow_config_info(initflow, 0, &config)){
		pr_info("input error \n");
		noc_probe_store_help();
		return -1;
	}

	noc_probe_reg = &noc->noc_reg->mctlpx_main_Probe[config.port_id];
	run_status = &noc->probe_run_status[config.port_id];

	/* get filter_base and counter_base by filter_idx */
	filter_idx = ffz(run_status->filters_inuse);

	if((filter_idx+1) > config.port_have_filter_num){
		pr_info("porbe %d resources not enough, use fiter %d ,but total filter %d \n", config.port_id,filter_idx, config.port_have_filter_num);
		return -1;
	}

	filter_base = &noc_probe_reg->filters_0_3[filter_idx];
	counter_low = &noc_probe_reg->counters_0_7[2*filter_idx];
	counter_high = &noc_probe_reg->counters_0_7[2*filter_idx+1];

	pr_info("port%02d.filter%02d  [0x%02x %13.13s->0x%02x %-9.9s] period:%x\n", \
						config.port_id,filter_idx, initflow, config.str, config.targ_flow, config.targ_str, stat_period);


	/* if the probe is active, disable it and wait until active bit is clear */
	if(noc_probe_reg->CfgCtl & CFGCTL_ACTIVE){
		noc_probe_reg->CfgCtl &= ~CFGCTL_GLOBALEN;

		while(noc_probe_reg->CfgCtl & CFGCTL_ACTIVE){
			msleep_interruptible(10);
		}
	}



	/* enable statistics noc_probe and chained counters */
	data = noc_probe_reg->MainCtl;
	data |= MAINCTL_STATEN | MAINCTL_FILTBYTEALWAYSCHAINABLEEN;
	noc_probe_reg->MainCtl = data; //devmem 0x99903008 32



	/* set Filters_x_RouteIdBase */
	data = 0;
	data = (initflow << INIT_FLOW_OFFSET) | (config.targ_flow << TARG_FLOW_OFFSET);
	filter_base->Filters_x_RouteIdBase = data; // devmem 0x99903044 32
	run_status->filter_RouteIdBase[filter_idx] = data;

	if(initflow >= MCTLP0_ALL ){ //filter all
				/* set Filters_x_RouteIdMask */
		filter_base->Filters_x_RouteIdMask = 0x0;// devmem 0x99903048 32
		filter_base->Filters_x_AddrBase_Low = startaddr; // devmem 0x9990304c 32
		filter_base->Filters_x_WindowSize = 0xffffffff; //devmem 0x99903050 32
		filter_base->Filters_x_Opcode = 0xf;  // devmem 0x99903060 32
		filter_base->Filters_x_Status = 0x3;
		filter_base->Filters_x_Length = 0xf;
		filter_base->Filters_x_Urgency = 0;
		//filter_base->Filters_x_UserMask = 0;
		pr_info("filter all %x %x \n", initflow, filter_base->Filters_x_RouteIdMask);
	}else{

		/* set Filters_x_RouteIdMask */
		filter_base->Filters_x_RouteIdMask = FILTERS_M_ROUTEIDMASK << FILTERS_M_ROUTEIDMASK_OFFSET;// devmem 0x99903048 32

		/* set Filters_x_AddrBase_Low */
		filter_base->Filters_x_AddrBase_Low = startaddr; // devmem 0x9990304c 32

		/* set Filters_x_WindowSize  */
		filter_base->Filters_x_WindowSize = windowsize; //devmem 0x99903050 32
		/* set Filters_x_Opcode to 0x3 for rd/wr */
		filter_base->Filters_x_Opcode = FILTERS_X_OPCODE_RDEN | FILTERS_X_OPCODE_WREN;  // devmem 0x99903060 32

		/* set Filters_x_Status to 0x1 for request */
		filter_base->Filters_x_Status = FILTERS_X_STATUS_REQEN;
		/* set Filters_x_Length to 0xf for packet number less than 2^15 */
		filter_base->Filters_x_Length = FILTER_M_LENGTH;
	}

	//pr_info("Filters_x_RouteIdBase %x %lx,%x \n",
	//		filter_base->Filters_x_RouteIdBase,&filter_base->Filters_x_RouteIdBase,filter_base->Filters_x_RouteIdMask);


	/* set Counters low Src to 0x14 for packet select by filterx */
	counter_low->Counters_x_Src = FILT_BYTE; //devmem 0x99903138 32
	//counter_low->Counters_x_AlarmMode = 0;//0x313c

	/* set Counters high Src to 0x10 to chain with counter low */
	counter_high->Counters_x_Src = CHAIN; //devmem 0x9990314c 32
	//counter_high->Counters_x_AlarmMode = 0;//0x3150

	/* set StatPeriod to stat_period, 2^stat_period/probe_clk = time in seconds */
	if(stat_period > 0x1F){
		stat_period = 0x1F;
		pr_info("stat_period exceed max value, set to 0x1F(31)\n");
	}
	noc_probe_reg->StatPeriod = stat_period;
	//noc_probe_reg->StatAlarmMin=0x06c20000;
	//noc_probe_reg->StatAlarmMax=0x06c00000;



	/* set field GlobalEn of register CfgCtl to start counting */
//	noc_probe_reg->CfgCtl = CFGCTL_GLOBALEN;
//	run_status->cpu_cycles = read_cycle();
//	run_status->probe_stat = PROBE_INUSE;
	run_status->filters_inuse |= 0x1 << filter_idx;
	run_status->stat_period = stat_period;
	run_status->clk_freq = config.clk;

	return config.port_id;
}


/* enable probe_mctlpx_main_probe to start statistics.
 *
 * @port_idx:        mctl port index, which is return by noc_probe_mctl_cfg.
 *
 * */
void noc_probe_mctlpx_enable(struct noc_plat *noc, probe_mctlpx_idx_t port_idx)
{
    probe_mctlpx_main_probe_t *probe_base = &noc->noc_reg->mctlpx_main_Probe[port_idx];
	struct probe_mctlpx_stat *run_status = &noc->probe_run_status[port_idx];


	/* set field GlobalEn of register CfgCtl to start counting */
	probe_base->CfgCtl = CFGCTL_GLOBALEN;

	run_status->cpu_cycles = 0x80000000;//read_cycle();
	run_status->probe_stat = PROBE_INUSE;
	//memset(run_status->counters, 0 , sizeof(run_status->counters));
	run_status->port_max_count = 0;
	memset(run_status->filter_max_count, 0 ,sizeof(run_status->filter_max_count));

	pr_info("probe_mctlpx_status[%d] probe_stat = %d \n", port_idx, run_status->probe_stat);

	// 创建并启动内核线程
	noc->my_thread = kthread_run(noc_kthread_function, noc, "noc_kthread");
	if (IS_ERR(noc->my_thread)) {
		printk(KERN_ERR "Failed to start kernel thread\n");
	}

}

/* disable probe_mctlpx_main_probe to stop statistics.
 *
 * @port_idx:        mctl port index, which is return by noc_probe_mctl_cfg.
 *
 * */
void noc_probe_mctlpx_disable(struct noc_plat *noc, probe_mctlpx_idx_t port_idx)
{
	probe_mctlpx_main_probe_t *probe_base = &noc->noc_reg->mctlpx_main_Probe[port_idx];
	struct probe_mctlpx_stat *run_status = &noc->probe_run_status[port_idx];

	/* clear bit GlobalEn of register CfgCtl to stop counting */
	probe_base->CfgCtl &= ~CFGCTL_GLOBALEN;

	run_status->probe_stat = PROBE_IDLE;
	memset(run_status, 0, sizeof(*run_status));

	pr_info("probe_mctlpx_status[%d] probe_stat = %d \n", port_idx, run_status->probe_stat);
}



static void noc_probe_store_help(void) {
	pr_info("\n\nusage: \n");
	pr_info("	kpu\n" );
	pr_info("	start  [0/1/2/3]\n" );
	pr_info("	stop [0/1/2/3] \n");
	pr_info("	cfg initflow startaddr  windowsize stat_period\n"
				"cfg [0x0/../0x14]  0x0  0x1f 0x8\n"
		);
	pr_info("	\ninitflow config  is :\n");
	noc_probe_get_initfow_config_info(0,1,0);
}



static void noc_probe_store_cfg(char *args, struct noc_plat *noc)
{
	char *param="";
	char *val="";


	int initflow_num =0;
	int i=0;
	InitFlow_t initflows[11] ;
	unsigned int startaddr = 0;
	unsigned int windowsize  = 0x1f;
	unsigned int stat_period = 0x1e;

	//noc_probe_mctl_cfg(noc, STORAHBM_SDC1AHBM, 0, 0x1f, 0x1e); //alway 0


	param = val="";
	args = next_arg(args, &param, &val);
	//pr_info("a=%s v=%s\n", param, val);
	if(strcmp(param, "initflow") == 0) {

		args = skip_spaces(val);
		while (*args)
		{
			args = next_arg(args, &param, &val);
			//pr_info("p=%s v=%s args =%s\n", param, val,args);
			initflows[initflow_num] = simple_strtoul(param, NULL, 16);
			initflow_num++;
		}

	}else {
		initflows[initflow_num] = simple_strtoul(param, NULL, 16);
		initflow_num = 1;
		if(*args)
		{
			param = val="";
			args = next_arg(args, &param, &val);
			startaddr = simple_strtoul(param, NULL, 16);
		}

		if(*args)
		{
			param = val="";
			args = next_arg(args, &param, &val);
			windowsize = simple_strtoul(param, NULL, 16);
		}

		if(*args)
		{
			param = val="";
			args = next_arg(args, &param, &val);
			stat_period = simple_strtoul(param, NULL, 16);
		}

	}


	for(i=0;i<initflow_num; i++){
		//pr_info("i=%d %x\n ", i, initflows[i]);
		noc_probe_mctl_cfg(noc, initflows[i], startaddr, windowsize, stat_period);
	}

}
// static void noc_probe_store_cfg_ddr_all(char *args, struct noc_plat *noc)
// {
// 	memset(noc->probe_run_status,0, sizeof(noc->probe_run_status));
// 	noc_probe_mctl_cfg(noc, MCTLP0_ALL, 0, 0x1f, 0x1e);
// 	noc_probe_mctl_cfg(noc, MCTLP1_ALL, 0, 0x1f, 0x1e);
// 	noc_probe_mctl_cfg(noc, MCTLP2_ALL, 0, 0x1f, 0x1e);
// 	noc_probe_mctl_cfg(noc, MCTLP3_ALL, 0, 0x1f, 0x1e);

// 	#if 0
// 	noc_probe_mctl_cfg(noc, AX25MAXIM, 0, 0x1f, 0x1e);
// 	noc_probe_mctl_cfg(noc, AX25PAXIM, 0, 0x1f, 0x1e);

// 	noc_probe_mctl_cfg(noc, GNNEAXIM  , 0, 0x1f, 0x1e);
// 	noc_probe_mctl_cfg(noc, AIDATAAXIM, 0, 0x1f, 0x1e);

// 	//noc_probe_mctl_cfg(noc, ISPAXIM_R2K, 0, 0x1f, 0x1e); // always 0
// 	//noc_probe_mctl_cfg(noc, ISPAXIM_VI,  0, 0x1f, 0x1e); //alway 0
// 	noc_probe_mctl_cfg(noc, DISPLAYAXIM, 0, 0x1f, 0x1e);
// 	//noc_probe_mctl_cfg(noc, ISPAXIM_3DTOF, 0, 0x1f, 0x1e);// alway 0
// 	noc_probe_mctl_cfg(noc, ISPAXIM_F2K,  0, 0x1f, 0x1e);
// 	noc_probe_mctl_cfg(noc, TWODAXIM, 	 0, 0x1f, 0x1e);
// 	//noc_probe_mctl_cfg(noc, ISPAXIM_MFCB, 0, 0x1f, 0x1e); //alway 0
// 	noc_probe_mctl_cfg(noc, H264AXIM, 	 0, 0x1f, 0x1e);


// 	noc_probe_mctl_cfg(noc, STORAHBM_EMACAHBM, 0, 0x1f, 0x1e); // <0x1000
// 	//noc_probe_mctl_cfg(noc, USBAHBM_SD3AHBM,   0, 0x1f, 0x1e);//alway 0
// 	//noc_probe_mctl_cfg(noc, STORAHBM_SDC2AHBM, 0, 0x1f, 0x1e); //alway 0
// 	noc_probe_mctl_cfg(noc, PDMAAXIM,    	 0, 0x1f, 0x1e); //alway 0
// 	//noc_probe_mctl_cfg(noc, SDMAAXIM,    	 0, 0x1f, 0x1e); //alway 0
// 	//noc_probe_mctl_cfg(noc, STORAHBM_SDC0AHBM, 0, 0x1f, 0x1e); //alway 0
// 	//noc_probe_mctl_cfg(noc, USBAHBM_USBAHBM,   0, 0x1f, 0x1e); //alway 0
// 	//noc_probe_mctl_cfg(noc, STORAHBM_SDC1AHBM, 0, 0x1f, 0x1e); //alway 0

// 	#endif



// }

static void noc_probe_store_start_or_end(char *args, struct noc_plat *noc, int start_flag)
{
	char *param="";
	char *val="";
	unsigned int probe_id=0;

	if(*args == 0){
		pr_info("invalid param\n");
		return ;
	}


	param = val="";
	args = next_arg(args, &param, &val);

	if(strcmp(param, "all") == 0){
		if(start_flag) {
			noc_probe_mctlpx_enable(noc, PROBE_MCTLP0);
			noc_probe_mctlpx_enable(noc, PROBE_MCTLP1);
			noc_probe_mctlpx_enable(noc, PROBE_MCTLP2);
			noc_probe_mctlpx_enable(noc, PROBE_MCTLP3);
		}else {
			noc_probe_mctlpx_disable(noc, PROBE_MCTLP0);
			noc_probe_mctlpx_disable(noc, PROBE_MCTLP1);
			noc_probe_mctlpx_disable(noc, PROBE_MCTLP2);
			noc_probe_mctlpx_disable(noc, PROBE_MCTLP3);
		}
		noc->all_probe_max_count=0;
	}
	else {
		probe_id = simple_strtoul(param, NULL, 16);
		if(start_flag) {
			noc_probe_mctlpx_enable(noc, probe_id);
		} else {
			noc_probe_mctlpx_disable(noc, probe_id);
		}
	}
}




/* read counters for all mctl ports, if the counter for high 16bits warps, print the counters value and
 * calculate the bandwidth.
 * can be called in in loop with short sleep, for example usleep(10), for accuracy.
 *
 * */
int  noc_probe_mctl_bandwidth_show(struct noc_plat *noc, char *buf, int buflen)
{
	int probe_i,filter_j;
	int fs = 0, fl = 0;
	int cur=0;

	//unsigned long  time;
	//unsigned long  speed = 0;
	unsigned long fillter_speed;
	unsigned long probe_speed;


	for(probe_i=0; probe_i < PROBE_MCTLPX_FILER_NUM_MAX; probe_i++){ //probe

		struct probe_mctlpx_stat *sta =  &noc->probe_run_status[probe_i];
		//probe_mctlpx_main_probe_t * probe_base = &noc->noc_reg->mctlpx_main_Probe[probe_i];
		if((sta->probe_stat == PROBE_IDLE) || (sta->filters_inuse == 0))
			continue;

		fs = ffs(sta->filters_inuse);
		fl = fls(sta->filters_inuse);

		for(filter_j=fs-1; filter_j<fl; filter_j++){ //filter
			struct initflow_config_st info;
			memset(&info, 0, sizeof(info));

			InitFlow_t initflow = (sta->filter_RouteIdBase[filter_j] >> INIT_FLOW_OFFSET) & 0X1F;

			//printk("wjx id=%x %x\n", probe_base->filters_0_3[filter_j].Filters_x_RouteIdBase, initflow);

			if(noc_probe_get_initfow_config_info(initflow,0, &info))
				printk("errror id=%lx %x\n", sta->filter_RouteIdBase[filter_j], initflow);

			fillter_speed =( sta->filter_max_count[filter_j]*(sta->clk_freq)/((1UL<<sta->stat_period) -1) )>>20;
			//printk("f=%s l=%d\n", __func__, __LINE__);
			//printk("wjx %d %d %lx\n", probe_i, filter_j, sta->filter_max_count[filter_j]);
			cur +=snprintf(&buf[cur], buflen - cur, \
				"p%02d.%02d count[0x%08lx]  [0x%02x %13.13s] refspeed=%ld MB/s \n" ,\
						probe_i, filter_j, sta->filter_max_count[filter_j], initflow, info.str, fillter_speed);

			sta->filter_max_count[filter_j] = 0;
		}

		probe_speed = ((sta->port_max_count) *(sta->clk_freq)/((1UL<<sta->stat_period) -1))>>20;
		cur +=snprintf(&buf[cur], buflen - cur, \
			"----p.%02d  count[0x%08lx] StatPeriod 0x%x speed=%ld MB/s\n" ,\
										probe_i, sta->port_max_count,  sta->stat_period, probe_speed);
		sta->port_max_count = 0;

	}
	//speed =( noc->all_probe_max_count*(400000000)/((1UL<<0x1e) -1) )>>20;
	cur +=snprintf(&buf[cur], buflen - cur, \
		"--------all port toatal max count[0x%08lx] \n" , noc->all_probe_max_count);
		noc->all_probe_max_count = 0;

	cur +=snprintf(&buf[cur], buflen - cur, \
		"speed calc method: speed=(count*freq/(1<<StatPeriod-1)>>20) MB/s\n" );

	return cur;
}



//读操作
static ssize_t noc_probe_show(struct device *dev, struct device_attribute *attr,
        char *buf)
{
	struct noc_plat *noc = dev_get_drvdata(dev);
	return noc_probe_mctl_bandwidth_show(noc, buf, PAGE_SIZE);
}
static void __ai_enable_power_clk(void)
{
	uint32_t *reg;
	unsigned long v;
	// // disable ai power
    // if (readl((void*)0x9110302c) & 0x2)
    //     writel(0x30001, (void*)0x91103028);

	    // // disable ai clk
    // value = readl((void*)0x91100008);
    // value &= ~((1 << 0));
    // writel(value, (void*)0x91100008);

	reg = ioremap(0x91100008, 4); //检测时钟
	v = *reg;
	if( (!test_bit(10, &v)) || (!test_bit(0, &v))  ){
		writel(v|BIT(0)|BIT(10), reg);
		printk("enable ai clk\n");
	}
	iounmap(reg);

	reg = ioremap(0x9110302c, 4); //power status
	v = *reg;
	iounmap(reg);

	if(!test_bit(1,&v)){
		printk("enable ai power\n");
		reg = ioremap(0x91103028, 4);
		v = *reg;
		writel(v|BIT(1)|BIT(17), reg);
		iounmap(reg);
	}
}
static void __vpu_enable_power_clk(void)
{
	uint32_t *reg;
	unsigned long v;
	 // disable vpu power
    // if (readl((void*)0x91103080) & 0x2)
    //     writel(0x30001, (void*)0x9110307c);

    // disable vpu clk
    // value = readl((void*)0x9110000c);
    // value &= ~((1 << 0));
    // writel(value, (void*)0x9110000c);

	reg = ioremap(0x9110000c, 4); //检测时钟
	v = *reg;
	if( (!test_bit(10, &v)) || (!test_bit(0, &v))  ){
		writel(v|BIT(0)|BIT(10), reg);
		printk("enable vpu clk\n");
	}
	iounmap(reg);

	reg = ioremap(0x91103080, 4); //power status
	v = *reg;
	iounmap(reg);

	if(!test_bit(1,&v)){
		printk("enable vpu power\n");
		reg = ioremap(0x9110307c, 4);
		v = *reg;
		writel(v|BIT(1)|BIT(17), reg);
		iounmap(reg);
	}
}

//写操作
static ssize_t noc_probe_store(struct device *dev, struct device_attribute *attr,
         const char *buf, size_t count)
{

	char *param="";
	char *val="";
	char *args = NULL;
	char *buf_tmp = kzalloc(count+2,0);

	struct noc_plat *noc = dev_get_drvdata(dev);

	args =  buf_tmp;
	memcpy(args, buf, count);


	args = skip_spaces(args);
	args = next_arg(args, &param, &val);


	if(strcmp(param, "all_ddr") == 0){
		// pr_info("ddr_all\n");
		// noc_probe_store_cfg_ddr_all(args,noc);
	}else if(strcmp(param, "kpu") == 0)	{
		pr_info("AI_AXI0 and  AI_AXI1 -->mp3 use main_probe1 \n");
		__ai_enable_power_clk();
		noc_probe_mctlpx_disable(noc, PROBE_MCTLP1);
		noc_probe_mctl_cfg(noc, AI_AXI0, 0, 0x1f, 0x16);
		noc_probe_mctl_cfg(noc, AI_AXI1, 0, 0x1f, 0x16);
		noc_probe_mctlpx_enable(noc, PROBE_MCTLP1);
		pr_info("start_probe,please wailt  %d ms;cat noc_probe  view result\n",((1<<0x16))/400000);
	}else if(strcmp(param, "cfg") == 0)	{
		noc_probe_store_cfg(args, noc);
	}else if(strcmp(param, "start") == 0)	{
		noc_probe_store_start_or_end(args, noc, 1);
		pr_info("start_probe,please cat noc_probe \n");
	}else if(strcmp(param, "stop") == 0)	{
		noc_probe_store_start_or_end(args, noc, 0);
		pr_info("disable_probe \n");
	}	else {
		noc_probe_store_help();
	}
	kfree(buf_tmp);

    return count;
}
static DEVICE_ATTR(probe, 0644, noc_probe_show, noc_probe_store);

static void ____noc_qos_show(struct noc_qos_st *qos, char *prompt)
{
	char *modstr[]={"Fixed","Limiter","Bypass","Regulator"};
	printk("-----%s :\n", prompt);
	//printk("mode:%s(%lx) ",modstr[qos->Mode],qos->Mode);
	//printk("Bandwidth:%lx sat:%lx ext:%lx\n",modstr[qos->Mode],qos->Mode, qos->Bandwidth, qos->Saturation, qos->ExtControl);
	printk("mode:%s(%x) Priority:%x Bandwidth:%x*axi(%x) Saturation:%x(%x) extc:%x\n",\
								modstr[qos->Mode], qos->Mode, qos->Priority, \
								qos->Bandwidth/256, qos->Bandwidth,  \
								qos->Saturation *16, qos->Saturation, \
								qos->ExtControl);
	print_hex_dump(KERN_INFO, "", DUMP_PREFIX_OFFSET, 16, 4, (void*)qos , 0x1c, 0);
	return;
}

/*
analysis_probe   /analysis
qos--查看qos带宽限制；设置带宽；
register
*/

//读操作
static ssize_t noc_qos_show(struct device *dev, struct device_attribute *attr,
        char *buf)
{
	ssize_t ret=0;
	struct noc_plat *noc = dev_get_drvdata(dev);

	__ai_enable_power_clk();
	__vpu_enable_power_clk();
	printk("Mode: 0=Fixed, 1=Limiter, 2=Bypass, 3=Regulator(priority)\n");
	printk("Bandwidth register value = (Limit Bandwidth/axi clk)*256 \n");
	____noc_qos_show(&noc->noc_reg->ai_axi0_I_main_QosGenerator,  "ai_axi0");
	____noc_qos_show(&noc->noc_reg->ai_axi1_I_main_QosGenerator,  "ai_axi1");
	____noc_qos_show(&noc->noc_reg->cpu0_axi_I_main_QosGenerator, "cpu0   ");
	____noc_qos_show(&noc->noc_reg->cpu1_axi_I_main_QosGenerator, "cpu1   ");
	____noc_qos_show(&noc->noc_reg->display_axi_I_main_QosGenerator, "display");
	____noc_qos_show(&noc->noc_reg->g2p5d_axi_I_main_QosGenerator, "g2p5d");
	____noc_qos_show(&noc->noc_reg->isp_3dnr_I_main_QosGenerator, "isp 3dnr");
	____noc_qos_show(&noc->noc_reg->isp_dwe_I_main_QosGenerator, "isp dwe");
	____noc_qos_show(&noc->noc_reg->isp_hdr_I_main_QosGenerator, "isp hdr");
	____noc_qos_show(&noc->noc_reg->isp_mp_mcm_I_main_QosGenerator, "isp mp mcm");
	____noc_qos_show(&noc->noc_reg->isp_scal_I_main_QosGenerator, "isp scal ");
	____noc_qos_show(&noc->noc_reg->sec_axi_I_main_QosGenerator, "sec");
	____noc_qos_show(&noc->noc_reg->stor_axi_I_main_QosGenerator, "store ");
	____noc_qos_show(&noc->noc_reg->vpu_axi_I_main_QosGenerator, "vpu");
	return ret;//noc_probe_mctl_bandwidth_show(noc, buf, PAGE_SIZE);
}

//写操作
static ssize_t noc_qos_store(struct device *dev, struct device_attribute *attr,
         const char *buf, size_t count)
{
	printk("f=%s l=%d\n", __func__, __LINE__);
	#if 0

	char *param="";
	char *val="";
	char *args = NULL;
	char *buf_tmp = kzalloc(count+2,0);


	struct noc_plat *noc = dev_get_drvdata(dev);

	args =  buf_tmp;
	memcpy(args, buf, count);


	args = skip_spaces(args);
	args = next_arg(args, &param, &val);


	if(strcmp(param, "ddr_probe_all_cfg") == 0){
		pr_info("ddr_all\n");
		noc_probe_store_cfg_ddr_all(args,noc);
	}else if(strcmp(param, "cfg_add") == 0)	{
		noc_probe_store_cfg(args, noc);
	}else if(strcmp(param, "start_probe") == 0)	{
		noc_probe_store_start_or_end(args, noc, 1);
		pr_info("start_probe,please cat noc_probe \n");
	}else if(strcmp(param, "disable_probe") == 0)	{
		noc_probe_store_start_or_end(args, noc, 0);
		pr_info("disable_probe \n");
	}	else {
		noc_probe_store_help();
	}
	kfree(buf_tmp);
	#endif
    return count;
}



static DEVICE_ATTR(qos, 0644, noc_qos_show, noc_qos_store);

// //读操作
// static ssize_t rate_show(struct device *dev, struct device_attribute *attr,
//         char *buf)
// {
// 	ssize_t ret=0;
// 	printk("f=%s l=%d\n", __func__, __LINE__);

// 	//struct noc_plat *noc = dev_get_drvdata(dev);
// 	//noc->noc_reg->ax25mAXIm;
// 	// print_hex_dump(KERN_INFO,"ax25mAXIm",DUMP_PREFIX_OFFSET,16,1,(void*)&noc->noc_reg->ax25mAXIm, sizeof(noc->noc_reg->ax25mAXIm), 0);
// 	// print_hex_dump(KERN_INFO,"ax25pAXIm",DUMP_PREFIX_OFFSET,16,1,(void*)&noc->noc_reg->ax25pAXIm, sizeof(noc->noc_reg->ax25pAXIm), 0);
// 	return ret;//noc_probe_mctl_bandwidth_show(noc, buf, PAGE_SIZE);
// }

// //写操作
// static ssize_t rate_store(struct device *dev, struct device_attribute *attr,
//          const char *buf, size_t count)
// {
// 	printk("f=%s l=%d\n", __func__, __LINE__);
// 	return 0;
// }
// static DEVICE_ATTR(rate, 0644, rate_show, rate_store);

static int noc_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *res;
	struct noc_plat *noc=NULL;


	noc  = kzalloc(sizeof(struct noc_plat), GFP_KERNEL);
	if (!noc) {
		printk("mpc_probe: kzalloc failed \n");
		return -ENOMEM;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
        printk("noc_probe: get resource failed \n");
		ret=-ENODEV;
		goto error;
	}
	noc->noc_reg = (struct noc_reg_st *)devm_ioremap(&pdev->dev, res->start, resource_size(res));

	if (!noc->noc_reg){
		ret=-ENODEV;
		goto error;
	}
	//pr_info("noc->noc_reg=%lx  %lx %lx\n", (unsigned long)noc->noc_reg, res->start, resource_size(res));


	platform_set_drvdata(pdev, noc);
	ret = device_create_file(&pdev->dev, &dev_attr_probe);
	if (ret != 0) {
		dev_err(&pdev->dev,
		"Failed to create xxx sysfs files: %d\n", ret);
		goto  error;
	}
	ret = device_create_file(&pdev->dev, &dev_attr_qos);
	if(ret != 0){
		device_remove_file(&pdev->dev, &dev_attr_probe);
		goto  error;
	}

	return 0;

error:
	if(noc)
		kfree(noc);
	return ret;

}
static int noc_remove(struct platform_device *pdev)
{
	struct noc_plat *noc = platform_get_drvdata(pdev);
	if(noc->my_thread){
		kthread_stop(noc->my_thread);
		noc->my_thread = NULL;
	}

	device_remove_file(&pdev->dev, &dev_attr_probe);
	device_remove_file(&pdev->dev, &dev_attr_qos);


	if(noc)
		kfree(noc);
	return 0;
}


static const struct of_device_id k230_noc_ids[] = {
	{ .compatible = "k230-noc" },
	{}
};

static struct platform_driver k230_noc_driver = {
    .probe          = noc_probe,
    .remove         = noc_remove,
    .driver         = {
        .name           = "k230-noc",
        .of_match_table = of_match_ptr(k230_noc_ids),
    },
};

static struct resource k230_device_resources[] = {
    {
        .start = 0x91300000,  // 设备寄存器起始地址
        .end   = 0x9130ffff,  // 设备寄存器结束地址
        .flags = IORESOURCE_MEM,  // 资源类型为内存
    },
    // {
    //     .start = 42,  // 中断号
    //     .end   = 42,
    //     .flags = IORESOURCE_IRQ,  // 资源类型为中断
    // },
};

static void k230_noc_device_release(struct device *dev)
{
	// struct platform_device *pdev = to_platform_device(dev);

	// kfree(pdev);
	return;
}
/* 定义 Platform Device */
static struct platform_device k230_noc_device = {
    .name = "k230-noc",  // 设备名称，必须与驱动匹配
    .id = -1,  // 设备实例 ID，-1 表示只有一个实例
    .num_resources = ARRAY_SIZE(k230_device_resources),
    .resource = k230_device_resources,
	.dev.release = k230_noc_device_release,
};

static int __init  noc_module_init(void)
{
	int ret;
	//pr_info("size =%lx, %lx ,%lx\n", sizeof(struct noc_reg_st), sizeof(struct noc_qos_st), sizeof(struct probe_mctlpx_main_probe));
	platform_device_register(&k230_noc_device);
	ret = platform_driver_register(&k230_noc_driver);
	if(ret)
		platform_device_unregister(&k230_noc_device);
	return ret;
}

static void __exit noc_module_deinit(void)
{

	platform_driver_unregister(&k230_noc_driver);
	platform_device_unregister(&k230_noc_device);
}

module_init(noc_module_init);
module_exit(noc_module_deinit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("k230 noc probe and qos driver");
MODULE_AUTHOR("wangjianxin@canaan-creative.com");

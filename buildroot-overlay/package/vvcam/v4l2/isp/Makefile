# SPDX-License-Identifier: GPL-2.0

ccflags-y	+= -DVVCAM_ISP0_BASE=0x90000000
ccflags-y	+= -DVVCAM_ISP1_BASE=0x90000000
ccflags-y	+= -DVVCAM_ISP_REG_SIZE=0x10000
ccflags-y	+= -DVVCAM_ISP0_IRQ=129
ccflags-y	+= -DVVCAM_ISP0_MI_IRQ=127
ccflags-y	+= -DVVCAM_ISP0_FE_IRQ=128
ccflags-y	+= -DVVCAM_ISP1_IRQ=129
ccflags-y	+= -DVVCAM_ISP1_MI_IRQ=127
ccflags-y	+= -DVVCAM_ISP1_FE_IRQ=128
ccflags-y	+= -DVVCAM_PLATFORM_REGISTER
ccflags-y	+= -DVVCAM_ISP_DEV_MAX=1
ccflags-y	+= -DVVCAM_TPG0=0xDE308300
ccflags-y	+= -DVVCAM_TPG1=0xDE308300
ccflags-y	+= -DVVCAM_TPG_SIZE=0x4

ccflags-y	+= -I$(src) -I$(src)/../common -I$(src)/isp_ctrl
ccflags-y	+= -DISP_AE_V3 -I$(src)/isp_ctrl/ae/ae_v3
ccflags-y	+= -DISP_AWB_V3 -I$(src)/isp_ctrl/awb/awb_v3
ccflags-y	+= -DISP_AF_V3 -I$(src)/isp_ctrl/af/af_v3
ccflags-y	+= -DISP_BLS -I$(src)/isp_ctrl/bls/bls
ccflags-y	+= -DISP_CCM -I$(src)/isp_ctrl/ccm/ccm
ccflags-y	+= -DISP_DPCC -I$(src)/isp_ctrl/dpcc/dpcc
ccflags-y	+= -DISP_GE -I$(src)/isp_ctrl/ge/ge
ccflags-y	+= -DISP_LSC_V3 -I$(src)/isp_ctrl/lsc/lsc_v3
ccflags-y	+= -DISP_WB -I$(src)/isp_ctrl/wb/wb
ccflags-y	+= -DISP_DG -I$(src)/isp_ctrl/dg/dg
ccflags-y	+= -DISP_CPROC -I$(src)/isp_ctrl/cproc/cproc
ccflags-y	+= -DISP_EE_V2 -I$(src)/isp_ctrl/ee/ee_v2
ccflags-y	+= -DISP_GC_V2 -I$(src)/isp_ctrl/gc/gc_v2
ccflags-y	+= -DISP_AFM_V1 -I$(src)/isp_ctrl/afm/afm_v1
ccflags-y	+= -DISP_DMSC_V2 -I$(src)/isp_ctrl/dmsc/dmsc_v2
ccflags-y	+= -DISP_EXP_V2 -I$(src)/isp_ctrl/exp/exp_v2

vvcam_isp_subdev-objs			:= vvcam_isp_procfs.o
vvcam_isp_subdev-objs			+= vvcam_isp_event.o
vvcam_isp_subdev-objs			+= vvcam_isp_platform.o
vvcam_isp_subdev-objs			+= vvcam_isp_driver.o
vvcam_isp_subdev-objs			+= isp_ctrl/vvcam_isp_ctrl.o
vvcam_isp_subdev-objs			+= isp_ctrl/ee/ee_v2/vvcam_isp_ee.o
vvcam_isp_subdev-objs			+= isp_ctrl/lsc/lsc_v3/vvcam_isp_lsc.o
vvcam_isp_subdev-objs			+= isp_ctrl/dpcc/dpcc/vvcam_isp_dpcc.o
vvcam_isp_subdev-objs			+= isp_ctrl/exp/exp_v2/vvcam_isp_exp.o
vvcam_isp_subdev-objs			+= isp_ctrl/afm/afm_v1/vvcam_isp_afm.o
vvcam_isp_subdev-objs			+= isp_ctrl/ge/ge/vvcam_isp_ge.o
vvcam_isp_subdev-objs			+= isp_ctrl/cproc/cproc/vvcam_isp_cproc.o
vvcam_isp_subdev-objs			+= isp_ctrl/wb/wb/vvcam_isp_wb.o
vvcam_isp_subdev-objs			+= isp_ctrl/dmsc/dmsc_v2/vvcam_isp_dmsc.o
vvcam_isp_subdev-objs			+= isp_ctrl/gc/gc_v2/vvcam_isp_gc.o
vvcam_isp_subdev-objs			+= isp_ctrl/dg/dg/vvcam_isp_dg.o
vvcam_isp_subdev-objs			+= isp_ctrl/af/af_v3/vvcam_isp_af.o
vvcam_isp_subdev-objs			+= isp_ctrl/ae/ae_v3/vvcam_isp_ae.o
vvcam_isp_subdev-objs			+= isp_ctrl/ccm/ccm/vvcam_isp_ccm.o
vvcam_isp_subdev-objs			+= isp_ctrl/bls/bls/vvcam_isp_bls.o
vvcam_isp_subdev-objs			+= isp_ctrl/awb/awb_v3/vvcam_isp_awb.o
vvcam_isp_subdev-objs			+= isp_ctrl/vvcam_isp_ctrl.o

obj-m		+= vvcam_isp_subdev.o

# SPDX-License-Identifier: GPL-2.0

ccflags-y	+= -DVVCAM_ISP0_BASE=0x90000000 
ccflags-y	+= -DVVCAM_ISP_REG_SIZE=0x10000
ccflags-y	+= -DVVCAM_ISP0_IRQ=129
ccflags-y	+= -DVVCAM_ISP0_MI_IRQ=127
ccflags-y	+= -DVVCAM_ISP0_FE_IRQ=128
#ccflags-y	+= -DVVCAM_PLATFORM_REGISTER
ccflags-y	+= -DVVCAM_ISP_DEV_MAX=1
ccflags-y	+= -I$(srctree)/$(src)/v4l2/common

vvcam_mipi-objs					:= mipi/vvcam_mipi_driver.o mipi/k230_csi.o mipi/k230_vi.o

obj-$(CONFIG_VVCAM_MIPI)		+= vvcam_mipi.o

vvcam_isp-objs					:= isp/vvcam_isp_driver.o isp/vvcam_isp_hal.o isp/vvcam_isp_procfs.o isp/vvcam_event.o

obj-$(CONFIG_VVCAM_ISP)			+= vvcam_isp.o

vvcam_vb-objs					:= vb/vvcam_vb_platform.o vb/vvcam_cma.o vb/vvcam_vb_driver.o vb/vvcam_vb_procfs.o vb/vvcam_mmz.o

obj-$(CONFIG_VVCAM_VB)			+= vvcam_vb.o

obj-y							+= v4l2/isp/

vvcam_video-objs				:= v4l2/video/vvcam_video_platform.o v4l2/video/vvcam_video_register.o v4l2/video/vvcam_video_event.o v4l2/video/vvcam_video_m2m_register.o v4l2/video/vvcam_video_driver.o

obj-$(CONFIG_VVCAM_VIDEO)		+= vvcam_video.o

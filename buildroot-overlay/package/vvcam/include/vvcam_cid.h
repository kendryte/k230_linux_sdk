/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Userspace IDs for the vvcam video node. These match the driver:
 *   scene dir/mode: vvcam_isp_driver.c MY_CID_BASE + 0/1
 *   sensor target:  VVCAM_VIDEO_CID_SENSOR_* in vvcam_video_register.c
 *   AWB enable:     VVCAM_ISP_CID_AWB_ENABLE
 *   cproc manual:   VVCAM_ISP_CID_CPROC_ENABLE / MODE / MANU_*
 * libcamera's vvcam pipeline includes this header. It is copied next to
 * vvcam.cpp after the pipeline patch is applied.
 */
#pragma once

#include <linux/videodev2.h>

#define VVCAM_CID_SCENE_CALIB_DIR 0x00980900
#define VVCAM_CID_SCENE_MODE      (VVCAM_CID_SCENE_CALIB_DIR + 1)

#define VVCAM_CID_SENSOR_BASE   0x00980900
#define VVCAM_CID_SENSOR_ENABLE (VVCAM_CID_SENSOR_BASE + 2)
#define VVCAM_CID_SENSOR_WIDTH  (VVCAM_CID_SENSOR_BASE + 3)
#define VVCAM_CID_SENSOR_HEIGHT (VVCAM_CID_SENSOR_BASE + 4)
#define VVCAM_CID_SENSOR_FPS    (VVCAM_CID_SENSOR_BASE + 5)

#define VVCAM_CID_AWB_ENABLE (V4L2_CID_USER_BASE + 0x2100)

#define VVCAM_CID_CPROC_BASE            (V4L2_CID_USER_BASE + 0x2900)
#define VVCAM_CID_CPROC_ENABLE          (VVCAM_CID_CPROC_BASE + 0x0)
#define VVCAM_CID_CPROC_MODE            (VVCAM_CID_CPROC_BASE + 0x3)
#define VVCAM_CID_CPROC_MANU_CONTRAST   (VVCAM_CID_CPROC_BASE + 0xA)
#define VVCAM_CID_CPROC_MANU_BRIGHT     (VVCAM_CID_CPROC_BASE + 0xB)
#define VVCAM_CID_CPROC_MANU_SATURATION (VVCAM_CID_CPROC_BASE + 0xC)

#pragma once
#ifndef __VVCAM_SENSOR_TARGET_H__
#define __VVCAM_SENSOR_TARGET_H__

#include <stdint.h>
#include <linux/videodev2.h>

#define VVCAM_SENSOR_CID_BASE 0x00980900
#define VVCAM_SENSOR_CID_ENABLE (VVCAM_SENSOR_CID_BASE + 2)
#define VVCAM_SENSOR_CID_WIDTH  (VVCAM_SENSOR_CID_BASE + 3)
#define VVCAM_SENSOR_CID_HEIGHT (VVCAM_SENSOR_CID_BASE + 4)
#define VVCAM_SENSOR_CID_FPS    (VVCAM_SENSOR_CID_BASE + 5)

struct vvcam_isp_sensor_target {
    uint32_t enable;
    uint32_t width;
    uint32_t height;
    uint32_t fps;
};

static inline int vvcam_set_sensor_target(int video_fd,
    uint32_t width, uint32_t height, uint32_t fps)
{
    struct v4l2_ext_control ctrl[4];
    struct v4l2_ext_controls ctrls;

    ctrl[0].id = VVCAM_SENSOR_CID_ENABLE;
    ctrl[0].size = 0;
    ctrl[0].value = 1;
    ctrl[1].id = VVCAM_SENSOR_CID_WIDTH;
    ctrl[1].size = 0;
    ctrl[1].value = width;
    ctrl[2].id = VVCAM_SENSOR_CID_HEIGHT;
    ctrl[2].size = 0;
    ctrl[2].value = height;
    ctrl[3].id = VVCAM_SENSOR_CID_FPS;
    ctrl[3].size = 0;
    ctrl[3].value = fps;

    memset(&ctrls, 0, sizeof(ctrls));
    ctrls.ctrl_class = V4L2_CTRL_CLASS_USER;
    ctrls.count = 4;
    ctrls.controls = ctrl;

    return ioctl(video_fd, VIDIOC_S_EXT_CTRLS, &ctrls);
}

#endif

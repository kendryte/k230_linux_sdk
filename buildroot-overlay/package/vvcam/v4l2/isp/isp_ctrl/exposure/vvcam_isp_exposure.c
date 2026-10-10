// SPDX-License-Identifier: GPL-2.0 OR MIT
/*
 * Copyright (C) 2026 Canaan Inc.
 *
 * Standard V4L2 exposure controls. The kernel only registers and forwards
 * them; per-port state lives in isp_media_server.
 */

#include <media/v4l2-ioctl.h>
#include "vvcam_isp_driver.h"
#include "vvcam_isp_ctrl.h"
#include "vvcam_isp_exposure.h"
#include "vvcam_isp_event.h"

#define VVCAM_ISP_EXPOSURE_FLAGS \
    (V4L2_CTRL_FLAG_VOLATILE | V4L2_CTRL_FLAG_EXECUTE_ON_WRITE)

static bool vvcam_isp_exposure_port_streaming(struct vvcam_isp_dev *isp_dev)
{
    uint32_t base;
    uint32_t i;

    if (isp_dev->ctrl_pad >= VVCAM_ISP_PAD_NR)
        return false;

    base = (isp_dev->ctrl_pad / VVCAM_ISP_PORT_PAD_NR) * VVCAM_ISP_PORT_PAD_NR;
    for (i = 0; i < VVCAM_ISP_PORT_PAD_NR; i++) {
        if (isp_dev->pad_data[base + i].stream)
            return true;
    }

    return false;
}

static int vvcam_isp_exposure_s_ctrl(struct v4l2_ctrl *ctrl)
{
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id) {
    case V4L2_CID_EXPOSURE_AUTO:
        /* AE mode can only be changed while the port is not streaming. */
        if (vvcam_isp_exposure_port_streaming(isp_dev))
            return -EBUSY;
        return vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);

    case V4L2_CID_EXPOSURE:
    case V4L2_CID_ANALOGUE_GAIN:
        return vvcam_isp_s_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);

    default:
        dev_err(isp_dev->dev, "unknown v4l2 ctrl id %d\n", ctrl->id);
        return -EACCES;
    }
}

static int vvcam_isp_exposure_g_ctrl(struct v4l2_ctrl *ctrl)
{
    struct vvcam_isp_dev *isp_dev =
        container_of(ctrl->handler, struct vvcam_isp_dev, ctrl_handler);

    switch (ctrl->id) {
    case V4L2_CID_EXPOSURE_AUTO:
    case V4L2_CID_EXPOSURE:
    case V4L2_CID_ANALOGUE_GAIN:
        return vvcam_isp_g_ctrl_event(isp_dev, isp_dev->ctrl_pad, ctrl);

    default:
        dev_err(isp_dev->dev, "unknown v4l2 ctrl id %d\n", ctrl->id);
        return -EACCES;
    }
}

static const struct v4l2_ctrl_ops vvcam_isp_exposure_ctrl_ops = {
    .s_ctrl = vvcam_isp_exposure_s_ctrl,
    .g_volatile_ctrl = vvcam_isp_exposure_g_ctrl,
};

static const struct v4l2_ctrl_config vvcam_isp_exposure_ctrls[] = {
    {
        .ops  = &vvcam_isp_exposure_ctrl_ops,
        .id   = V4L2_CID_EXPOSURE_AUTO,
        .type = V4L2_CTRL_TYPE_MENU,
        .flags= VVCAM_ISP_EXPOSURE_FLAGS,
        .name = "Auto Exposure",
        .min  = V4L2_EXPOSURE_AUTO,
        .max  = V4L2_EXPOSURE_APERTURE_PRIORITY,
        .menu_skip_mask = BIT(V4L2_EXPOSURE_SHUTTER_PRIORITY),
        .def  = V4L2_EXPOSURE_AUTO,
    },
    {
        /*
         * Unit: 1 us. The V4L2 spec gives this control the old
         * EXPOSURE_ABSOLUTE unit of 100 us, which is coarser than one
         * sensor line (about 7 us on imx335). v4l2-ctl therefore shows a
         * value 100 times smaller than the microseconds written here.
         * The daemon clamps to the sensor mode range.
         */
        .ops  = &vvcam_isp_exposure_ctrl_ops,
        .id   = V4L2_CID_EXPOSURE,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= VVCAM_ISP_EXPOSURE_FLAGS,
        .name = "Exposure",
        .step = 1,
        .min  = 1,
        .max  = 1000000,
        .def  = 10000,
    },
    {
        /* Unit: 1/1000 x (1000 = 1.0x). The daemon clamps to the sensor range. */
        .ops  = &vvcam_isp_exposure_ctrl_ops,
        .id   = V4L2_CID_ANALOGUE_GAIN,
        .type = V4L2_CTRL_TYPE_INTEGER,
        .flags= VVCAM_ISP_EXPOSURE_FLAGS,
        .name = "Analogue Gain",
        .step = 1,
        .min  = 1000,
        .max  = 256000,
        .def  = 1000,
    },
};

int vvcam_isp_exposure_ctrl_count(void)
{
    return ARRAY_SIZE(vvcam_isp_exposure_ctrls);
}

int vvcam_isp_exposure_s_range(struct vvcam_isp_dev *isp_dev,
                               const struct vvcam_isp_exp_range *range)
{
    struct vvcam_isp_exp_limits limits = { .valid = false };
    unsigned long flags;
    uint32_t port;

    if (range->pad >= VVCAM_ISP_PAD_NR)
        return -EINVAL;

    if (range->valid) {
        if (range->exposure_min > range->exposure_max ||
            range->again_min > range->again_max)
            return -EINVAL;
        limits.valid = true;
        limits.exposure_min = range->exposure_min;
        limits.exposure_max = range->exposure_max;
        limits.again_min = range->again_min;
        limits.again_max = range->again_max;
    }

    port = range->pad / VVCAM_ISP_PORT_PAD_NR;
    spin_lock_irqsave(&isp_dev->exp_limits_lock, flags);
    isp_dev->exp_limits[port] = limits;
    spin_unlock_irqrestore(&isp_dev->exp_limits_lock, flags);

    dev_dbg(isp_dev->dev, "port %u exposure %d..%d us, again %d..%d%s\n",
        port, limits.exposure_min, limits.exposure_max,
        limits.again_min, limits.again_max, limits.valid ? "" : " (cleared)");

    return 0;
}

void vvcam_isp_exposure_fixup_range(struct vvcam_isp_dev *isp_dev,
                                    uint32_t pad, uint32_t id,
                                    s64 *pmin, s64 *pmax, s64 *pdef)
{
    struct vvcam_isp_exp_limits limits;
    unsigned long flags;
    s64 lo, hi, sensor_lo, sensor_hi;

    if (pad >= VVCAM_ISP_PAD_NR)
        return;
    if (id != V4L2_CID_EXPOSURE && id != V4L2_CID_ANALOGUE_GAIN)
        return;

    spin_lock_irqsave(&isp_dev->exp_limits_lock, flags);
    limits = isp_dev->exp_limits[pad / VVCAM_ISP_PORT_PAD_NR];
    spin_unlock_irqrestore(&isp_dev->exp_limits_lock, flags);

    if (!limits.valid)
        return;

    if (id == V4L2_CID_EXPOSURE) {
        lo = limits.exposure_min;
        hi = limits.exposure_max;
    } else {
        lo = limits.again_min;
        hi = limits.again_max;
    }

    /* Never report values S_CTRL would reject. */
    sensor_lo = lo;
    sensor_hi = hi;
    lo = max(lo, *pmin);
    hi = min(hi, *pmax);
    if (lo > hi) {
        dev_warn_once(isp_dev->dev,
            "sensor %s range %lld..%lld is outside control %lld..%lld, QUERYCTRL keeps the control range\n",
            id == V4L2_CID_EXPOSURE ? "exposure" : "analogue gain",
            sensor_lo, sensor_hi, *pmin, *pmax);
        return;
    }

    *pmin = lo;
    *pmax = hi;
    *pdef = clamp(*pdef, lo, hi);
}

int vvcam_isp_exposure_ctrl_create(struct vvcam_isp_dev *isp_dev)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(vvcam_isp_exposure_ctrls); i++) {
        v4l2_ctrl_new_custom(&isp_dev->ctrl_handler,
                            &vvcam_isp_exposure_ctrls[i], NULL);
        if (isp_dev->ctrl_handler.error) {
            dev_err(isp_dev->dev, "register isp exposure ctrl %s failed %d.\n",
                vvcam_isp_exposure_ctrls[i].name, isp_dev->ctrl_handler.error);
        }
    }

    return 0;
}

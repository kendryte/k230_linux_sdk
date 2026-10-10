/* SPDX-License-Identifier: GPL-2.0 OR MIT */
/*
 * Copyright (C) 2026 Canaan Inc.
 */

#ifndef __VVCAM_ISP_EXPOSURE_H__
#define __VVCAM_ISP_EXPOSURE_H__

#include "vvcam_isp_ctrl.h"

#ifdef __KERNEL__
struct vvcam_isp_exp_range;

int vvcam_isp_exposure_ctrl_count(void);
int vvcam_isp_exposure_ctrl_create(struct vvcam_isp_dev *isp_dev);
int vvcam_isp_exposure_s_range(struct vvcam_isp_dev *isp_dev,
                               const struct vvcam_isp_exp_range *range);
/* Narrows a QUERYCTRL result to the sensor range published for pad. */
void vvcam_isp_exposure_fixup_range(struct vvcam_isp_dev *isp_dev,
                                    uint32_t pad, uint32_t id,
                                    s64 *pmin, s64 *pmax, s64 *pdef);
#endif

#endif

/****************************************************************************
*
*    The MIT License (MIT)
*
*    Copyright (c) 2014 - 2024 Vivante Corporation
*
*    Permission is hereby granted, free of charge, to any person obtaining a
*    copy of this software and associated documentation files (the "Software"),
*    to deal in the Software without restriction, including without limitation
*    the rights to use, copy, modify, merge, publish, distribute, sublicense,
*    and/or sell copies of the Software, and to permit persons to whom the
*    Software is furnished to do so, subject to the following conditions:
*
*    The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
*
*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*    DEALINGS IN THE SOFTWARE.
*
*****************************************************************************
*
*    The GPL License (GPL)
*
*    Copyright (C) 2014 - 2024 Vivante Corporation
*
*    This program is free software; you can redistribute it and/or
*    modify it under the terms of the GNU General Public License
*    as published by the Free Software Foundation; either version 2
*    of the License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*****************************************************************************
*
*    Note: This software is released under dual MIT and GPL licenses. A
*    recipient may use this file under the terms of either the MIT license or
*    GPL License. If you wish to use only one license not the other, you can
*    indicate your decision by deleting one of the above license notices in your
*    version of this file.
*
*****************************************************************************/


#include <media/v4l2-ioctl.h>
#include "vvcam_isp_driver.h"
#include "vvcam_isp_ctrl.h"

#if defined(ISP_AE_V3) || defined(ISP_AE_V4) || defined(ISP_AE_V4_1)
#include "vvcam_isp_ae.h"
#endif

#if defined(ISP_AWB_V3) || defined(ISP_AWB_V4) || defined(ISP_AWB_V4_1)
#include "vvcam_isp_awb.h"
#endif

#if defined(ISP_GC_V1) || defined(ISP_GC_V2)
#include "vvcam_isp_gc.h"
#endif

#if defined(ISP_2DNR_V3) || defined(ISP_2DNR_V5_2) || defined(ISP_2DNR_V6)
#include "vvcam_isp_2dnr.h"
#endif

#if defined(ISP_EE_V1) || defined(ISP_EE_V2) || defined(ISP_EE_V3)
#include "vvcam_isp_ee.h"
#endif

#if defined(ISP_CPROC)
#include "vvcam_isp_cproc.h"
#endif

#if defined(ISP_WDR_V3) || defined(ISP_WDR_V4) || defined(ISP_WDR_V5) \
    || defined(ISP_WDR_V5_1) || defined(ISP_WDR_V5_2) || defined(ISP_WDR_V5_2_1) \
    || defined(ISP_WDR_V5_3)
#include "vvcam_isp_wdr.h"
#endif

#if defined(ISP_BLS)
#include "vvcam_isp_bls.h"
#endif

#if defined(ISP_CCM)
#include "vvcam_isp_ccm.h"
#endif

#if defined(ISP_DPCC)
#include "vvcam_isp_dpcc.h"
#endif

#if defined(ISP_GE)
#include "vvcam_isp_ge.h"
#endif

#if defined(ISP_LSC_V1) || defined(ISP_LSC_V2) || defined(ISP_LSC_V3) \
    || defined(ISP_LSC_V4)
#include "vvcam_isp_lsc.h"
#endif

#if defined(ISP_WB_V1) || defined(ISP_WB_V1_1)
#include "vvcam_isp_wb.h"
#endif

#if defined(ISP_3DNR_V1_2) || defined(ISP_3DNR_V2_1) || defined(ISP_3DNR_V2_4) \
    || defined(ISP_3DNR_V3) || defined(ISP_3DNR_V3_1) || defined(ISP_3DNR_V4)
#include "vvcam_isp_3dnr.h"
#endif

#if defined(ISP_AF_V3) || defined(ISP_AF_V4) || defined(ISP_AF_V4_1_1) \
    || defined(ISP_AF_V4_3)
#include "vvcam_isp_af.h"
#endif

#if defined(ISP_PDAF_V1)
#include "vvcam_isp_pdaf.h"
#endif

#if defined(ISP_AFM_V1) || defined(ISP_AFM_V1_1) || defined(ISP_AFM_V3)
#include "vvcam_isp_afm.h"
#endif

#if defined(ISP_DMSC_V1) || defined(ISP_DMSC_V2) || defined(ISP_DMSC_V3)
#include "vvcam_isp_dmsc.h"
#endif

#if defined(ISP_EXP_V2)
#include "vvcam_isp_exp.h"
#endif

#if defined(ISP_EXP_V3)
#include "vvcam_isp_exp_v3.h"
#endif

#if defined(ISP_GTM_V1)
#include "vvcam_isp_gtm.h"
#endif

#if defined(ISP_HDR_V1_3) || defined(ISP_HDR_V2) || defined(ISP_HDR_V2_1) \
    || defined(ISP_HDR_V2_1_2DOL) || defined(ISP_HDR_V3) \
    || defined(ISP_HDR_V3_2DOL) || defined(ISP_HDR_V3_1)
#include "vvcam_isp_hdr.h"
#endif

#if defined(ISP_RGBIR_V1) || defined(ISP_RGBIR_V1_1) || defined(ISP_RGBIR_V2) \
    || defined(ISP_RGBIR_V2_1)
#include "vvcam_isp_rgbir.h"
#endif

#if defined(ISP_DG_V1)
#include "vvcam_isp_dg.h"
#endif

#if defined(ISP_CMPD_20BIT) || defined(ISP_CMPD_24BIT)
#include "vvcam_isp_cpd.h"
#endif

#if defined(ISP_DPF_V1)
#include "vvcam_isp_dpf.h"
#endif

#if defined(ISP_CNR_V1) || defined(ISP_CNR_V2_1) || defined(ISP_CNR_V2_2)
#include "vvcam_isp_cnr.h"
#endif

#if defined(ISP_GWDR_V1)
#include "vvcam_isp_gwdr.h"
#endif

#if defined(ISP_YNR_V1)
#include "vvcam_isp_ynr.h"
#endif

#if defined(ISP_LUT3D_V1)
#include "vvcam_isp_lut3d.h"
#endif

int vvcam_isp_ctrl_init(struct vvcam_isp_dev *isp_dev)
{
    uint32_t ctrl_count = 0;

#if defined(ISP_AE_V3) || defined(ISP_AE_V4) || defined(ISP_AE_V4_1)
    ctrl_count += vvcam_isp_ae_ctrl_count();
#endif

#if defined(ISP_AWB_V3) || defined(ISP_AWB_V4) || defined(ISP_AWB_V4_1)
    ctrl_count += vvcam_isp_awb_ctrl_count();
#endif

#if defined(ISP_GC_V1) || defined(ISP_GC_V2)
    ctrl_count += vvcam_isp_gc_ctrl_count();
#endif

#if defined(ISP_2DNR_V3) || defined(ISP_2DNR_V5_2) || defined(ISP_2DNR_V6)
    ctrl_count += vvcam_isp_2dnr_ctrl_count();
#endif

#if defined(ISP_EE_V1) || defined(ISP_EE_V2) || defined(ISP_EE_V3)
    ctrl_count += vvcam_isp_ee_ctrl_count();
#endif

#if defined(ISP_CPROC)
    ctrl_count += vvcam_isp_cproc_ctrl_count();
#endif

#if defined(ISP_WDR_V3) || defined(ISP_WDR_V4) || defined(ISP_WDR_V5) \
    || defined(ISP_WDR_V5_1) || defined(ISP_WDR_V5_2) || defined(ISP_WDR_V5_2_1) \
    || defined(ISP_WDR_V5_3)
    ctrl_count += vvcam_isp_wdr_ctrl_count();
#endif

#if defined(ISP_BLS)
    ctrl_count += vvcam_isp_bls_ctrl_count();
#endif

#if defined(ISP_CCM)
    ctrl_count += vvcam_isp_ccm_ctrl_count();
#endif

#if defined(ISP_DPCC)
    ctrl_count += vvcam_isp_dpcc_ctrl_count();
#endif

#if defined(ISP_GE)
    ctrl_count += vvcam_isp_ge_ctrl_count();
#endif

#if defined(ISP_LSC_V1) || defined(ISP_LSC_V2) || defined(ISP_LSC_V3) \
    || defined(ISP_LSC_V4)
    ctrl_count += vvcam_isp_lsc_ctrl_count();
#endif

#if defined(ISP_WB_V1) || defined(ISP_WB_V1_1)
    ctrl_count += vvcam_isp_wb_ctrl_count();
#endif

#if defined(ISP_3DNR_V1_2) || defined(ISP_3DNR_V2_1) || defined(ISP_3DNR_V2_4) \
    || defined(ISP_3DNR_V3) || defined(ISP_3DNR_V3_1) || defined(ISP_3DNR_V4)
    ctrl_count += vvcam_isp_3dnr_ctrl_count();
#endif

#if defined(ISP_AF_V3) || defined(ISP_AF_V4) || defined(ISP_AF_V4_1_1) \
    || defined(ISP_AF_V4_3)
    ctrl_count += vvcam_isp_af_ctrl_count();
#endif

#if defined(ISP_PDAF_V1)
    ctrl_count += vvcam_isp_pdaf_ctrl_count();
#endif

#if defined(ISP_AFM_V1) || defined(ISP_AFM_V1_1) || defined(ISP_AFM_V3)
    ctrl_count += vvcam_isp_afm_ctrl_count();
#endif

#if defined(ISP_DMSC_V1) || defined(ISP_DMSC_V2) || defined(ISP_DMSC_V3)
    ctrl_count += vvcam_isp_dmsc_ctrl_count();
#endif

#if defined(ISP_EXP_V2)
    ctrl_count += vvcam_isp_exp_ctrl_count();
#endif

#if defined(ISP_EXP_V3)
    ctrl_count += vvcam_isp_exp_v3_ctrl_count();
#endif

#if defined(ISP_GTM_V1)
    ctrl_count += vvcam_isp_gtm_ctrl_count();
#endif

#if defined(ISP_HDR_V1_3) || defined(ISP_HDR_V2) || defined(ISP_HDR_V2_1) \
    || defined(ISP_HDR_V2_1_2DOL) || defined(ISP_HDR_V3) \
    || defined(ISP_HDR_V3_2DOL) || defined(ISP_HDR_V3_1)
    ctrl_count += vvcam_isp_hdr_ctrl_count();
#endif

#if defined(ISP_RGBIR_V1) || defined(ISP_RGBIR_V1_1) || defined(ISP_RGBIR_V2) \
    || defined(ISP_RGBIR_V2_1)
    ctrl_count += vvcam_isp_rgbir_ctrl_count();
#endif

#if defined(ISP_DG_V1)
    ctrl_count += vvcam_isp_dg_ctrl_count();
#endif

#if defined(ISP_CMPD_20BIT) || defined(ISP_CMPD_24BIT)
    ctrl_count += vvcam_isp_cpd_ctrl_count();
#endif

#if defined(ISP_DPF_V1)
    ctrl_count += vvcam_isp_dpf_ctrl_count();
#endif

#if defined(ISP_CNR_V1) || defined(ISP_CNR_V2_1) || defined(ISP_CNR_V2_2)
    ctrl_count += vvcam_isp_cnr_ctrl_count();
#endif

#if defined(ISP_GWDR_V1)
    ctrl_count += vvcam_isp_gwdr_ctrl_count();
#endif

#if defined(ISP_YNR_V1)
    ctrl_count += vvcam_isp_ynr_ctrl_count();
#endif

#if defined(ISP_LUT3D_V1)
    ctrl_count += vvcam_isp_lut3d_ctrl_count();
#endif

    v4l2_ctrl_handler_init(&isp_dev->ctrl_handler,  ctrl_count);

#if defined(ISP_AE_V3) || defined (ISP_AE_V4) || defined(ISP_AE_V4_1)
    vvcam_isp_ae_ctrl_create(isp_dev);
#endif

#if defined(ISP_AWB_V3) || defined(ISP_AWB_V4) || defined(ISP_AWB_V4_1)
   vvcam_isp_awb_ctrl_create(isp_dev);
#endif

#if defined(ISP_GC_V1) || defined(ISP_GC_V2)
   vvcam_isp_gc_ctrl_create(isp_dev);
#endif

#if defined(ISP_2DNR_V3) || defined(ISP_2DNR_V5_2) || defined(ISP_2DNR_V6)
   vvcam_isp_2dnr_ctrl_create(isp_dev);
#endif

#if defined(ISP_EE_V1) || defined(ISP_EE_V2) || defined(ISP_EE_V3)
   vvcam_isp_ee_ctrl_create(isp_dev);
#endif

#if defined(ISP_CPROC)
    vvcam_isp_cproc_ctrl_create(isp_dev);
#endif

#if defined(ISP_WDR_V3) || defined(ISP_WDR_V4) || defined(ISP_WDR_V5) \
    || defined(ISP_WDR_V5_1) || defined(ISP_WDR_V5_2) || defined(ISP_WDR_V5_2_1) \
    || defined(ISP_WDR_V5_3)
    vvcam_isp_wdr_ctrl_create(isp_dev);
#endif

#if defined(ISP_BLS)
    vvcam_isp_bls_ctrl_create(isp_dev);
#endif

#if defined(ISP_CCM)
    vvcam_isp_ccm_ctrl_create(isp_dev);
#endif

#if defined(ISP_DPCC)
    vvcam_isp_dpcc_ctrl_create(isp_dev);
#endif

#if defined(ISP_GE)
    vvcam_isp_ge_ctrl_create(isp_dev);
#endif

#if defined(ISP_LSC_V1) || defined(ISP_LSC_V2) || defined(ISP_LSC_V3) \
    || defined(ISP_LSC_V4)
    vvcam_isp_lsc_ctrl_create(isp_dev);
#endif

#if defined(ISP_WB_V1) || defined(ISP_WB_V1_1)
    vvcam_isp_wb_ctrl_create(isp_dev);
#endif

#if defined(ISP_3DNR_V1_2) || defined(ISP_3DNR_V2_1) || defined(ISP_3DNR_V2_4) \
    || defined(ISP_3DNR_V3) || defined(ISP_3DNR_V3_1) || defined(ISP_3DNR_V4)
    vvcam_isp_3dnr_ctrl_create(isp_dev);
#endif

#if defined(ISP_AF_V3) || defined(ISP_AF_V4) || defined(ISP_AF_V4_1_1) \
    || defined(ISP_AF_V4_3)
    vvcam_isp_af_ctrl_create(isp_dev);
#endif

#if defined(ISP_PDAF_V1)
    vvcam_isp_pdaf_ctrl_create(isp_dev);
#endif

#if defined(ISP_AFM_V1) || defined(ISP_AFM_V1_1) || defined(ISP_AFM_V3)
    vvcam_isp_afm_ctrl_create(isp_dev);
#endif

#if defined(ISP_DMSC_V1) || defined(ISP_DMSC_V2) || defined(ISP_DMSC_V3)
    vvcam_isp_dmsc_ctrl_create(isp_dev);
#endif

#if defined(ISP_EXP_V2)
    vvcam_isp_exp_ctrl_create(isp_dev);
#endif

#if defined(ISP_EXP_V3)
    vvcam_isp_exp_v3_ctrl_create(isp_dev);
#endif

#if defined(ISP_GTM_V1)
     vvcam_isp_gtm_ctrl_create(isp_dev);
#endif

#if defined(ISP_HDR_V1_3) || defined(ISP_HDR_V2) || defined(ISP_HDR_V2_1) \
    || defined(ISP_HDR_V2_1_2DOL) || defined(ISP_HDR_V3) \
    || defined(ISP_HDR_V3_2DOL) || defined(ISP_HDR_V3_1)
    vvcam_isp_hdr_ctrl_create(isp_dev);
#endif

#if defined(ISP_RGBIR_V1) || defined(ISP_RGBIR_V1_1) || defined(ISP_RGBIR_V2) \
    || defined(ISP_RGBIR_V2_1)
    vvcam_isp_rgbir_ctrl_create(isp_dev);
#endif

#if defined(ISP_DG_V1)
    vvcam_isp_dg_ctrl_create(isp_dev);
#endif

#if defined(ISP_CMPD_20BIT) || defined(ISP_CMPD_24BIT)
    vvcam_isp_cpd_ctrl_create(isp_dev);
#endif

#if defined(ISP_DPF_V1)
    vvcam_isp_dpf_ctrl_create(isp_dev);
#endif

#if defined(ISP_CNR_V1) || defined(ISP_CNR_V2_1) || defined(ISP_CNR_V2_2)
    vvcam_isp_cnr_ctrl_create(isp_dev);
#endif

#if defined(ISP_GWDR_V1)
    vvcam_isp_gwdr_ctrl_create(isp_dev);
#endif

#if defined(ISP_YNR_V1)
    vvcam_isp_ynr_ctrl_create(isp_dev);
#endif

#if defined(ISP_LUT3D_V1)
    vvcam_isp_lut3d_ctrl_create(isp_dev);
#endif

    isp_dev->sd.ctrl_handler = &isp_dev->ctrl_handler;

    return 0;
}

int vvcam_isp_ctrl_destroy(struct vvcam_isp_dev *isp_dev)
{
    v4l2_ctrl_handler_free(&isp_dev->ctrl_handler);

    return 0;
}


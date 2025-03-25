/* Copyright (c) 2024, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
// dma_buf_manager.h
#ifndef DMA_BUF_MANAGER_H
#define DMA_BUF_MANAGER_H

#include "utils.h"
#include "ai_base.h"

class DMABufManager
{
public:
    /**
     * @brief DMABufManager,DMA缓冲区管理器
     * @param isp_shape   isp输入大小（chw）
     * @param dmabufs     DMA缓冲区,将视频帧数据直接传输到硬件设备ai_2d
     * @return None
     */
    DMABufManager(FrameCHWSize isp_shape,std::vector<std::tuple<int, void*>> dmabufs);
    
    /**
     * @brief DMABufManager析构函数
     * @return None
     */
    ~DMABufManager();
    
    /**
     * @brief DMABufManager,
     * @param isp_shape   isp输入大小（chw）
     * @param dmabufs     DMA缓冲区,将视频帧数据直接传输到硬件设备ai_2d
     * @return None
     */
     
    /**
     * @brief 获取index对应的DMABuf
     * @param index       DMABuf索引
     * @return None
     */
    runtime_tensor& get_buf_for_index(unsigned index);
private:
    int ai2d_fd = -1;                    // 用于管理/dev/k230-ai2d的打开是释放
    std::vector<runtime_tensor> dmabufs; //DMA缓冲区,将视频帧数据直接传输到硬件设备ai_2d
};

#endif
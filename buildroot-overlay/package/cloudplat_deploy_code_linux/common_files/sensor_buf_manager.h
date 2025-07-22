/* Copyright (c) 2025, Canaan Bright Sight Co., Ltd
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
#ifndef SENSOR_BUF_MANAGER_H
#define SENSOR_BUF_MANAGER_H

#include "ai_utils.h"
#include "ai_base.h"

/**
 * @class SensorBufManager
 * @brief DMA 缓冲区管理器，用于管理视频帧数据缓冲区并与 AI2D 硬件设备交互
 *
 * 负责管理 ISP 输入尺寸的缓冲区，将视频帧数据通过 DMA 传输到 AI2D 进行后续处理。
 */
class SensorBufManager
{
public:
    /**
     * @brief 构造函数，初始化 DMA 缓冲区管理器
     * 
     * @param isp_shape ISP 输入图像尺寸（通道、高度、宽度）
     * @param sensor_bufs DMA 缓冲区列表，元素为 tuple<int, void*>，代表缓冲区大小和指针
     */
    SensorBufManager(FrameCHWSize isp_shape, std::vector<std::tuple<int, void*>> sensor_bufs);
    
    /**
     * @brief 析构函数，释放相关资源
     */
    ~SensorBufManager();
    
    /**
     * @brief 获取指定索引对应的缓冲区张量引用
     * 
     * @param index 缓冲区索引
     * @return 对应的 runtime_tensor 引用，用于后续操作
     */
    runtime_tensor& get_buf_for_index(unsigned index);

private:
    std::vector<std::tuple<int, void*>> sensor_bufs;  ///< DMA 缓冲区列表，存储大小与缓冲区指针
    runtime_tensor ai2d_in_tensor;                     ///< 用于与 AI2D 硬件交互的输入张量
    size_t isp_size;                                   ///< ISP 输入图像数据总大小（字节数）
};

#endif

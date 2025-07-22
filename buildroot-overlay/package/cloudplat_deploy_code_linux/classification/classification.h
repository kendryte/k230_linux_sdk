/* Copyright (c) 2025, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _CLASSIFICATION_H
#define _CLASSIFICATION_H

#include "ai_utils.h"
#include "ai_base.h"
#include "parse_args.h"
#include "text_paint.h"

/**
 * @brief 分类结果结构体
 */
typedef struct cls_res
{
    float score;      ///< 分类得分
    std::string label;///< 分类标签名
} cls_res;

/**
 * @brief 图像分类任务类
 *
 * 封装了分类模型的加载、图像预处理、模型推理、后处理和结果可视化的完整流程。
 */
class Classification : public AIBase
{
public:
    /**
     * @brief Classification 构造函数
     *
     * 加载 Kmodel 模型并初始化推理输入输出、分类阈值等参数。
     *
     * @param args         解析自 config.json 的参数（包含模型路径、阈值、标签等）
     * @param isp_shape    原始图像输入尺寸（CHW 格式）
     * @param debug_mode   调试等级：0=关闭，1=显示耗时，2=显示所有信息
     */
    Classification(config_args args, FrameCHWSize isp_shape, const int debug_mode);

    /**
     * @brief Classification 析构函数
     */
    ~Classification();

    /**
     * @brief 图像预处理
     *
     * 将原始图像转换为模型输入格式，完成缩放、归一化等步骤。
     *
     * @param input_tensor 模型输入 Tensor 引用
     */
    void pre_process(runtime_tensor &input_tensor);

    /**
     * @brief 模型推理
     *
     * 使用加载的 Kmodel 对预处理后的图像进行前向推理。
     */
    void inference();

    /**
     * @brief 后处理分类结果
     *
     * 解析推理输出结果，获取分类得分最高的标签。
     *
     * @param result 存储最终分类结果（分数和标签）
     */
    void post_process(cls_res &result);

    /**
     * @brief 在图像上绘制分类结果
     *
     * @param draw_frame 带绘制的图像
     * @param result     分类结果（标签+分数）
     * @param writepen   文本绘制对象
     */
    static void draw_result(cv::Mat &draw_frame, cls_res &result, TextRenderer &writepen);

private:
    /**
     * @brief 近似计算 exp(x)
     *
     * @param x 输入值
     * @return  近似的 e^x 值
     */
    float fast_exp(float x);

    /**
     * @brief 计算 sigmoid(x)
     *
     * @param x 输入值
     * @return  sigmoid(x) 的值
     */
    float sigmoid(float x);

    std::unique_ptr<ai2d_builder> ai2d_builder_; ///< ai2d 构建器，用于图像预处理
    runtime_tensor ai2d_out_tensor_;             ///< ai2d 输出 tensor
    FrameCHWSize image_size_;                    ///< 原始图像尺寸（来自摄像头）
    FrameCHWSize input_size_;                    ///< 模型输入尺寸

    float cls_thresh;                            ///< 分类阈值，用于过滤低置信度结果
    std::vector<std::string> labels;             ///< 类别标签列表
    int num_class;                               ///< 类别总数
};

#endif // _CLASSIFICATION_H

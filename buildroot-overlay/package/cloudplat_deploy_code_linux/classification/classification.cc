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

#include "classification.h"
#include "parse_args.h"

/**
 * @brief Classification 构造函数
 *
 * 初始化模型、设置图像尺寸、预处理构建器等。
 *
 * @param args         配置参数（包含 kmodel 路径、分类标签、阈值等）
 * @param isp_shape    摄像头原始图像尺寸（CHW 格式）
 * @param debug_mode   调试等级
 */
Classification::Classification(config_args args, FrameCHWSize isp_shape, const int debug_mode)
    : AIBase(args.kmodel_path.c_str(), "Classification", debug_mode)
{
    cls_thresh = args.obj_thresh;
    labels = args.labels;
    num_class = labels.size();
    image_size_ = {isp_shape.channel, isp_shape.height, isp_shape.width};
    input_size_ = {input_shapes_[0][1], input_shapes_[0][2], input_shapes_[0][3]};
    ai2d_out_tensor_ = this->get_input_tensor(0);
    Utils::resize_set(image_size_, input_size_, ai2d_builder_);
}

/**
 * @brief Classification 析构函数
 */
Classification::~Classification()
{
}

/**
 * @brief 图像预处理
 *
 * 将原始图像通过 ai2d 预处理为模型输入格式。
 *
 * @param input_tensor 原始图像对应的 Tensor
 */
void Classification::pre_process(runtime_tensor &input_tensor)
{
    ScopedTiming st(model_name_ + " pre_process", debug_mode_);
    ai2d_builder_->invoke(input_tensor, ai2d_out_tensor_).expect("error occurred in ai2d running");
}

/**
 * @brief 执行模型推理
 */
void Classification::inference()
{
    this->run();         // 调用基础类 AIBase 中的 run() 执行推理
    this->get_output();  // 获取推理输出
}

/**
 * @brief 快速近似计算 exp(x)
 *
 * 使用位运算方法提高性能。
 *
 * @param x 输入值
 * @return e^x 的近似值
 */
float Classification::fast_exp(float x)
{
    union {
        uint32_t i;
        float f;
    } v{};
    v.i = (1 << 23) * (1.4426950409 * x + 126.93490512f);
    return v.f;
}

/**
 * @brief sigmoid 函数
 *
 * @param x 输入值
 * @return sigmoid(x)
 */
float Classification::sigmoid(float x)
{
    return 1.0f / (1.0f + fast_exp(-x));
}

/**
 * @brief 后处理分类输出结果
 *
 * 对模型输出进行 softmax 或 sigmoid，并根据阈值输出最高置信类别。
 *
 * @param result 保存最终分类结果的结构体（label 和 score）
 */
void Classification::post_process(cls_res &result)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    float* output = p_outputs_[0];

    if (num_class > 2)
    {
        // 多分类 softmax 后处理
        float sum = 0.0f;
        for (int i = 0; i < num_class; ++i)
        {
            sum += exp(output[i]);
        }

        for (int i = 0; i < num_class; ++i)
        {
            output[i] = exp(output[i]) / sum;
        }

        int max_index = std::max_element(output, output + num_class) - output;

        if (output[max_index] >= cls_thresh)
        {
            result.label = labels[max_index];
            result.score = output[max_index];
        }
        else
        {
            result.label = ""; // 低于阈值可设为空
            result.score = 0.0f;
        }
    }
    else
    {
        // 二分类 sigmoid 后处理
        float pre = sigmoid(output[0]);
        if (pre > cls_thresh)
        {
            result.label = labels[1];  // 通常为“正类”
            result.score = pre;
        }
        else
        {
            result.label = labels[0];  // 通常为“负类”
            result.score = 1 - pre;
        }
    }
}

/**
 * @brief 在图像上绘制分类结果文本
 *
 * @param draw_frame 带绘制的图像帧（3 通道或 4 通道）
 * @param result     分类结果（标签和分数）
 * @param writepen   文本绘制工具（TextRenderer）
 */
void Classification::draw_result(cv::Mat &draw_frame, cls_res &result, TextRenderer &writepen)
{
    std::string text = "";
    
    if(result.label!=""){
        text = "类别: " + result.label + ", 分数: " + std::to_string(result.score).substr(0, 5);
    }
     

    if (draw_frame.channels() == 3)
    {
        writepen.putText(draw_frame, text, cv::Point(20, 40), cv::Scalar(0, 255, 0));
        std::cout << "==========分类结果===========" << std::endl;
        std::cout << text << std::endl;
        std::cout << "=============================" << std::endl;
    }
    else if (draw_frame.channels() == 4)
    {
        writepen.putText(draw_frame, text, cv::Point(20, 40), cv::Scalar(0, 255, 0, 255));
    }
}

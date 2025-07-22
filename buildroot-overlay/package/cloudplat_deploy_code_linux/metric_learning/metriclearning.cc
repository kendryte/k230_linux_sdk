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

#include "metriclearning.h"

/**
 * @brief 构造函数，初始化 Metric Learning 模型，包括加载模型、设置输入输出形状、构建 ai2d 预处理器等。
 * 
 * @param args          配置参数，包括模型路径、标签等
 * @param isp_shape     摄像头图像的尺寸（通道数、高度、宽度）
 * @param debug_mode    是否启用调试模式
 */
Metriclearning::Metriclearning(config_args args, FrameCHWSize isp_shape, const int debug_mode)
: AIBase(args.kmodel_path.c_str(), "Metriclearning", debug_mode)
{
    labels = args.labels;
    num_class = labels.size();

    // 原始图像尺寸（C, H, W）
    image_size_ = {isp_shape.channel, isp_shape.height, isp_shape.width};

    // 模型输入尺寸（C, H, W）
    input_size_ = {input_shapes_[0][1], input_shapes_[0][2], input_shapes_[0][3]};

    // 获取输入张量并初始化 ai2d 预处理器
    ai2d_out_tensor_ = this->get_input_tensor(0);
    Utils::resize_set(image_size_, input_size_, ai2d_builder_);
}

/**
 * @brief 析构函数
 */
Metriclearning::~Metriclearning()
{
}

/**
 * @brief 图像预处理函数，将原始图像 resize 到模型输入尺寸，并填充到 ai2d 输入张量中。
 * 
 * @param input_tensor 输入张量（原始图像数据）
 */
void Metriclearning::pre_process(runtime_tensor &input_tensor)
{
    ScopedTiming st(model_name_ + " pre_process", debug_mode_);
    ai2d_builder_->invoke(input_tensor, ai2d_out_tensor_)
        .expect("error occurred in ai2d running");
}

/**
 * @brief 执行模型推理，包括运行和获取输出。
 */
void Metriclearning::inference()
{
    this->run();         // 执行推理
    this->get_output();  // 获取推理结果
}

/**
 * @brief 推理后处理，提取特征向量并保存为二进制文件。
 *        每次调用后将输出保存为 result_idx.bin 文件。
 */
void Metriclearning::post_process()
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);

    // 获取输出特征维度大小
    embedding_size = output_shapes_[0][1];

    // 生成保存文件名
    std::string file_name = "result_" + std::to_string(idx) + ".bin";

    // 保存特征向量到文件
    Utils::dump_binary_file(file_name.c_str(),
                            reinterpret_cast<char *>(p_outputs_[0]),
                            output_shapes_[0][1] * sizeof(float));
    idx++;
}


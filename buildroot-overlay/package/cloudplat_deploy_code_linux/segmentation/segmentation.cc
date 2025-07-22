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

#include "segmentation.h"

/**
 * @brief Segmentation 类构造函数
 * @param args        配置参数，包括模型路径、类别标签、阈值等
 * @param isp_shape   原始图像的通道、高度、宽度信息
 * @param debug_mode  调试模式标志，控制日志输出等
 */
Segmentation::Segmentation(config_args args, FrameCHWSize isp_shape, const int debug_mode)
: AIBase(args.kmodel_path.c_str(), "Segmentation", debug_mode)
{
    // 读取配置中的分割阈值和类别标签
    thresh = args.obj_thresh;
    labels = args.labels;
    num_class = labels.size();

    // 记录输入图像尺寸（CHW格式）
    image_size_ = {isp_shape.channel, isp_shape.height, isp_shape.width};

    // 记录模型输入尺寸（CHW格式）
    input_size_ = {input_shapes_[0][1], input_shapes_[0][2], input_shapes_[0][3]};

    // 获取模型的输入 tensor，用于后续拷贝预处理结果
    ai2d_out_tensor_ = this->get_input_tensor(0);

    // 设置 ai2d 预处理器，匹配输入图像与模型尺寸
    Utils::resize_set(image_size_, input_size_, ai2d_builder_);
}

/**
 * @brief Segmentation 类析构函数
 */
Segmentation::~Segmentation()
{
}

/**
 * @brief 图像预处理函数
 * @param input_tensor 原始图像数据张量（CHW 格式）
 *
 * 功能：将原始图像 resize、padding、normalize 等处理，
 *       输出至模型输入张量 ai2d_out_tensor_
 */
void Segmentation::pre_process(runtime_tensor &input_tensor)
{
    ScopedTiming st(model_name_ + " pre_process", debug_mode_);

    // 调用 ai2d 预处理，处理结果写入 ai2d_out_tensor_
    ai2d_builder_->invoke(input_tensor, ai2d_out_tensor_).expect("error occurred in ai2d running");
}

/**
 * @brief 模型推理函数
 *
 * 功能：调用推理运行函数和结果读取函数
 */
void Segmentation::inference()
{
    this->run();         // 执行模型推理
    this->get_output();  // 获取推理输出
}

/**
 * @brief 后处理函数，将模型输出转换为彩色掩膜图像并缩放至原图尺寸
 * @param draw_frame 输出图像（彩色 BGRA 掩膜），大小与输入图一致
 */
void Segmentation::post_process(cv::Mat &draw_frame)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);

    // 获取输出数据指针（形状为 [1, C, H, W]，按像素展开）
    float* output = p_outputs_[0];
    int height = input_shapes_[0][2];  // 输出图高
    int width = input_shapes_[0][3];   // 输出图宽

    // 创建一个空的 BGRA 彩色图像，大小与模型输出一致
    cv::Mat images_pred_color = cv::Mat::zeros(height, width, CV_8UC4);

    // 遍历每个像素点，执行 softmax 计算，并映射为颜色
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int loc = num_class * (x + y * width);  // 当前像素起始位置

            // softmax 数值稳定性处理（先减去最大值）
            float max_val = output[loc];
            for (int c = 0; c < num_class; ++c)
                max_val = std::max(max_val, output[loc + c]);

            // softmax 分数计算
            float sum = 0.0f;
            std::vector<float> scores(num_class);
            for (int c = 0; c < num_class; ++c)
            {
                scores[c] = std::exp(output[loc + c] - max_val);
                sum += scores[c];
            }
            for (int c = 0; c < num_class; ++c)
                scores[c] /= sum;

            // 找出最大概率类别
            int max_idx = 0;
            float max_score = scores[0];
            for (int c = 0; c < num_class; ++c)
            {
                if (scores[c] > max_score)
                {
                    max_score = scores[c];
                    max_idx = c;
                }
            }

            // 设置对应的 BGRA 颜色（简单的调色方案，可自定义）
            cv::Vec4b& color = images_pred_color.at<cv::Vec4b>(y, x);
            if (max_idx != 0) // 类别 0 为背景，跳过上色
            {
                color[3] = std::max(255 - max_idx * 60, 0);                      // Blue
                color[2] = std::min(max_idx * 80, 255);                          // Green
                color[1] = std::max(255 - (num_class - max_idx) * 100, 0);       // Red
                color[0] = 127;                                                  // Alpha
            }
        }
    }

    // 将分割彩图缩放至与原图 draw_frame 一致大小
    cv::resize(images_pred_color, draw_frame, cv::Size(draw_frame.cols, draw_frame.rows));
}

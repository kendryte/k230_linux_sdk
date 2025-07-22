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

#include "multilabel_cls.h"

/**
 * @brief 构造函数，初始化多标签分类模型，包括模型路径、标签、输入输出尺寸以及 ai2d 预处理构建。
 * 
 * @param args         模型配置参数，包括模型路径、标签、阈值等
 * @param isp_shape    原始图像尺寸（通道，高度，宽度）
 * @param debug_mode   是否启用调试模式
 */
MultilabelCls::MultilabelCls(config_args args, FrameCHWSize isp_shape, const int debug_mode)
: AIBase(args.kmodel_path.c_str(), "MultilabelCls", debug_mode)
{
    cls_thresh = args.obj_thresh;
    labels = args.labels;
    num_class = labels.size();

    // 原始图像尺寸
    image_size_ = {isp_shape.channel, isp_shape.height, isp_shape.width};

    // 模型输入张量尺寸
    input_size_ = {input_shapes_[0][1], input_shapes_[0][2], input_shapes_[0][3]};

    // 获取推理输入张量
    ai2d_out_tensor_ = this->get_input_tensor(0);

    // 构建 ai2d 预处理器
    Utils::resize_set(image_size_, input_size_, ai2d_builder_);
}

/**
 * @brief 析构函数，释放资源
 */
MultilabelCls::~MultilabelCls()
{
}

/**
 * @brief 图像预处理函数，将输入图像张量 resize 并拷贝到推理输入张量中
 * 
 * @param input_tensor 输入图像数据张量（摄像头采集或其他来源）
 */
void MultilabelCls::pre_process(runtime_tensor &input_tensor)
{
    ScopedTiming st(model_name_ + " pre_process", debug_mode_);
    ai2d_builder_->invoke(input_tensor, ai2d_out_tensor_)
        .expect("error occurred in ai2d running");
}

/**
 * @brief 执行模型推理，包含运行模型和获取输出结果
 */
void MultilabelCls::inference()
{
    this->run();
    this->get_output();
}

/**
 * @brief 快速指数计算函数，用于替代 std::exp，加速 sigmoid 函数
 * 
 * @param x 指数函数输入值
 * @return float 估算的 e^x
 */
float MultilabelCls::fast_exp(float x)
{
    union {
        uint32_t i;
        float f;
    } v{};
    v.i = (1 << 23) * (1.4426950409f * x + 126.93490512f);
    return v.f;
}

/**
 * @brief Sigmoid 激活函数
 * 
 * @param x 输入值
 * @return float sigmoid(x) 的结果
 */
float MultilabelCls::sigmoid(float x)
{
    return 1.0f / (1.0f + fast_exp(-x));
}

/**
 * @brief 后处理函数，将模型输出进行 sigmoid 激活并与阈值比较生成多标签结果
 * 
 * @param results 存储多标签分类结果的输出向量
 */
void MultilabelCls::post_process(std::vector<multi_lable_res> &results)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);

    float* output = p_outputs_[0];
    multi_lable_res result;

    for (int j = 0; j < num_class; j++)
    {
        // Sigmoid 激活
        output[j] = sigmoid(output[j]);

        // 保存分数
        result.score_vec.push_back(output[j]);

        // 根据阈值判断是否存在该类
        if (output[j] > cls_thresh)
            result.id_vec.push_back(1);
        else
            result.id_vec.push_back(0);
    }

    result.labels = labels;
    results.push_back(result);
}

/**
 * @brief 绘制多标签分类结果到图像上
 * 
 * @param draw_frame 要绘制的图像（cv::Mat）
 * @param results    多标签分类结果
 * @param writepen   用于绘制文本的字体渲染器
 */
void MultilabelCls::draw_result(cv::Mat &draw_frame, std::vector<multi_lable_res> &results, TextRenderer &writepen)
{
    for (int i = 0; i < results.size(); i++)
    {
        std::string text;
        int point_x = 15;
        int point_y = 0;
        cv::Scalar color;

        for (int j = 0; j < results[i].labels.size(); j++)
        {
            if (results[i].id_vec[j] == 1)
            {
                // 存在该标签
                text = "存在类别: " + results[i].labels[j] + ", 分数: " +
                       std::to_string(results[i].score_vec[j]).substr(0, 5);
            }
            else
            {
                // 不存在该标签
                text = "不存在类别: " + results[i].labels[j];
            }

            // 绘制文本（支持 3 通道和 4 通道图像）
            if (draw_frame.channels() == 3)
            {
                writepen.putText(draw_frame, text, cv::Point(20, 40+j*30), cv::Scalar(0, 255, 0));
            }
            else if (draw_frame.channels() == 4)
            {
                writepen.putText(draw_frame, text, cv::Point(20, 40+j*30), cv::Scalar(0, 255, 0, 255));
            }
        }
    }
}

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

#include "ocr_reco.h"

/**
 * @brief OCRReco 构造函数，用于初始化 OCR 字符识别模型
 * @param args         模型配置参数，包括字典路径、输出类别数、是否固定长度等
 * @param isp_shape    输入图像原始尺寸（CHW 格式）
 * @param debug_mode   调试模式开关
 */
OCRReco::OCRReco(config_args args, FrameCHWSize isp_shape, const int debug_mode)
: AIBase(args.kmodel_path.c_str(), "OCRReco", debug_mode)
{
    // 设置字典大小和是否固定输出长度
    dict_size = args.dict_num;
    flag = args.fixed_length;

    // 从字典文件中读取字符映射（每行一个字符）
    std::ifstream dict_string(DICT_STRING);
    while (!dict_string.eof())
    {
        std::string line;
        while (std::getline(dict_string, line))
        {
            txt_string.push_back(line);
        }
    }

    // 获取模型输入尺寸（HW）
    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];

    // 保存图像输入尺寸信息
    image_size_ = {isp_shape.channel, isp_shape.height, isp_shape.width};
    input_size_ = {input_shapes_[0][1], input_shapes_[0][2], input_shapes_[0][3]};

    // 获取模型输入 tensor
    ai2d_out_tensor_ = this->get_input_tensor(0);

    // 设置图像预处理器（按一边等比 padding resize，背景色填充为 114）
    Utils::padding_resize_one_side_set(image_size_, input_size_, ai2d_builder_, cv::Scalar(114, 114, 114));
}

/**
 * @brief 析构函数
 */
OCRReco::~OCRReco()
{
}

/**
 * @brief 图像预处理函数
 * @param input_tensor 输入图像张量（CHW 格式）
 *
 * 功能：对输入图像执行 resize、normalize 等预处理，并写入模型输入 tensor
 */
void OCRReco::pre_process(runtime_tensor &input_tensor)
{
    ScopedTiming st(model_name_ + " pre_process", debug_mode_);
    ai2d_builder_->invoke(input_tensor, ai2d_out_tensor_).expect("error occurred in ai2d running");
}

/**
 * @brief 执行模型推理
 */
void OCRReco::inference()
{
    this->run();         // 执行前向推理
    this->get_output();  // 获取推理输出
}

/**
 * @brief 模型后处理函数，解析输出 logits 并转换为文字字符串
 */
void OCRReco::post_process()
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);

    std::string results = "";

    // 获取模型输出指针（形状为 [batch, seq_len, dict_size]）
    float* output = p_outputs_[0];

    // 计算序列长度（通常宽度/4 为 CRNN 推理序列长度）
    int size = input_shapes_[0][3] / 4;

    // 保存每个位置预测的字符索引
    std::vector<int> result;

    // 遍历每个时间步，取最大概率的类别索引（argmax）
    for (int i = 0; i < size; i++)
    {
        float maxs = -10.f;
        int index = -1;

        for (int j = 0; j < dict_size; j++)
        {
            if (maxs < output[i * dict_size + j])
            {
                index = j;
                maxs = output[i * dict_size + j];
            }
        }

        result.push_back(index);
    }

    // 解码字符串，去除重复字符与 blank 类别（通常 blank 是索引 0 或 dict_size - 1）
    for (int i = 0; i < size; i++)
    {
        if (result[i] >= 0 &&
            result[i] != 0 &&                        // 忽略 blank 类别 0
            result[i] != (dict_size - 1) &&          // 忽略末尾的额外 blank 类
            !(i > 0 && result[i - 1] == result[i]))  // 去除重复字符
        {
            results += txt_string[result[i]];        // 追加字符
        }
    }

    // 打印识别结果
    std::cout<<"=======识别结果========="<<std::endl;
    std::cout << results << std::endl;
    std::cout<<"======================="<<std::endl;

}

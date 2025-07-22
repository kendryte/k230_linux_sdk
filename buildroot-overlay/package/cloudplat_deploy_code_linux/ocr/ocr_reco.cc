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
 * @brief OCRReco 构造函数，初始化模型、加载字典和设置输入形状信息
 * 
 * @param args        模型配置信息，包括模型路径、字典数量、是否定长等
 * @param isp_shape   图像原始尺寸（通道数、高、宽）
 * @param debug_mode  是否开启调试模式（0关闭，1开启）
 */
OCRReco::OCRReco(config_args args, FrameCHWSize isp_shape, const int debug_mode)
: AIBase(args.kmodel_path.c_str(), "OCRReco", debug_mode)
{
    dict_size = args.dict_num;
    flag = args.fixed_length;

    // 读取字典文件，将每一行的字符串加入 txt_string 列表中
    ifstream dict_string(DICT_STRING);
    while (!dict_string.eof())
    {
        string line;
        while (getline(dict_string, line))
        {
            // 去除每行末尾可能的 '\r' 和前后的空白字符
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            // 如果需要还可以去除两侧的空白字符（可选）
            line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), line.end());
            txt_string.push_back(line);
        }
    }

    // 获取模型输入图像宽高（NCHW 格式中的 H 和 W）
    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];
}

/**
 * @brief OCRReco 析构函数
 */
OCRReco::~OCRReco()
{
}

/**
 * @brief 图像预处理，包括 resize、padding、BGR 转 RGB、打包成 CHW 格式并同步到推理 tensor
 * 
 * @param ori_img OpenCV 读取的原始图像（BGR 格式）
 */
void OCRReco::pre_process(cv::Mat &ori_img)
{
    image_size_ = {ori_img.channels(), ori_img.rows, ori_img.cols};
    input_size_ = {input_shapes_[0][1], input_shapes_[0][2], input_shapes_[0][3]};
    ai2d_out_tensor_ = get_input_tensor(0);

    // 设置 padding + resize 参数（保持宽高比）
    Utils::padding_resize_one_side_set(image_size_, input_size_, ai2d_builder_, cv::Scalar(114, 114, 114));

    // 将 BGR 拆分为三个通道，并转换为 CHW 格式（R 通道优先）
    std::vector<uint8_t> chw_vec;
    std::vector<cv::Mat> bgrChannels(3);
    cv::split(ori_img, bgrChannels);
    for (int i = 2; i >= 0; i--)
    {
        std::vector<uint8_t> data = std::vector<uint8_t>(bgrChannels[i].reshape(1, 1));
        chw_vec.insert(chw_vec.end(), data.begin(), data.end());
    }

    // 创建临时输入 tensor，并填充图像数据
    dims_t rec_in_shape { 1, ori_img.channels(), ori_img.rows, ori_img.cols };
    runtime_tensor rec_input_tensor = host_runtime_tensor::create(typecode_t::dt_uint8, rec_in_shape, hrt::pool_shared).expect("cannot create input tensor");
    auto rec_input_buf = rec_input_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(rec_input_buf.data()), chw_vec.data(), chw_vec.size());
    hrt::sync(rec_input_tensor, sync_op_t::sync_write_back, true).expect("write back input failed");

    // 执行 AI2D 预处理
    ai2d_builder_->invoke(rec_input_tensor, ai2d_out_tensor_).expect("error occurred in ai2d running");
}

/**
 * @brief 执行推理过程，包括模型执行和输出提取
 */
void OCRReco::inference()
{
    this->run();         // 启动模型执行
    this->get_output();  // 获取模型输出 tensor
}

/**
 * @brief 后处理，将模型输出的分类索引转为最终字符串（文字识别结果）
 * 
 * @param results 输出结果字符串（识别文本）
 */
void OCRReco::post_process(string &results)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    results = "";

    float* output = p_outputs_[0];
    int size = input_shapes_[0][3] / 4;  // 时间步数量
    std::vector<int> result;

    // 遍历每个时间步，找到概率最大的类别索引
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

    // 根据规则拼接最终字符串（去重、去除 blank 和 end 符号）
    for (int i = 0; i < size; i++)
    {
        if (result[i] >= 0 && result[i] != 0 && result[i] != (dict_size - 1) && !(i > 0 && result[i - 1] == result[i]))
        {
            results += txt_string[result[i]];
        }
    }
}

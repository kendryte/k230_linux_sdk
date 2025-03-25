/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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
#include "freetype/freetype.h"

OCRReco::OCRReco(const char *kmodel_file, int dict_size, const int debug_mode)
:dict_size(dict_size), AIBase(kmodel_file,"OCRReco", debug_mode)
{
	ifstream dict_string(DICT_STRING);
	while (!dict_string.eof())
    {
        string line;
		while (getline(dict_string, line))
        {
            // 如果行末有回车符 (\r)，将其去除
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
			txt_string.push_back(line);
        }
    }

    model_name_ = "OCRReco";
    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];
    ai2d_out_tensor_ = this -> get_input_tensor(0);
}

OCRReco::OCRReco(const char *kmodel_file, int dict_size, FrameCHWSize isp_shape, const int debug_mode)
:dict_size(dict_size), AIBase(kmodel_file,"OCRReco", debug_mode)
{   
    ifstream dict_string(DICT_STRING);
	while (!dict_string.eof())
    {
        string line;
		while (getline(dict_string, line))
            // 如果行末有回车符 (\r)，将其去除
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
			txt_string.push_back(line);
    }

    model_name_ = "OCRReco";
    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];
    isp_shape_ = isp_shape;
    ai2d_out_tensor_ = this -> get_input_tensor(0);
    Utils::resize(isp_shape, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_);
}

OCRReco::~OCRReco()
{
}

void OCRReco::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::bgr2rgb_and_hwc2chw(ori_img, chw_vec);
    Utils::padding_resize_one_side({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(104, 117, 123));

}

void OCRReco::pre_process(runtime_tensor& img_data)
{
    ScopedTiming st(model_name_ + " pre_process video", debug_mode_);
    ai2d_builder_->invoke(img_data,ai2d_out_tensor_).expect("error occurred in ai2d running");
}

void OCRReco::inference()
{
    this->run();
    this->get_output();
}

std::string OCRReco::post_process()
{
    float* output = p_outputs_[0];
    int size = input_width / 4;
    std::string final_string;

    for (int i = 0; i < size; i++)
    {
        float maxs = -10.f;
        int index = -1;

        // 找到最大值对应的索引
        for (int j = 0; j < dict_size; j++)
        {
            if (maxs < output[i * dict_size + j])
            {
                index = j;
                maxs = output[i * dict_size + j];
            }
        }

        // 检查是否需要将结果加入
        if (index >= 0 && index != dict_size - 1 && !(i > 0 && output[(i-1) * dict_size + index] == output[i * dict_size + index]))
        {
            // 将对应的字符转换为字符串并拼接到final_string中
            final_string += txt_string[index];
            if (debug_mode_ != 0)
                std::cout << txt_string[index] << std::endl;
        }
    }
    return final_string;
}
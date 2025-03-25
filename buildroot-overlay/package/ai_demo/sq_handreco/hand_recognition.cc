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

#include "hand_recognition.h"
#include "vi_vo.h"

HandRecognition::HandRecognition(const char *kmodel_file, const int debug_mode) : AIBase(kmodel_file,"HandRecognition",debug_mode)
{
    model_name_ = "HandRecognition";
    labels_ = {"gun","other","yeah","five"};
    ai2d_out_tensor_ = get_input_tensor(0);
}


HandRecognition::HandRecognition(const char *kmodel_file, FrameCHWSize isp_shape, const int debug_mode) : AIBase(kmodel_file,"HandRecognition", debug_mode)
{
    model_name_ = "HandRecognition";
    labels_ = {"gun","other","yeah","five"};
    // input->isp（Fixed size）
    isp_shape_ = isp_shape;
    dims_t in_shape{1, isp_shape.channel, isp_shape.height, isp_shape.width};
    dims_t out_shape{1, input_shapes_[0][1], input_shapes_[0][2], input_shapes_[0][3]};
    ai2d_out_tensor_ = get_input_tensor(0);
}
HandRecognition::~HandRecognition()
{
}

void HandRecognition::pre_process(cv::Mat ori_img, Bbox &bbox)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);

    std::vector<uint8_t> chw_vec;
    Utils::bgr2rgb_and_hwc2chw(ori_img, chw_vec);
    Utils::crop_resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, bbox, ai2d_out_tensor_);
}

void HandRecognition::pre_process(runtime_tensor& img_data, Bbox &bbox)
{
    ScopedTiming st(model_name_ + " pre_process_video", debug_mode_);
    Utils::crop_resize(bbox, ai2d_builder_, img_data, ai2d_out_tensor_);
}

void HandRecognition::inference()
{
    this->run();
    this->get_output();
}

std::string HandRecognition::post_process()
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    float *output = p_outputs_[0];
    float pred[4] = {0};
    softmax(output, pred, output_shapes_[0][1] );
    auto it = std::max_element(pred, pred + output_shapes_[0][1]);
    size_t idx = it - pred;
    std::string text = labels_[idx] + ":" + std::to_string(round(*it * 100) / 100.0);

    return text;
}

void HandRecognition::softmax(float* x, float* dx, uint32_t len)
{
    float max_value = x[0];
    for (uint32_t i = 0; i < len; i++)
    {
        if (max_value < x[i])
        {
            max_value = x[i];
        }
    }
    for (uint32_t i = 0; i < len; i++)
    {
        x[i] -= max_value;
        x[i] = expf(x[i]);
    }
    float sum_value = 0.0f;
    for (uint32_t i = 0; i < len; i++)
    {
        sum_value += x[i];
    }
    for (uint32_t i = 0; i < len; i++)
    {
        dx[i] = x[i] / sum_value;
    }
}

void HandRecognition::draw_result(cv::Mat& src_img, BoxInfo& results, string text, bool pic_mode)
{   
    int src_w = src_img.cols;
    int src_h = src_img.rows;
	
    if(pic_mode)
    {
        int w = results.x2 - results.x1 + 1;
        int h = results.y2 - results.y1 + 1;
        cv::rectangle(src_img, cv::Rect(static_cast<int>(results.x1), static_cast<int>(results.y1) , w, h), cv::Scalar(255, 255, 255), 2, 2, 0);
        cv::putText(src_img, text , {results.x1,std::max(int(results.y1-10),0)}, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 0, 0), 1, 8, 0);

    }
    else
    {
        int w = results.x2 - results.x1 + 1;
        int h = results.y2 - results.y1 + 1;
        
        int rect_x = results.x1/ SENSOR_WIDTH * src_w;
        int rect_y = results.y1/ SENSOR_HEIGHT * src_h;
        int rect_w = (float)w / SENSOR_WIDTH * src_w;
        int rect_h = (float)h / SENSOR_HEIGHT  * src_h;
        cv::rectangle(src_img, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255,255, 255), 2, 2, 0);
        cv::putText(src_img, text, {rect_x,std::max(int(rect_y-10),0)}, cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 0), 2, 8, 0);
    }   
}

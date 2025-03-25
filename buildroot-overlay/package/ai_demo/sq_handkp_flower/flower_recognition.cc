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
#include "vi_vo.h"
#include "flower_recognition.h"
#include "hand_detection.h"
#include "hand_keypoint.h"

FlowerRecognition::FlowerRecognition(const char *kmodel_file, const int debug_mode) : AIBase(kmodel_file,"FlowerRecognition",debug_mode)
{
    model_name_ = "FlowerRecognition";
    ai2d_out_tensor_ = get_input_tensor(0);
}

FlowerRecognition::FlowerRecognition(const char *kmodel_file, FrameCHWSize isp_shape, const int debug_mode) : AIBase(kmodel_file,"FlowerRecognition", debug_mode)
{
    model_name_ = "FlowerRecognition";
    // input->isp（Fixed size）
    isp_shape_ = isp_shape;
    dims_t in_shape{1, isp_shape.channel, isp_shape.height, isp_shape.width};
    // ouput（Fixed size）
    dims_t out_shape{1, input_shapes_[0][1], input_shapes_[0][2], input_shapes_[0][3]};
    ai2d_out_tensor_ = get_input_tensor(0);
}

FlowerRecognition::~FlowerRecognition()
{
}

void FlowerRecognition::pre_process(cv::Mat ori_img, Bbox &bbox)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);

    std::vector<uint8_t> chw_vec;
    Utils::bgr2rgb_and_hwc2chw(ori_img, chw_vec);
    Utils::crop_resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, bbox, ai2d_out_tensor_);
}

void FlowerRecognition::pre_process(runtime_tensor& img_data, Bbox &bbox)
{
    ScopedTiming st(model_name_ + " pre_process_video", debug_mode_);
    Utils::crop_resize(bbox, ai2d_builder_, img_data, ai2d_out_tensor_);
}

void FlowerRecognition::inference()
{
    this->run();
    this->get_output();
}

std::string FlowerRecognition::post_process()
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    float *output = p_outputs_[0];
    float pred[102] = {0};
    softmax(output, pred, output_shapes_[0][1] );
    auto it = std::max_element(pred, pred + output_shapes_[0][1]);
    size_t idx = it - pred;
    std::string text = labels[idx] + ":" + std::to_string(round(*it * 100) / 100.0);

    return text;
}

void FlowerRecognition::draw_flower_results(cv::Mat& img, vector<Sort::TrackingBox>& frameTrackingResult, vector<HandKeyPointInfo>& hand_keypoint_results, vector<BoxInfo>& flower_recognition_results)
{
    int src_width = img.cols;
    int src_height = img.rows;

    for(int i=0;i< frameTrackingResult.size();i++)
    {
        auto tb = frameTrackingResult[i];

        int rect_x = tb.box.x / SENSOR_WIDTH * src_width;
        int rect_y = tb.box.y / SENSOR_HEIGHT  * src_height;
        int rect_w = (float)tb.box.width / SENSOR_WIDTH * src_width;
        int rect_h = (float)tb.box.height / SENSOR_HEIGHT  * src_height;

        cv::rectangle(img, cv::Rect(rect_x, rect_y, rect_w, rect_h), cv::Scalar(255, 0, 255), 2, 2, 0);

        HandKeypoint::draw_keypoints(img, hand_keypoint_results[i], false);
    }
    int flower_x1 = flower_recognition_results[0].x1 / SENSOR_WIDTH * src_width;
    int flower_y1 = flower_recognition_results[0].y1 / SENSOR_HEIGHT  * src_height;
    int flower_x2 = flower_recognition_results[0].x2 / SENSOR_WIDTH * src_width;
    int flower_y2 = flower_recognition_results[0].y2 / SENSOR_HEIGHT  * src_height;
    cv::circle(img, cv::Point(flower_x1, flower_y1), 6, cv::Scalar(0,0,0), 3);
    cv::circle(img, cv::Point(flower_x2, flower_y2), 5, cv::Scalar(0,0,0), 3);
    cv::rectangle(img, cv::Point(flower_x1, flower_y1),cv::Point(flower_x2, flower_y2), cv::Scalar(0, 255, 255), 2, 2, 0);
    if (flower_x1 < flower_x2)
    {
        cv::putText(img, flower_recognition_results[0].label, cv::Point(flower_x1, flower_y1-20),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 0, 0), 2);
    }
    else
    {
        cv::putText(img, flower_recognition_results[0].label, cv::Point(flower_x2, flower_y2-20),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 0, 0), 2);
    }
}

void FlowerRecognition::softmax(float* x, float* dx, uint32_t len)
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

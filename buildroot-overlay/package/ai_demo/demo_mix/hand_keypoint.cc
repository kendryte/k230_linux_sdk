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
#include "hand_keypoint.h"

double vector_2d_angle(std::vector<double> v1, std::vector<double> v2)
{
    double v1_x = v1[0];
    double v1_y = v1[1];
    double v2_x = v2[0];
    double v2_y = v2[1];
    double v1_norm = std::sqrt(v1_x * v1_x + v1_y * v1_y);
    double v2_norm = std::sqrt(v2_x * v2_x + v2_y * v2_y);
    double dot_product = v1_x * v2_x + v1_y * v2_y;
    double cos_angle = dot_product / (v1_norm * v2_norm);
    double angle = std::acos(cos_angle) * 180 / M_PI;
    if (angle > 180.0)
    {
        return 65535.0;
    }
    return angle;
}

std::vector<double> hand_angle(float* results)
{
    double angle_;
    std::vector<double> angle_list;
    //---------------------------- thumb 大拇指角度
    angle_ = vector_2d_angle(
        {(results[0] - results[4]), (results[1] - results[5])},
        {(results[6] - results[8]), (results[7] - results[9])}
    );
    angle_list.push_back(angle_);
    //---------------------------- index 食指角度
    angle_ = vector_2d_angle(
        {(results[0] - results[12]), (results[1] - results[13])},
        {(results[14] - results[16]), (results[15] - results[17])}
    );
    angle_list.push_back(angle_);
    //---------------------------- middle 中指角度
    angle_ = vector_2d_angle(
        {(results[0] - results[20]), (results[1] - results[21])},
        {(results[22] - results[24]), (results[23] - results[25])}
    );
    angle_list.push_back(angle_);
    //---------------------------- ring 无名指角度
    angle_ = vector_2d_angle(
        {(results[0] - results[28]), (results[1] - results[29])},
        {(results[30] - results[32]), (results[31] - results[33])}
    );
    angle_list.push_back(angle_);
    //---------------------------- pink 小拇指角度
    angle_ = vector_2d_angle(
        {(results[0] - results[36]), (results[1] - results[37])},
        {(results[38] - results[40]), (results[39] - results[41])}
    );
    angle_list.push_back(angle_);
    return angle_list;
}

std::string h_gesture(std::vector<double> angle_list)
{
    int thr_angle = 65;
    int thr_angle_thumb = 53;
    int thr_angle_s = 49;
    std::string gesture_str="other";

    bool present = std::find(angle_list.begin(),angle_list.end(),65535) != angle_list.end();
    if (present)
    {
        std::cout<<"gesture_str:"<<gesture_str<<std::endl;
    }else{
        if (angle_list[0]>thr_angle_thumb && angle_list[1]>thr_angle && angle_list[2]>thr_angle && (angle_list[3]>thr_angle) && (angle_list[4]>thr_angle))
            {gesture_str = "fist";}
        else if ((angle_list[0]<thr_angle_s)  && (angle_list[1]<thr_angle_s) && (angle_list[2]<thr_angle_s) && (angle_list[3]<thr_angle_s) && (angle_list[4]<thr_angle_s))
            {gesture_str = "five";}
        else if ((angle_list[0]<thr_angle_s)  && (angle_list[1]<thr_angle_s) && (angle_list[2]>thr_angle) && (angle_list[3]>thr_angle) && (angle_list[4]>thr_angle))
            {gesture_str = "gun";}
        else if ((angle_list[0]<thr_angle_s)  && (angle_list[1]<thr_angle_s) && (angle_list[2]>thr_angle) && (angle_list[3]>thr_angle) && (angle_list[4]<thr_angle_s))
            {gesture_str = "love";}
        else if ((angle_list[0]>5)  && (angle_list[1]<thr_angle_s) && (angle_list[2]>thr_angle) && (angle_list[3]>thr_angle) && (angle_list[4]>thr_angle))
            {gesture_str = "one";}
        else if ((angle_list[0]<thr_angle_s)  && (angle_list[1]>thr_angle) && (angle_list[2]>thr_angle) && (angle_list[3]>thr_angle) && (angle_list[4]<thr_angle_s))
            {gesture_str = "six";}
        else if ((angle_list[0]>thr_angle_thumb)  && (angle_list[1]<thr_angle_s) && (angle_list[2]<thr_angle_s) && (angle_list[3]<thr_angle_s) && (angle_list[4]>thr_angle))
            {gesture_str = "three";}
        else if ((angle_list[0]<thr_angle_s)  && (angle_list[1]>thr_angle) && (angle_list[2]>thr_angle) && (angle_list[3]>thr_angle) && (angle_list[4]>thr_angle))
            {gesture_str = "thumbUp";}
        else if ((angle_list[0]>thr_angle_thumb)  && (angle_list[1]<thr_angle_s) && (angle_list[2]<thr_angle_s) && (angle_list[3]>thr_angle) && (angle_list[4]>thr_angle))
            {gesture_str = "yeah";}
    }
    return gesture_str;
}

HandKeypoint::HandKeypoint(const char *kmodel_file, const int debug_mode)
:AIBase(kmodel_file, "HandKeypoint", debug_mode)
{
    model_name_ = "HandKeypoint";
    ai2d_out_tensor_ = get_input_tensor(0);
}

HandKeypoint::HandKeypoint(const char *kmodel_file, FrameCHWSize isp_shape, const int debug_mode)
: AIBase(kmodel_file,"HandKeypoint", debug_mode)
{
    model_name_ = "HandKeypoint";
    isp_shape_ = isp_shape;
    dims_t in_shape{1, isp_shape.channel, isp_shape.height, isp_shape.width};
    int isp_size = isp_shape.channel * isp_shape.height * isp_shape.width;
    ai2d_out_tensor_ = get_input_tensor(0);
}

HandKeypoint::~HandKeypoint()
{
}

void HandKeypoint::pre_process(cv::Mat ori_img, Bbox &bbox)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::hwc_to_chw(ori_img, chw_vec);
    Utils::crop_resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, bbox, ai2d_out_tensor_);
}

// for video
void HandKeypoint::pre_process(runtime_tensor& img_data, Bbox &bbox)
{
    ScopedTiming st(model_name_ + " pre_process_video", debug_mode_);
    Utils::crop_resize(bbox, ai2d_builder_, img_data, ai2d_out_tensor_);
}

void HandKeypoint::inference()
{
    this->run();
    this->get_output();
}

void HandKeypoint::post_process(Bbox &bbox, HandKeyPointInfo& results)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    float *pred = p_outputs_[0];

    int64_t output_tensor_size = output_shapes_[0][1];// 关键点输出 （x,y）*21= 42
    for (unsigned i = 0; i < output_tensor_size / 2; i++)
    {
        float x_kp;
        float y_kp;
        x_kp = pred[i * 2] * bbox.w + bbox.x;
        y_kp = pred[i * 2 + 1] * bbox.h + bbox.y;

        results.pred.push_back(static_cast<int>(x_kp));
        results.pred.push_back(static_cast<int>(y_kp));
    }
}    

std::string HandKeypoint::draw_keypoints(cv::Mat &img, BoxInfo& bbox, HandKeyPointInfo& post_process_results, bool pic_mode)
{
    // ScopedTiming st(model_name_ + " draw_keypoints", debug_mode_);
    int src_width = img.cols, src_height = img.rows;
    float *results = post_process_results.pred.data();
    // 绘制关键点像素坐标
    int64_t output_tensor_size = post_process_results.pred.size();
    
    if(pic_mode)
    {
        int w = bbox.x2 - bbox.x1 + 1;
        int h = bbox.y2 - bbox.y1 + 1;
        cv::rectangle(img, cv::Rect(static_cast<int>(bbox.x1), static_cast<int>(bbox.y1) , w, h), cv::Scalar(255, 255, 255), 2, 2, 0);

        for (unsigned i = 0; i < output_tensor_size / 2; i++)
        {
            cv::circle(img, cv::Point(results[i * 2], results[i * 2 + 1]), 2, cv::Scalar(255, 155, 0), 3);
        }

        for (unsigned k = 0; k < 5; k++)
        {
            int i = k*8;
            unsigned char R = 255, G = 0, B = 0;

            switch(k)
            {
                case 0:R = 255; G = 0; B = 0;break;
                case 1:R = 255; G = 0; B = 255;break;
                case 2:R = 255; G = 255; B = 0;break;
                case 3:R = 0; G = 255; B = 0;break;
                case 4:R = 0; G = 0; B = 255;break;
                default: std::cout << "error" << std::endl;
            }

            cv::line(img, cv::Point(results[0], results[1]), cv::Point(results[i + 2], results[i + 3]), cv::Scalar(B,G,R), 2, cv::LINE_AA);
            cv::line(img, cv::Point(results[i + 2], results[i + 3]), cv::Point(results[i + 4], results[i + 5]), cv::Scalar(B, G, R), 2, cv::LINE_AA);
            cv::line(img, cv::Point(results[i + 4], results[i + 5]), cv::Point(results[i + 6], results[i + 7]), cv::Scalar(B, G, R), 2, cv::LINE_AA);
            cv::line(img, cv::Point(results[i + 6], results[i + 7]), cv::Point(results[i + 8], results[i + 9]), cv::Scalar(B, G, R), 2, cv::LINE_AA);
        }
        std::vector<double> angle_list = hand_angle(results);
        std::string gesture = h_gesture(angle_list);
        std::string text = "Gesture: " + gesture;
        cv::putText(img, text, cv::Point(bbox.x1,std::max(int(bbox.y1-10),0)),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 195, 0), 2);
        cv::putText(img, text, cv::Point(bbox.x1,std::max(int(bbox.y1-10),0)),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(0, 150, 255), 2);
        return text;
    }
    else
    {         
        int w = bbox.x2 - bbox.x1 + 1;
        int h = bbox.y2 - bbox.y1 + 1;
        
        int rect_x = bbox.x1/ SENSOR_WIDTH * src_width;
        int rect_y = bbox.y1/ SENSOR_HEIGHT * src_height;
        int rect_w = (float)w / SENSOR_WIDTH * src_width;
        int rect_h = (float)h / SENSOR_HEIGHT  * src_height;
        cv::rectangle(img, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255,255, 255), 2, 2, 0);

        for (unsigned i = 0; i < output_tensor_size / 2; i++)
        {
            results[i * 2] = results[i * 2] / SENSOR_WIDTH * src_width;
            results[i * 2 + 1] = results[i * 2 + 1] / SENSOR_HEIGHT * src_height;
            cv::circle(img, cv::Point(results[i * 2], results[i * 2 + 1]), 2, cv::Scalar(255, 155, 0), 3);
        }

        for (unsigned k = 0; k < 5; k++)
        {
            int i = k*8;
            unsigned char R = 255, G = 0, B = 0;

            switch(k)
            {
                case 0:R = 255; G = 0; B = 0;break;
                case 1:R = 255; G = 0; B = 255;break;
                case 2:R = 255; G = 255; B = 0;break;
                case 3:R = 0; G = 255; B = 0;break;
                case 4:R = 0; G = 0; B = 255;break;
                default: std::cout << "error" << std::endl;
            }

            cv::line(img, cv::Point(results[0], results[1]), cv::Point(results[i + 2], results[i + 3]), cv::Scalar(B,G,R), 2, cv::LINE_AA);
            cv::line(img, cv::Point(results[i + 2], results[i + 3]), cv::Point(results[i + 4], results[i + 5]), cv::Scalar(B, G, R), 2, cv::LINE_AA);
            cv::line(img, cv::Point(results[i + 4], results[i + 5]), cv::Point(results[i + 6], results[i + 7]), cv::Scalar(B, G, R), 2, cv::LINE_AA);
            cv::line(img, cv::Point(results[i + 6], results[i + 7]), cv::Point(results[i + 8], results[i + 9]), cv::Scalar(B, G, R), 2, cv::LINE_AA);
        }

        std::vector<double> angle_list = hand_angle(results);
        std::string gesture = h_gesture(angle_list);
        std::string text = "Gesture: " + gesture;
        cv::putText(img, text, cv::Point(rect_x,rect_y),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 0, 0), 2);
        return text;
    } 
}

void HandKeypoint::draw_keypoints_video(cv::Mat &img, BoxInfo& bbox, HandKeyPointInfo& post_process_results, bool pic_mode)
{
    // ScopedTiming st(model_name_ + " draw_keypoints", debug_mode_);
    int src_width = img.cols, src_height = img.rows;
    float *results = post_process_results.pred.data();
    // 绘制关键点像素坐标
    int64_t output_tensor_size = post_process_results.pred.size();
    
    int w = bbox.x2 - bbox.x1 + 1;
    int h = bbox.y2 - bbox.y1 + 1;
    int rect_x = bbox.x1/ SENSOR_WIDTH * src_width;
    int rect_y = bbox.y1/ SENSOR_HEIGHT * src_height;
    int rect_w = (float)w / SENSOR_WIDTH * src_width;
    int rect_h = (float)h / SENSOR_HEIGHT  * src_height;
    cv::rectangle(img, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar( 255,255, 255), 2, 2, 0);

    for (unsigned i = 0; i < output_tensor_size / 2; i++)
    {
        results[i * 2] = results[i * 2] / SENSOR_WIDTH * src_width;
        results[i * 2 + 1] = results[i * 2 + 1] / SENSOR_HEIGHT * src_height;
        cv::circle(img, cv::Point(results[i * 2], results[i * 2 + 1]), 2, cv::Scalar(255, 155, 0), 3);
    }

    for (unsigned k = 0; k < 5; k++)
    {
        int i = k*8;
        unsigned char R = 255, G = 0, B = 0;

        switch(k)
        {
            case 0:R = 255; G = 0; B = 0;break;
            case 1:R = 255; G = 0; B = 255;break;
            case 2:R = 255; G = 255; B = 0;break;
            case 3:R = 0; G = 255; B = 0;break;
            case 4:R = 0; G = 0; B = 255;break;
            default: std::cout << "error" << std::endl;
        }
        cv::line(img, cv::Point(results[0], results[1]), cv::Point(results[i + 2], results[i + 3]), cv::Scalar(B,G,R), 2, cv::LINE_AA);
        cv::line(img, cv::Point(results[i + 2], results[i + 3]), cv::Point(results[i + 4], results[i + 5]), cv::Scalar(B, G, R), 2, cv::LINE_AA);
        cv::line(img, cv::Point(results[i + 4], results[i + 5]), cv::Point(results[i + 6], results[i + 7]), cv::Scalar(B, G, R), 2, cv::LINE_AA);
        cv::line(img, cv::Point(results[i + 6], results[i + 7]), cv::Point(results[i + 8], results[i + 9]), cv::Scalar(B, G, R), 2, cv::LINE_AA);
    }
    std::vector<double> angle_list = hand_angle(results);
    std::string gesture = h_gesture(angle_list);
    std::string text = "Gesture: " + gesture;
    cv::putText(img, text, cv::Point(rect_x,rect_y),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 0, 0), 2);
}


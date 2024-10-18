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
#include "face_gender.h"
#include "vi_vo.h"
#include <vector>

FaceGender::FaceGender(const char *kmodel_file, const int debug_mode) : AIBase(kmodel_file,"FaceGender",debug_mode)
{
    model_name_ = "FaceGender";
	margin_ = 0.4;
    ai2d_out_tensor_ = get_input_tensor(0);
}

FaceGender::FaceGender(const char *kmodel_file, FrameCHWSize isp_shape, const int debug_mode) : AIBase(kmodel_file,"FaceGender", debug_mode)
{
    model_name_ = "FaceGender";
	margin_ = 0.4;

	// paddr_ = paddr;
    isp_shape_ = isp_shape;
    ai2d_out_tensor_ = get_input_tensor(0);
}

FaceGender::~FaceGender()
{
}

// ai2d for image
void FaceGender::pre_process(cv::Mat ori_img,Bbox& rect)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
	
	int ori_w = ori_img.cols;
    int ori_h = ori_img.rows;
    float x1 = rect.x, y1 = rect.y;
    float x2 = rect.x + rect.w, y2 = rect.y + rect.h;
    int xw1 = std::max(int(x1 - margin_ * rect.w), int(0));
    int yw1 = std::max(int(y1 - margin_ * rect.h), int(0));
    int xw2 = std::min(int(x2 + margin_ * rect.w), int(ori_w - 1));
    int yw2 = std::min(int(y2 + margin_ * rect.h), int(ori_h - 1));
    std::vector<uint8_t> hwc_vec = std::vector<uint8_t>(ori_img.reshape(1, 1));
    Bbox crop_info = {xw1,yw1,xw2-xw1,yw2-yw1};
    Utils::crop_resize_hwc({ori_img.channels(), ori_img.rows, ori_img.cols}, hwc_vec,crop_info, ai2d_out_tensor_);
}

// ai2d for video
void FaceGender::pre_process(runtime_tensor& img_data,Bbox& rect)
{
    float x1 = rect.x, y1 = rect.y;
    float x2 = rect.x + rect.w, y2 = rect.y + rect.h;
    int xw1 = std::max(int(x1 - margin_ * rect.w), int(0));
    int yw1 = std::max(int(y1 - margin_ * rect.h), int(0));
    int xw2 = std::min(int(x2 + margin_ * rect.w), int(isp_shape_.width - 1));
    int yw2 = std::min(int(y2 + margin_ * rect.h), int(isp_shape_.height - 1));
	Bbox crop_info = {xw1,yw1,xw2-xw1,yw2-yw1};
    Utils::crop_resize_hwc(crop_info,ai2d_builder_,img_data, ai2d_out_tensor_);
}

void FaceGender::inference()
{
    this->run();
    this->get_output();
}

void FaceGender::post_process(FaceGenderInfo& result)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
	result.gender = (p_outputs_[0][0]>0.5) ? "F" : "M";
}

void FaceGender::draw_result(cv::Mat& src_img,Bbox& bbox,FaceGenderInfo& result, bool pic_mode)
{
    int src_w = src_img.cols;
    int src_h = src_img.rows;
    int max_src_size = std::max(src_w,src_h);

    char text[30];
	sprintf(text, "%s",result.gender.c_str());

    if(pic_mode)
    {
        cv::rectangle(src_img, cv::Rect(bbox.x, bbox.y , bbox.w, bbox.h), cv::Scalar(255, 255, 255), 2, 2, 0);
        cv::putText(src_img, text , {bbox.x,std::max(int(bbox.y-10),0)}, cv::FONT_HERSHEY_COMPLEX, 0.7, cv::Scalar(255, 0, 0), 1, 8, 0);
    }
    else
    {
		int x = bbox.x / isp_shape_.width * src_w;
        int y = bbox.y / isp_shape_.height * src_h;
        int w = bbox.w / isp_shape_.width * src_w;
        int h = bbox.h / isp_shape_.height * src_h;
        cv::rectangle(src_img, cv::Rect(x, y , w, h), cv::Scalar(255,255, 255, 255), 2, 2, 0);
        if(result.gender == "F")
			cv::putText(src_img,text,cv::Point(x,std::max(int(y-10),0)),cv::FONT_HERSHEY_COMPLEX,2,cv::Scalar(255,255, 0, 255), 2, 8, 0);
		else
			cv::putText(src_img,text,cv::Point(x,std::max(int(y-10),0)),cv::FONT_HERSHEY_COMPLEX,2,cv::Scalar(255,255, 255, 0), 2, 8, 0);
    }  
}

void FaceGender::draw_result_video(cv::Mat& src_img,Bbox& bbox,FaceGenderInfo& result)
{
    int src_w = src_img.cols;
    int src_h = src_img.rows;
    int max_src_size = std::max(src_w,src_h);

    char text[30];
	sprintf(text, "%s",result.gender.c_str());

	int x = bbox.x / SENSOR_WIDTH * src_w;
    int y = bbox.y / SENSOR_HEIGHT * src_h;
    int w = bbox.w / SENSOR_WIDTH * src_w;
    int h = bbox.h / SENSOR_HEIGHT * src_h;
    cv::rectangle(src_img, cv::Rect(x, y , w, h), cv::Scalar(255, 255, 255), 2, 2, 0);
    if(result.gender == "F")
		cv::putText(src_img,text,cv::Point(x,std::max(int(y-10),0)),cv::FONT_HERSHEY_COMPLEX,2,cv::Scalar(255, 0, 255), 2, 8, 0);
	else
		cv::putText(src_img,text,cv::Point(x,std::max(int(y-10),0)),cv::FONT_HERSHEY_COMPLEX,2,cv::Scalar(255, 255, 0), 2, 8, 0);
}
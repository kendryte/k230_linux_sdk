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

#include "ocr_box.h"

OCRBox::OCRBox(const char *kmodel_file, float threshold, float box_thresh, const int debug_mode)
:threshold(threshold), box_thresh(box_thresh), AIBase(kmodel_file,"OCRBox", debug_mode)
{
    model_name_ = "OCRBox";
    flag = 0;

    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];

    output = new float[input_shapes_[0][2]*input_shapes_[0][3]];

    ai2d_out_tensor_ = this -> get_input_tensor(0);
}

OCRBox::OCRBox(const char *kmodel_file, float threshold, float box_thresh, FrameCHWSize isp_shape, const int debug_mode)
:threshold(threshold), box_thresh(box_thresh), AIBase(kmodel_file,"OCRBox", debug_mode)
{
    model_name_ = "OCRBox";
    flag = 0;

    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];

    output = new float[input_shapes_[0][2]*input_shapes_[0][3]];

    ai2d_out_tensor_ = this -> get_input_tensor(0);
    // fixed padding resize param
    Utils::padding_resize(isp_shape, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_, cv::Scalar(104, 117, 123), false);
}


OCRBox::~OCRBox()
{
    // delete[] output;
}

// ai2d for image
void OCRBox::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::bgr2rgb_and_hwc2chw(ori_img, chw_vec);
    Utils::padding_resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(104, 117, 123));
}

// ai2d for video
void OCRBox::pre_process(runtime_tensor& img_data)
{
    ScopedTiming st(model_name_ + " pre_process video", debug_mode_);
    ai2d_builder_->invoke(img_data, ai2d_out_tensor_).expect("error occurred in ai2d running");
}

void OCRBox::inference()
{
    // this->set_input_tensor(0, ai2d_out_tensor_);
    this->run();
    this->get_output();
}

void OCRBox::post_process(FrameSize frame_size, vector<Boxb> &results)
{
    for(int i = 0; i < input_height * input_width; i++)
        output[i] = p_outputs_[0][2*i];

    int row = frame_size.height;
    int col = frame_size.width;
    float ratio_w = 1.0 * input_width / col;
    float ratio_h = 1.0 * input_height / row;
    float ratio = std::min(ratio_h, ratio_w);
    int unpad_w = (int)round(col * ratio);
    int unpad_h = (int)round(row * ratio);

    float dw = (1.0 * input_width - unpad_w);
    float dh = (1.0 * input_height - unpad_h);

    int h = int(round((dh - 0.1) / 2));
    int w = int(round((dw - 0.1) / 2 ));

    Mat src(input_height, input_width, CV_32FC1, output);
    Mat mask(src > threshold);
    vector<vector<cv::Point>> contours;
    vector<cv::Vec4i> hierarchy;
    cv::findContours(mask, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    int num = contours.size();
    for(int i = 0; i < num; i++)
    {
        if(contours[i].size() < 4)
            continue;

        Boxb b;
        getBox(b, contours[i]);
        vector<cv::Point> con;
        unclip(contours[i], con);

        getBox(b, con);
        float score = boxScore(src, contours[i], b, w, h, frame_size);
        if (score < box_thresh)
            continue;
        b.score = score;
        
        float ratiow = 1.0 * input_width / frame_size.width;
        float ratioh = 1.0 * input_height / frame_size.height;

        for(int i = 0; i < 4; i++)
        {
            
            b.vertices[i].x = std::max(std::min((int)b.vertices[i].x, input_width), 0);
            b.vertices[i].y = std::max(std::min((int)b.vertices[i].y, input_height), 0);
            
            if(flag == 1){
                b.ver_src[i].x = b.vertices[i].x;
                b.ver_src[i].y = b.vertices[i].y;
                if (ratioh < ratiow)
                {
                    b.vertices[i].x = (b.vertices[i].x - w) / input_height * frame_size.height;
                    b.vertices[i].y = (b.vertices[i].y - h) / input_height * frame_size.height;
                }
                else
                {
                    b.vertices[i].x = (b.vertices[i].x - w) / input_width * frame_size.width;
                    b.vertices[i].y = (b.vertices[i].y - h) / input_width * frame_size.width;
                }
            }
            else
            {
                b.vertices[i].x = (b.vertices[i].x - w) / (input_width - 2 * w) * frame_size.width;
                b.vertices[i].y = (b.vertices[i].y - h) / (input_height - 2 * h) * frame_size.height;
                b.ver_src[i].x = b.vertices[i].x;
                b.ver_src[i].y = b.vertices[i].y;
            }
            b.vertices[i].x = std::max((float)0, std::min(b.vertices[i].x, (float)frame_size.width));
            b.vertices[i].y = std::max((float)0, std::min(b.vertices[i].y, (float)frame_size.height));
        }
        results.push_back(b);
    }
}

void OCRBox::getBox(Boxb& b, vector<cv::Point> contours)
{
    cv::RotatedRect minrect = minAreaRect(contours);
    cv::Point2f vtx[4];
    minrect.points(vtx);
    for(int i = 0; i < 4; i++)
    {
        b.vertices[i].x = vtx[i].x;
        b.vertices[i].y = vtx[i].y;
    }
}

double OCRBox::distance(cv::Point p0, cv::Point p1)
{
    return sqrt((p0.x - p1.x) * (p0.x - p1.x) + (p1.y - p0.y) * (p1.y - p0.y));
}

void OCRBox::unclip(vector<cv::Point> contours, vector<cv::Point>& con)
{
    Path subj;
    Paths solution;
    double dis = 0.0;
    for(int i = 0; i < contours.size(); i++)
        subj << IntPoint(contours[i].x, contours[i].y);
    

    for(int i = 0; i < contours.size() - 1; i++)
        dis += distance(contours[i], contours[i+1]);
    

    double dis1 = (-1 * Area(subj)) * 1.5 / dis;

    ClipperOffset co;
    co.AddPath(subj, jtSquare, etClosedPolygon);
    co.Execute(solution, dis1);
    Path tmp = solution[0];
    
    for(int i = 0; i < tmp.size(); i++)
    {
        cv::Point p(tmp[i].X, tmp[i].Y);
        con.push_back(p);
    }
    for(int i = 0; i < con.size(); i++)
        subj << IntPoint(con[i].x, con[i].y);
}

float OCRBox::boxScore(Mat src, vector<cv::Point> contours, Boxb& b, int w, int h, FrameSize frame_size)
{
    int xmin = input_width;
    int xmax = 0;
    int ymin = input_height;
    int ymax = 0;
    for(int i = 0; i < contours.size(); i++)
    {
        xmin = floor((contours[i].x < xmin ? contours[i].x : xmin));
        xmax = ceil((contours[i].x > xmax ? contours[i].x : xmax));
        ymin = floor((contours[i].y < ymin ? contours[i].y : ymin));
        ymax = ceil((contours[i].y > ymax ? contours[i].y : ymax));
    }

    for(int i = 0; i < contours.size(); i++)
    {
        contours[i].x = contours[i].x - xmin;
        contours[i].y = contours[i].y - ymin;
    }
    vector<vector<cv::Point>> vec;
    vec.clear();
    vec.push_back(contours);
    float ratiow = 1.0 * input_width / frame_size.width;
    float ratioh = 1.0 * input_height / frame_size.height;
    if (flag == 1)
    {
        if (ratioh < ratiow)
        {
            b.meanx = ((1.0 * xmin + xmax) / 2 - w) / input_height * frame_size.height;
            b.meany = ((1.0 * ymin + ymax) / 2 - h) / input_height * frame_size.height;
        }
        else
        {
            b.meanx = ((1.0 * xmin + xmax) / 2 - w) / input_width * frame_size.width;
            b.meany = ((1.0 * ymin + ymax) / 2 - h) / input_width * frame_size.width;
        }
    }
    else
    {
        b.meanx = ((1.0 * xmin + xmax) / 2 - w) / (input_width - 2 * w) * frame_size.width;
        b.meany = ((1.0 * ymin + ymax) / 2 - h) / (input_height - 2 * h) * frame_size.height;
    }
    Mat img = Mat::zeros(ymax - ymin + 1, xmax - xmin + 1, CV_8UC1);
    cv::fillPoly(img, vec, cv::Scalar(1));
    return (float)cv::mean(src(cv::Rect(xmin, ymin, xmax-xmin+1, ymax-ymin+1)), img)[0];  
}

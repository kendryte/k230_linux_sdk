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
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.f
 */

#include "pose_detect.h"
#include "utils.h"


// for image
poseDetect::poseDetect(const char *kmodel_file, float obj_thresh,float nms_thresh,  const int debug_mode): obj_thresh_(obj_thresh),nms_thresh_(nms_thresh), AIBase(kmodel_file,"poseDetect", debug_mode)
{
    model_name_ = "poseDetect";
    
    ai2d_out_tensor_ = get_input_tensor(0);
}   

// for video
poseDetect::poseDetect(const char *kmodel_file, float obj_thresh,float nms_thresh, FrameCHWSize isp_shape, const int debug_mode): obj_thresh_(obj_thresh),nms_thresh_(nms_thresh), AIBase(kmodel_file,"poseDetect", debug_mode)
{
    model_name_ = "poseDetect";

    isp_shape_ = isp_shape;
    dims_t in_shape{1, isp_shape_.channel, isp_shape_.height, isp_shape_.width};
    int isp_size = isp_shape_.channel * isp_shape_.height * isp_shape_.width;

    // ai2d_out_tensor
    ai2d_out_tensor_ = get_input_tensor(0);
    // fixed padding resize param
    Utils::padding_resize_params(params,isp_shape_, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_, cv::Scalar(114, 114, 114), false);
}

poseDetect::~poseDetect()
{

}

// ai2d for image
void poseDetect::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::hwc_to_chw(ori_img, chw_vec);
    Utils::padding_resize_params(params,{ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(114, 114, 114));
}

// ai2d for video
void poseDetect::pre_process(runtime_tensor& img_data)
{
    ScopedTiming st(model_name_ + " pre_process video", debug_mode_);
    ai2d_builder_->invoke(img_data,ai2d_out_tensor_).expect("error occurred in ai2d running");
}

void poseDetect::inference()
{
    this->run();
    this->get_output();
}

bool poseDetect::BatchDetect(  float* all_data, std::vector<std::vector<OutputPose>>& output,cv::Vec4d params)
{

    _outputTensorShape = { output_shapes_[0][0], output_shapes_[0][1], output_shapes_[0][2] };
    _anchorLength = output_shapes_[0][1];

    // [1, 56 ,8400] -> [1, 8400, 56]
    cv::Mat output0 = cv::Mat(cv::Size((int)_outputTensorShape[2], (int)_outputTensorShape[1]), CV_32F, all_data).t(); 
 
    float* pdata = (float*)output0.data; // [classid,x,y,w,h,x,y,...21个点]
    int rows = output0.rows; // 预测框的数量 8400
    // 一张图片的预测框
 
    vector<float> confidences;
    vector<cv::Rect> boxes;
    vector<int> labels;
    vector<vector<float>> kpss;

    for (int r=0; r<rows; ++r){
 
        // 得到人类别概率
        auto kps_ptr = pdata + 5;
 
        // 预测框坐标映射到原图上
        float score = pdata[4];
        
        if(score > obj_thresh_){
            float x = (pdata[0] - params[2]) / params[0]; //x
            float y = (pdata[1] - params[3]) / params[1]; //y
            float w = pdata[2] / params[0]; //w
            float h = pdata[3] / params[1]; //h
 
            float left = MAX(int(x - 0.5 *w +0.5), 0);
            float top = MAX(int(y - 0.5*h + 0.5), 0);
 
            std::vector<float> kps;
            for (int k=0; k< 17; k++){
                
                float kps_x = (*(kps_ptr + 3 * k) - params[2]) / params[0];
                float kps_y = (*(kps_ptr + 3 * k + 1) - params[3]) / params[1];
                float kps_s = *(kps_ptr + 3 * k + 2);
 
                kps.push_back(kps_x);
                kps.push_back(kps_y);
                kps.push_back(kps_s);
            }
 
            confidences.push_back(score);
            labels.push_back(0);
            kpss.push_back(kps);
            boxes.push_back(cv::Rect(left, top, float(w + 0.5), float(h + 0.5)));
        }
        pdata += _anchorLength; //下一个预测框
    }
 
    // 对一张图的预测框执行NMS处理
    vector<int> nms_result;
    
    std::vector<BoxInfo> boxinfo_results;
    BoxInfo res;
    float x1,y1,x2,y2,score_;
    int label,idx;
    for(int i=0;i<boxes.size();i++)
    {
        x1 =  float( boxes[i].tl().x ) ;
        y1 = float( boxes[i].tl().y ) ;
        x2 = float( boxes[i].br().x ) ;
        y2 = float( boxes[i].br().y ) ;
        score_ = confidences[i];
        label = labels[i];
        idx = i;

        res = {  x1,y1,x2,y2,score_,label,idx };
        boxinfo_results.push_back(res);
    }


    nms_pose(boxinfo_results, nms_thresh_,nms_result);

    // 对一张图片：依据NMS处理得到的索引，得到类别id、confidence、box，并置于结构体OutputDet的容器中
    vector<OutputPose> temp_output;
    for (size_t i=0; i<boxinfo_results.size(); ++i){
        int idx = boxinfo_results[i].idx;
        OutputPose result;
 
        result.confidence = confidences[idx];
        result.box = boxes[idx];
        result.label = labels[idx];
        result.kps = kpss[idx];

        temp_output.push_back(result);
    }
    output.push_back(temp_output); // 多张图片的输出；添加一张图片的输出置于此容器中
 
    if (output.size())
        return true;
    else
        return false;
 
}

bool poseDetect::Detect(float* all_data, std::vector<OutputPose> &output,cv::Vec4d params){
    vector<vector<OutputPose>> temp_output;

    bool flag = BatchDetect(all_data, temp_output,params);
    output = temp_output[0];
    return true;
}


bool poseDetect::post_process( std::vector<OutputPose> &output,cv::Vec4d params)
{
    ScopedTiming st(model_name_ + " post_process video", debug_mode_);
    foutput_0 = p_outputs_[0];
    bool find_ = Detect(foutput_0,output,params);

    return find_;

}

void poseDetect::nms_pose(std::vector<BoxInfo> &input_boxes, float NMS_THRESH,vector<int> &nms_result)
{
    std::sort(input_boxes.begin(), input_boxes.end(), [](BoxInfo a, BoxInfo b) { return a.score > b.score; });
    std::vector<float> vArea(input_boxes.size());
    for (int i = 0; i < int(input_boxes.size()); ++i)
    {
        vArea[i] = (input_boxes.at(i).x2 - input_boxes.at(i).x1 + 1)
            * (input_boxes.at(i).y2 - input_boxes.at(i).y1 + 1);
    }
    for (int i = 0; i < int(input_boxes.size()); ++i)
    {
        for (int j = i + 1; j < int(input_boxes.size());)
        {
            float xx1 = std::max(input_boxes[i].x1, input_boxes[j].x1);
            float yy1 = std::max(input_boxes[i].y1, input_boxes[j].y1);
            float xx2 = std::min(input_boxes[i].x2, input_boxes[j].x2);
            float yy2 = std::min(input_boxes[i].y2, input_boxes[j].y2);
            float w = std::max(float(0), xx2 - xx1 + 1);
            float h = std::max(float(0), yy2 - yy1 + 1);
            float inter = w * h;
            float ovr = inter / (vArea[i] + vArea[j] - inter);
            if (ovr >= NMS_THRESH)
            {
                input_boxes.erase(input_boxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            }
            else
            {
                j++;
            }
        }
    }
}


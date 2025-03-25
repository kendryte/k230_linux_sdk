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
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.f
 */

#include "vi_vo.h"
#include "pose_detect.h"

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

    int ori_w = isp_shape_.width;
    int ori_h = isp_shape_.height;
    int width = input_shapes_[0][3];
    int height = input_shapes_[0][2];
    float ratiow = (float)width / ori_w;
    float ratioh = (float)height / ori_h;
    float ratio = ratiow < ratioh ? ratiow : ratioh;
    int new_w = (int)(ratio * ori_w);
    int new_h = (int)(ratio * ori_h);
    float dw = (float)(width - new_w) / 2;
    float dh = (float)(height - new_h) / 2;
    int top = (int)(roundf(dh - 0.1));
    int bottom = (int)(roundf(dh + 0.1));
    int left = (int)(roundf(dw - 0.1));
    int right = (int)(roundf(dw - 0.1));

    params[0] = ratio;
    params[1] = ratio;

    params[2] = left;
    params[3] = top;

    // ai2d_out_tensor
    ai2d_out_tensor_ = get_input_tensor(0);
    // fixed padding resize param
    Utils::padding_resize(isp_shape_, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_, cv::Scalar(114, 114, 114), false);
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

    int ori_w = ori_img.cols;
    int ori_h = ori_img.rows;
    int width = input_shapes_[0][3];
    int height = input_shapes_[0][2];
    float ratiow = (float)width / ori_w;
    float ratioh = (float)height / ori_h;
    float ratio = ratiow < ratioh ? ratiow : ratioh;
    int new_w = (int)(ratio * ori_w);
    int new_h = (int)(ratio * ori_h);
    float dw = (float)(width - new_w) / 2;
    float dh = (float)(height - new_h) / 2;
    int top = (int)(roundf(dh - 0.1));
    int bottom = (int)(roundf(dh + 0.1));
    int left = (int)(roundf(dw - 0.1));
    int right = (int)(roundf(dw - 0.1));

    params[0] = ratio;
    params[1] = ratio;

    params[2] = left;
    params[3] = top;

    Utils::padding_resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(114, 114, 114));
    
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

void nms_pose(std::vector<BoxInfo> &input_boxes, float NMS_THRESH,vector<int> &nms_result)
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
    vector<Rect> boxes;
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
 
            // int left = MAX(int(x - 0.5 *w +0.5), 0);
            // int top = MAX(int(y - 0.5*h + 0.5), 0);
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
            // boxes.push_back(Rect(left, top, int(w + 0.5), int(h + 0.5)));
            boxes.push_back(Rect(left, top, float(w + 0.5), float(h + 0.5)));
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

void poseDetect::DrawPred(cv::Mat& img, std::vector<OutputPose>& results,
              const std::vector<std::vector<unsigned int>> &SKELLTON,
              const std::vector<std::vector<unsigned int>> &KPS_COLORS,
              const std::vector<std::vector<unsigned int>> &LIMB_COLORS)
{
    const int num_point =17;
    for (auto &result:results){
        int  left,top,width, height;
        left = result.box.x;
        top = result.box.y;
        width = result.box.width;
        height = result.box.height;

        std::string label = "person:  " + std::to_string(result.confidence).substr(0, 4);
        int baseLine;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        top = std::max(top, labelSize.height) -10 ;
        putText(img, label, cv::Point(left, top - 30), cv::FONT_HERSHEY_SIMPLEX, 3, cv::Scalar(0,0,255), 5);
        cv::rectangle(img, cv::Point(left, top), cv::Point(left+width, top+height), cv::Scalar(0, 0, 255), 10);
 
        // 连线
        auto &kps = result.kps;
        for (int k=0; k<num_point+2; k++){// 不要设置为>0.5f ,>0.0f显示效果比较好
            // 关键点绘制
            if (k<num_point){
                int kps_x = std::round(kps[k*3]);
                int kps_y = std::round(kps[k*3 + 1]);
                float kps_s = kps[k*3 + 2];
 
                if (kps_s > 0.0f){
                    cv::Scalar kps_color = cv::Scalar(KPS_COLORS[k][0],KPS_COLORS[k][1],KPS_COLORS[k][2]);
                    cv::circle(img, {kps_x, kps_y}, 5, kps_color, 15);
                }
            }
 
            auto &ske = SKELLTON[k];
            int pos1_x = std::round(kps[(ske[0] -1) * 3]);
            int pos1_y = std::round(kps[(ske[0] -1) * 3 + 1]);
 
            int pos2_x = std::round(kps[(ske[1] -1) * 3]);
            int pos2_y = std::round(kps[(ske[1] -1) * 3 + 1]);
 
            float pos1_s = kps[(ske[0] -1) * 3 + 2];
            float pos2_s = kps[(ske[1] -1) * 3 + 2];
 
            if (pos1_s > 0.0f && pos2_s >0.0f){// 不要设置为>0.5f ,>0.0f显示效果比较好
                cv::Scalar limb_color = cv::Scalar(LIMB_COLORS[k][0], LIMB_COLORS[k][1], LIMB_COLORS[k][3]);
                cv::line(img, {pos1_x, pos1_y}, {pos2_x, pos2_y}, limb_color, 10);
            }
 
        }
    }
}

void poseDetect::draw_result_video(cv::Mat& img, std::vector<OutputPose>& results,
              const std::vector<std::vector<unsigned int>> &SKELLTON,
              const std::vector<std::vector<unsigned int>> &KPS_COLORS,
              const std::vector<std::vector<unsigned int>> &LIMB_COLORS)
{
    int osd_width= img.cols;
    int osd_height = img.rows;

    const int num_point =17;
    for (auto &result:results){
        int  left,top,width, height;
        left = result.box.x;
        top = result.box.y;
        width = result.box.width;
        height = result.box.height;

        std::string label = "person:  " + std::to_string(result.confidence).substr(0, 4);
        int baseLine;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        top = std::max(top, labelSize.height) -10 ;

        // 连线
        auto &kps = result.kps;
        for (int k=0; k<num_point+2; k++){// 不要设置为>0.5f ,>0.0f显示效果比较好
            // 关键点绘制
            if (k<num_point){
                int kps_x = std::round(kps[k*3]);
                int kps_y = std::round(kps[k*3 + 1]);
                float kps_s = kps[k*3 + 2];
 
                int kps_x1 =  (float)kps_x / SENSOR_WIDTH * osd_width;
                int kps_y1 =  (float)kps_y / SENSOR_HEIGHT  * osd_height;

                if (kps_s > 0.0f){
                    cv::Scalar kps_color = cv::Scalar(255,KPS_COLORS[k][0],KPS_COLORS[k][1],KPS_COLORS[k][2]);
                    cv::circle(img, {kps_x1, kps_y1}, 5, kps_color, -3);
                }
            }
 
            auto &ske = SKELLTON[k];
            int pos1_x = std::round(kps[(ske[0] -1) * 3]);
            int pos1_y = std::round(kps[(ske[0] -1) * 3 + 1]);

            int pos1_x_ =  (float)pos1_x / SENSOR_WIDTH * osd_width;
            int pos1_y_ =  (float)pos1_y / SENSOR_HEIGHT  * osd_height;

            int pos2_x = std::round(kps[(ske[1] -1) * 3]);
            int pos2_y = std::round(kps[(ske[1] -1) * 3 + 1]);

            int pos2_x_ = (float)pos2_x / SENSOR_WIDTH * osd_width;
            int pos2_y_ =  (float)pos2_y / SENSOR_HEIGHT  * osd_height;
 
            float pos1_s = kps[(ske[0] -1) * 3 + 2];
            float pos2_s = kps[(ske[1] -1) * 3 + 2];
 
            if (pos1_s > 0.0f && pos2_s >0.0f){// 不要设置为>0.5f ,>0.0f显示效果比较好
                cv::Scalar limb_color = cv::Scalar(255,LIMB_COLORS[k][0], LIMB_COLORS[k][1], LIMB_COLORS[k][3]);
                cv::line(img, {pos1_x_, pos1_y_}, {pos2_x_, pos2_y_}, limb_color,3);
            }
        }

        int plot_x1 = int(left * 1.0 / SENSOR_WIDTH * osd_width);
        int plot_y1 = int((top - 30) * 1.0 / SENSOR_HEIGHT  * osd_height);
        int plot_x2 = int((left+width) * 1.0 / SENSOR_WIDTH * osd_width);
        int plot_y2 = (top + height) > kps[kps.size() - 2] ? int((top+height) * 1.0 / SENSOR_HEIGHT  * osd_height) : int(kps[kps.size() - 2] * 1.0 / SENSOR_HEIGHT  * osd_height);

        // std::cout << top + height << " " << kps[kps.size() - 2] << std::endl;
        // putText(img, label, cv::Point(plot_x1, plot_y1), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(255,255,0,0), 4);
        cv::rectangle(img, cv::Point(plot_x1, plot_y1), cv::Point(plot_x2, plot_y2+20), cv::Scalar(255, 255, 0, 0), 4);
    }
}
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

#include "yolo26.h"

Yolo26::Yolo26(char* task_type, char* task_mode, char *kmodel_file, float conf_thres,float mask_thres,std::vector<std::string> labels, FrameSize image_wh,int kp_num, int kp_dim, int debug_mode)
:AIBase(kmodel_file,"Yolo26", debug_mode)
{
    task_type_=task_type;
    task_mode_=task_mode;
    conf_thres_=conf_thres;
    mask_thres_=mask_thres;
    image_wh_=image_wh;
    input_wh_={input_shapes_[0][3], input_shapes_[0][2]};
    labels_=labels;
    label_num_=labels_.size();
    colors=getColorsForClasses(label_num_);
    kp_num_=kp_num;
    kp_dim_=kp_dim;
    kps_size_=kp_num*kp_dim;
    kp_colors=getColorsForClasses(kp_num);
    max_box_num_=50;
    box_num_=300;
    debug_mode_=debug_mode;
    model_input_tensor_=get_input_tensor(0);
    if(strcmp(task_type_,"classify")==0){
        Utils::center_crop_resize_set(image_wh_,input_wh_,ai2d_builder_);
    }
    else if(strcmp(task_type_,"detect")==0){
        box_feature_len_=6;
        Utils::padding_resize_one_side_set(image_wh_,input_wh_,ai2d_builder_, cv::Scalar(114, 114, 114));
    }
    else if(strcmp(task_type_,"segment")==0){
        box_feature_len_=38;
        Utils::padding_resize_one_side_set(image_wh_,input_wh_,ai2d_builder_, cv::Scalar(114, 114, 114));
    }
    else if(strcmp(task_type_,"obb")==0){
        box_feature_len_=7;
        Utils::padding_resize_one_side_set(image_wh_,input_wh_,ai2d_builder_, cv::Scalar(114, 114, 114));
    }
    else if(strcmp(task_type_,"pose")==0){
        box_feature_len_=kps_size_+6;
        Utils::padding_resize_one_side_set(image_wh_,input_wh_,ai2d_builder_, cv::Scalar(114, 114, 114));
    }
    else{
        std::cerr << "不支持该任务类型: " << task_type_ << std::endl;
        exit(EXIT_FAILURE);
    }

}
    
Yolo26::~Yolo26()
{
}

void Yolo26::pre_process(runtime_tensor &input_tensor)
{
    ScopedTiming st("Yolo26::pre_process", debug_mode_);
    ai2d_builder_->invoke(input_tensor,model_input_tensor_).expect("error occurred in ai2d running");
}

void Yolo26::inference()
{
    this->run();
    this->get_output();
}

void Yolo26::post_process(std::vector<YOLOBbox> &yolo_results)
{
    ScopedTiming st("Yolo26::post_process", debug_mode_);
    yolo_results.clear();
    if(strcmp(task_type_,"classify")==0){
        float* output0 = p_outputs_[0];
        YOLOBbox res;
        if(label_num_ > 2){
            float sum = 0.0;
            for (int i = 0; i < label_num_; i++){
                sum += exp(output0[i]);
            }
            int max_index;
            for (int i = 0; i < label_num_; i++)
            {
                output0[i] = exp(output0[i]) / sum;
            }
            max_index = std::max_element(output0,output0+label_num_) - output0; 
            if (output0[max_index] >= conf_thres_)
            {
                res.index = max_index;
                res.confidence = output0[max_index];
                yolo_results.push_back(res);
            }
        }
        else
        {
            float pred = sigmoid(output0[0]);
            if (pred > conf_thres_)
            {
                res.index = 0;
                res.confidence = pred;
            }
            else{
                res.index = 1;
                res.confidence = 1-pred;
            }
            yolo_results.push_back(res);
        }
    }
    else if(strcmp(task_type_,"detect")==0){
        float ratiow = (float)input_wh_.width / image_wh_.width;
        float ratioh = (float)input_wh_.height / image_wh_.height;
        float ratio = ratiow < ratioh ? ratiow : ratioh;
        for(int i=0;i<box_num_;i++){
            float* vec=p_outputs_[0]+i*box_feature_len_;
            float box[4]={vec[0],vec[1],vec[2],vec[3]};
            float score=vec[4];
            float max_class_index=vec[5];
            if(score>conf_thres_){
                YOLOBbox bbox;
                float x_1=box[0]/ratio*1.0;
                float y_1=box[1]/ratio*1.0;
                float x_2=box[2]/ratio*1.0;
                float y_2=box[3]/ratio*1.0;
                int x=int(MAX(x_1,0));
                int y=int(MAX(y_1,0));
                int w=int(x_2-x_1);
                int h=int(y_2-y_1);
                if (w <= 0 || h <= 0) { continue; }
                bbox.box=cv::Rect(x,y,w,h);
                bbox.confidence=score;
                bbox.index=max_class_index;
                yolo_results.push_back(bbox);
            }
        }
    }
    else if(strcmp(task_type_,"segment")==0){
        float ratiow = input_wh_.width / (image_wh_.width*1.0);
        float ratioh = input_wh_.height / (image_wh_.height*1.0);
        float ratio = ratiow < ratioh ? ratiow : ratioh;
        int new_w=int(image_wh_.width*ratio);
        int new_h=int(image_wh_.height*ratio);
        int pad_w=input_wh_.width-new_w>0?input_wh_.width-new_w:0;
        int pad_h=input_wh_.height-new_h>0?input_wh_.height-new_h:0;
       
        int mask_w=input_wh_.width/4;
        int mask_h=input_wh_.height/4;
        cv::Mat protos=cv::Mat(32,mask_w*mask_h,CV_32FC1,p_outputs_[1]);

        for(int i=0;i<box_num_;i++){
            float* vec=p_outputs_[0]+i*box_feature_len_;
            float box[4]={vec[0],vec[1],vec[2],vec[3]};
            float score=vec[4];
		    float class_id=vec[5];
            if(score>conf_thres_){
                YOLOBbox bbox;
                float x_1=box[0]/ratio*1.0;
                float y_1=box[1]/ratio*1.0;
                float x_2=box[2]/ratio*1.0;
                float y_2=box[3]/ratio*1.0;
                int x=int(MAX(x_1,0));
                int y=int(MAX(y_1,0));
                int w=int(x_2-x_1);
                int h=int(y_2-y_1);
                if (w <= 0 || h <= 0) { continue; }
                bbox.box=cv::Rect(x,y,w,h);
                bbox.confidence=score;
                bbox.index=class_id;
                bbox.mask=cv::Mat(1, 32, CV_32F, vec + 6);
                yolo_results.push_back(bbox);
            }

        }
        
        for (int i = 0; i < yolo_results.size(); i++)
        {
            cv::Mat mask_box = yolo_results[i].mask * protos;
            cv::Mat mask_box_(mask_h, mask_w, CV_32FC1, mask_box.data);
            cv::Rect roi(0, 0, mask_w - int(pad_w * (mask_w / float(input_wh_.width))),
                         mask_h - int(pad_h * (mask_h / float(input_wh_.height))));

            cv::Mat dest;
            cv::exp(-mask_box_, dest);
            dest = 1.0 / (1.0 + dest);
            dest = dest(roi);
            yolo_results[i].mask = dest;
        }
    }
    else if(strcmp(task_type_,"obb")==0){
        float ratiow = (float)input_wh_.width / image_wh_.width;
        float ratioh = (float)input_wh_.height / image_wh_.height;
        float ratio = ratiow < ratioh ? ratiow : ratioh;
        
        for(int i=0;i<box_num_;i++){
            float* vec=p_outputs_[0]+i*box_feature_len_;
            float box[4]={vec[0],vec[1],vec[2],vec[3]};
            float score=vec[4];
            float class_id=vec[5];
            float angle=vec[6];
            if(score>conf_thres_){
                YOLOBbox bbox;
                float x_=box[0]/ratio*1.0;
                float y_=box[1]/ratio*1.0;
                float w_=box[2]/ratio*1.0;
                float h_=box[3]/ratio*1.0;
                int x=int(MAX(x_,0));
                int y=int(MAX(y_,0));
                int w=int(w_);
                int h=int(h_);
                if (w <= 0 || h <= 0) { continue; }
                bbox.box=cv::Rect(x,y,w,h);
                bbox.confidence=score;
                bbox.angle=angle;
                bbox.index=class_id;
                yolo_results.push_back(bbox);
            }

        }
    }
    else if(strcmp(task_type_,"pose")==0){
        float ratiow = (float)input_wh_.width / image_wh_.width;
        float ratioh = (float)input_wh_.height / image_wh_.height;
        float ratio = ratiow < ratioh ? ratiow : ratioh;
        for(int i=0;i<box_num_;i++){
            float* vec=p_outputs_[0]+i*box_feature_len_;
            float box[4]={vec[0],vec[1],vec[2],vec[3]};
            float score=vec[4];
            float max_class_index=vec[5];
            float* kps = vec+6;
            if(score>conf_thres_){
                YOLOBbox bbox;
                float x_1=box[0]/ratio*1.0;
                float y_1=box[1]/ratio*1.0;
                float x_2=box[2]/ratio*1.0;
                float y_2=box[3]/ratio*1.0;
                int x=int(MAX(x_1,0));
                int y=int(MAX(y_1,0));
                int w=int(x_2-x_1);
                int h=int(y_2-y_1);
                if (w <= 0 || h <= 0) { continue; }
                bbox.box=cv::Rect(x,y,w,h);
                bbox.confidence=score;
                bbox.kp_num=kp_num_;
                bbox.kp_dim=kp_dim_;
                bbox.index=0;
                bbox.kps = (float*)malloc(kps_size_ * sizeof(float));
                // 关键点坐标还原到原始图像尺寸
                for(int k=0;k<kp_num_;k++){
                    if(kp_dim_==3){
                        bbox.kps[k*3+0] = kps[k*3+0] / ratio;
                        bbox.kps[k*3+1] = kps[k*3+1] / ratio;
                        bbox.kps[k*3+2] = kps[k*3+2];
                    }
                    else if(kp_dim_==2){
                        bbox.kps[k*2+0] = kps[k*2+0] / ratio;
                        bbox.kps[k*2+1] = kps[k*2+1] / ratio;
                    }
                    else{
                        memcpy(bbox.kps, kps, kps_size_ * sizeof(float));
                        break;
                    }
                }
                yolo_results.push_back(bbox);
            }
        }
    }
}

void Yolo26::draw_results(cv::Mat &draw_frame,std::vector<YOLOBbox> &yolo_results)
{
    ScopedTiming st("Yolo26::draw_results", debug_mode_);
    if(strcmp(task_type_,"classify")==0){
        if(yolo_results.size()>0){
            string text=labels_[yolo_results[0].index]+" score:"+std::to_string(yolo_results[0].confidence);
            cv::putText(draw_frame, text, cv::Point(50,50), cv::FONT_HERSHEY_DUPLEX, 1, colors[yolo_results[0].index], 2, 0);
        }
    }
    else if(strcmp(task_type_,"detect")==0){
        int w_=draw_frame.cols;
        int h_=draw_frame.rows;
        int res_size=MIN(yolo_results.size(),max_box_num_);
        for(int i=0;i<res_size;i++){
            YOLOBbox box_=yolo_results[i];
            cv::Rect box=box_.box;
            int idx=box_.index;
            float score=box_.confidence;
            int x=int(box.x*float(w_)/image_wh_.width);
            int y=int(box.y*float(h_)/image_wh_.height);
            int w=int(box.width*float(w_)/image_wh_.width);
            int h=int(box.height*float(h_)/image_wh_.height);
            int x_right = x + w;
            int y_bottom = y + h;
            if (x_right > w_)
            {
                w = w_ - x;
            }
            if (y_bottom > h_)
            {
                h = h_ - y;
            }
            cv::Rect new_box(x,y,w,h);
            cv::rectangle(draw_frame, new_box, colors[idx], 2, 8);
            cv::putText(draw_frame, labels_[idx]+" "+std::to_string(score), cv::Point(MIN(new_box.x + 5,w_), MAX(new_box.y - 10,0)), cv::FONT_HERSHEY_DUPLEX, 1, colors[idx], 2, 0);
        }
    }
    else if(strcmp(task_type_,"segment")==0){
        int w_=draw_frame.cols;
        int h_=draw_frame.rows;
        int res_size=MIN(yolo_results.size(),max_box_num_);
        for(int i=0;i<res_size;i++){
            YOLOBbox box_=yolo_results[i];
            cv::Rect box=box_.box;
            int idx=box_.index;
            float score=box_.confidence;
            int x=int(box.x*float(w_)/image_wh_.width);
            int y=int(box.y*float(h_)/image_wh_.height);
            int w=int(box.width*float(w_)/image_wh_.width);
            int h=int(box.height*float(h_)/image_wh_.height);
            int x_right = x + w;
            int y_bottom = y + h;
            if (x_right > w_)
            {
                w = w_ - x;
            }
            if (y_bottom > h_)
            {
                h = h_ - y;
            }
            cv::Rect new_box(x,y,w,h);
            cv::rectangle(draw_frame, new_box, colors[idx], 2, 8);
            cv::putText(draw_frame, labels_[idx]+" "+std::to_string(score), cv::Point(MIN(new_box.x + 5,w_), MAX(new_box.y - 10,0)), cv::FONT_HERSHEY_DUPLEX, 1, colors[idx], 2, 0);
            
            cv::Mat mask=box_.mask;
            cv::Mat mask_d;
            cv::resize(mask,mask_d,cv::Size(w_,h_),cv::INTER_NEAREST);
            mask_d=mask_d(new_box) > mask_thres_;
            draw_frame(new_box).setTo(colors[idx],mask_d);
        }
    }
    else if(strcmp(task_type_,"obb")==0){
        int w_=draw_frame.cols;
        int h_=draw_frame.rows;
        int res_size=MIN(yolo_results.size(),max_box_num_);
        for(int i=0;i<res_size;i++){
            YOLOBbox box_=yolo_results[i];
            cv::Rect box=box_.box;
            int idx=box_.index;
            float score=box_.confidence;
            float angle=box_.angle;
            std::vector<std::pair<int, int>> corners=yolo26_calculate_obb_corners(box.x, box.y,box.width, box.height, angle);
            int x_0=int(corners[0].first*float(w_)/image_wh_.width);
            int y_0=int(corners[0].second*float(h_)/image_wh_.height);
            int x_1=int(corners[1].first*float(w_)/image_wh_.width);
            int y_1=int(corners[1].second*float(h_)/image_wh_.height);
            int x_2=int(corners[2].first*float(w_)/image_wh_.width);
            int y_2=int(corners[2].second*float(h_)/image_wh_.height);
            int x_3=int(corners[3].first*float(w_)/image_wh_.width);
            int y_3=int(corners[3].second*float(h_)/image_wh_.height);
            cv::line(draw_frame, cv::Point(x_0, y_0), cv::Point(x_1, y_1), colors[idx], 2);
            cv::line(draw_frame, cv::Point(x_1, y_1), cv::Point(x_2, y_2), colors[idx], 2);
            cv::line(draw_frame, cv::Point(x_2, y_2), cv::Point(x_3, y_3), colors[idx], 2);
            cv::line(draw_frame, cv::Point(x_3, y_3), cv::Point(x_0, y_0), colors[idx], 2);
            cv::putText(draw_frame, std::to_string(idx), cv::Point(x_0 , y_0 - 10), cv::FONT_HERSHEY_DUPLEX, 1, colors[idx], 2, 0);
        }
    }
    else if(strcmp(task_type_,"pose")==0){
        int w_=draw_frame.cols;
        int h_=draw_frame.rows;
        int res_size=MIN(yolo_results.size(),max_box_num_);
        for(int i=0;i<res_size;i++){
            YOLOBbox box_=yolo_results[i];
            cv::Rect box=box_.box;
            int idx=box_.index;
            float score=box_.confidence;
            int x=int(box.x*float(w_)/image_wh_.width);
            int y=int(box.y*float(h_)/image_wh_.height);
            int w=int(box.width*float(w_)/image_wh_.width);
            int h=int(box.height*float(h_)/image_wh_.height);
            int x_right = x + w;
            int y_bottom = y + h;
            if (x_right > w_)
            {
                w = w_ - x;
            }
            if (y_bottom > h_)
            {
                h = h_ - y;
            }
            cv::Rect new_box(x,y,w,h);
            cv::rectangle(draw_frame, new_box, colors[0], 2, 8);
            cv::putText(draw_frame, "person "+std::to_string(score), cv::Point(MIN(new_box.x + 5,w_), MAX(new_box.y - 10,0)), cv::FONT_HERSHEY_DUPLEX, 1, colors[0], 2, 0);
            if(box_.kps != nullptr){
                for(int j=0;j<kp_num_;j++){
                    int kp_x, kp_y;
                    if(kp_dim_==3){
                        kp_x=int(box_.kps[j*3+0]*float(w_)/image_wh_.width);
                        kp_y=int(box_.kps[j*3+1]*float(h_)/image_wh_.height);
                        float kp_conf=box_.kps[j*3+2];
                        if(kp_conf < 0.5f) continue;
                    }
                    else if(kp_dim_==2){
                        kp_x=int(box_.kps[j*2+0]*float(w_)/image_wh_.width);
                        kp_y=int(box_.kps[j*2+1]*float(h_)/image_wh_.height);
                    }
                    else{
                        continue;
                    }
                    if(kp_x>=0 && kp_x<w_ && kp_y>=0 && kp_y<h_)
                        cv::circle(draw_frame, cv::Point(kp_x, kp_y), 5, kp_colors[j], -1, 8);
                }
            }
        }
    }
}

float Yolo26::fast_exp(float x)
{
    union {
        uint32_t i;
        float f;
    } v{};
    v.i = (1 << 23) * (1.4426950409 * x + 126.93490512f);
    return v.f;
}

float Yolo26::sigmoid(float x)
{
    return 1.0f / (1.0f + fast_exp(-x));
}

template<typename T>
T clamp(T value, T low, T high) {
    return (value < low) ? low : (value > high) ? high : value;
}


std::vector<std::pair<int, int>> Yolo26::yolo26_calculate_obb_corners(float x_center, float y_center, float width, float height, float angle) {
    float cos_angle = std::cos(angle);  // 计算余弦
    float sin_angle = std::sin(angle);  // 计算正弦
    float dx = width / 2.0f;
    float dy = height / 2.0f;

    std::vector<std::pair<int, int>> corners = {
        { static_cast<int>(x_center + cos_angle * dx - sin_angle * dy),
          static_cast<int>(y_center + sin_angle * dx + cos_angle * dy) },

        { static_cast<int>(x_center - cos_angle * dx - sin_angle * dy),
          static_cast<int>(y_center - sin_angle * dx + cos_angle * dy) },

        { static_cast<int>(x_center - cos_angle * dx + sin_angle * dy),
          static_cast<int>(y_center - sin_angle * dx - cos_angle * dy) },

        { static_cast<int>(x_center + cos_angle * dx + sin_angle * dy),
          static_cast<int>(y_center + sin_angle * dx - cos_angle * dy) }
    };

    return corners;
}

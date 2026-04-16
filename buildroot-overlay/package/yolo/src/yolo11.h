/* Copyright (c) 2022, Canaan Bright Sight Co., Ltd
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

#ifndef _YOLO11_H
#define _YOLO11_H

#include "utils.h"
#include "ai_base.h"


/**
 * @brief Yolo11
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class Yolo11 : public AIBase
{
    public:

    /**
    * @brief Yolo11构造函数，加载kmodel,并初始化kmodel输入、输出
    * @param kmodel_file kmodel文件路径
    * @param conf_thres 多目标检测conf_thres
    * @param nms_thres   多目标检测nms阈值
    * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
    * @return None
    */
    Yolo11(char* task_type,char* task_mode,char *kmodel_file, float conf_thres, float nms_thres,float mask_thres,std::vector<std::string> labels,FrameSize image_wh,int kp_num = 17, int kp_dim = 3, int debug_mode = 0);
    
    /**
    * @brief Yolo11析构函数
    * @return None
    */
    ~Yolo11();

    void pre_process(runtime_tensor &input_tensor);

    /**
    * @brief kmodel推理
    * @return None
    */
    void inference();

    void post_process(std::vector<YOLOBbox> &yolo_results);

    void draw_results(cv::Mat &draw_frame,std::vector<YOLOBbox> &yolo_results);

    private:

    void yolo11_nms(std::vector<YOLOBbox> &bboxes,  float confThreshold, float nmsThreshold, std::vector<int> &indices);

    void yolo11_rotate_nms(std::vector<YOLOBbox> &bboxes, float confThreshold, float nmsThreshold,std::vector<int> &indices);
    
    float yolo11_iou_calculate(cv::Rect &rect1, cv::Rect &rect2);

    std::vector<std::pair<int, int>> yolo11_calculate_obb_corners(float x_center, float y_center, float width, float height, float angle);

    float fast_exp(float x);

    float sigmoid(float x);

    std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
    runtime_tensor model_input_tensor_;             // ai2d输出tensor

    char* task_type_;
    char* task_mode_;
    // 标签列表
    std::vector<std::string> labels_;
    int label_num_;
    std::vector<cv::Scalar> colors;
    FrameSize image_wh_;
    FrameSize input_wh_;
    // 置信度阈值
    float conf_thres_;
    // nms阈值
    float nms_thres_;
    // 分割任务使用的mask阈值
    float mask_thres_;
    // 检测框的总数
    int box_num_;
    int max_box_num_;
    // 每个检测框的特征维度
    int box_feature_len_;
    // 关键点数量
    int kp_num_;
    // 关键点维度
    int kp_dim_;
    int kps_size_;
    std::vector<cv::Scalar> kp_colors;
    // kmodel的输出处理结果
    float *output_det_;
    int debug_mode_;
};
#endif
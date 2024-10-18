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

#ifndef _OBJECT_DETECT
#define _OBJECT_DETECT

#include <iostream>
#include <vector>
#include "utils.h"
#include "ai_base.h"

using namespace std;
using namespace cv;

/**
 * @brief 检测框信息
 */
typedef struct BoxInfo
{
    float x1;   // 检测框左上顶点x坐标
    float y1;   // 检测框左上顶点y坐标
    float x2;   // 检测框右下顶点x坐标
    float y2;   // 检测框右下顶点y坐标
    float score;    // 检测框的得分
    int label;  // 检测框的标签
} BoxInfo;


/**
 * @brief 检测任务
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class objectDetect: public AIBase
{
    public:

        /** 
        * for image
        * @brief objectDetect 构造函数，加载kmodel,并初始化kmodel输入、输出、类阈值和NMS阈值
        * @param kmodel_file kmodel文件路径
        * @param obj_thresh 检测框阈值
        * @param nms_thresh NMS阈值
        * @param debug_mode 0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */

        objectDetect(const char *kmodel_file, float obj_thresh,float nms_thresh,  const int debug_mode);

        /** 
        * for video
        * @brief objectDetect 构造函数，加载kmodel,并初始化kmodel输入、输出、类阈值和NMS阈值
        * @param kmodel_file kmodel文件路径
        * @param obj_thresh 检测框阈值
        * @param nms_thresh NMS阈值
        * @param isp_shape   isp输入大小（chw）
        * @param debug_mode 0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */
        objectDetect(const char *kmodel_file, float obj_thresh,float nms_thresh, FrameCHWSize isp_shape, const int debug_mode);
        /** 
        * @brief  objectDetect 析构函数
        * @return None
        */
        ~objectDetect();

        /**
         * @brief 图片预处理（ai2d for image）
         * @param ori_img 原始图片
         * @return None
         */
        void pre_process(cv::Mat ori_img);
        
        /**
        * @brief 视频流预处理（ai2d for isp）
        * @param img_data 当前视频帧数据
        * @return None
        */
        void pre_process(runtime_tensor& img_data);

        /**
         * @brief kmodel推理
         * @return None
         */
        void inference();

        /** 
        * @brief postprocess 函数，对输出解码后的结果，进行NMS处理
        * @param frame_size 帧大小
        * @param result   所有候选检测框
        * @return None
        */
        void post_process(FrameSize frame_size,std::vector<BoxInfo> &result);

        std::vector<std::string> labels  // 类别标签
        {
            "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck",
            "boat", "traffic light", "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat",
            "dog", "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe",
            "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard",
            "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard", "tennis racket", "bottle",
            "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
            "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake",
            "chair", "couch", "potted plant", "bed", "dining table", "toilet", "tv", "laptop",
            "mouse", "remote", "keyboard", "cell phone", "microwave", "oven", "toaster", "sink",
            "refrigerator", "book", "clock", "frame_sizevase", "scissors", "teddy bear", "hair drier", "toothbrush"
        };

    private:
        float obj_thresh_;  // 检测框阈值
        float nms_thresh_;  // NMS阈值
 
        int anchors_num_ = 3;  // 锚框个数
        int classes_num_ = 80;   // 类别数
        int channels_ = anchors_num_ * (5 + classes_num_);  // 通道数
        float anchors_0_[3][2] = { { 10, 13 }, { 16, 30 }, { 33, 23 } };  // 第一组锚框
        float anchors_1_[3][2] = { { 30, 61 }, { 62, 45 }, { 59, 119 } };  // 第二组锚框
        float anchors_2_[3][2] = { { 116, 90 }, { 156, 198 }, { 373, 326 } }; // 第三组锚框

        std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
        runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
        runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
        FrameCHWSize isp_shape_;                     // isp对应的地址大小

};
#endif

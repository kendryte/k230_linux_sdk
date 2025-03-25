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

#ifndef _POSE_ACTION_
#define _POSE_ACTION_

#include <iostream>
#include <vector>
#include "utils.h"
#include "ai_base.h"
#include "pose_detect.h"

/**
 * @brief 关键点信息
 */
struct KKeyPoint
{
    cv::Point2f p;  // 关键点
    float prob;     // 关键点概率值
};


/** 
 * @brief action帮助信息
 */
struct action_helper {
  bool mark = false;  // 是否标记
  int action_count = 0;  // action计数
  int latency = 0;  // 延迟次数
};


/**
 * @brief PoseAction 类
 * 封装了PoseAction类常用的函数，包括清除动作计数函数、获取动作计数函数、获取xy比例函数、获取xyhigher函数、多种动作检查函数等
 */
class PoseAction
{
    public:

        /** 
        * @brief 获取“深蹲”次数
        * @param kpts_sframe  每帧关键点
        * @param recid  某种动作记录编号
        * @param thres_conf  关键点阈值
        * @return 深蹲次数
        */
        static int check_deep_down(std::vector<KKeyPoint> &kpts_sframe, int recid,float thres_conf);

        /** 
        * @brief 单一动作检查
        * @param results_kpts  关键点结果
        * @param thres_conf  关键点阈值
        * @param actionid  动作编号
        * @param recid  某种动作记录编号
        * @return 动作次数
        */ 
        static int single_action_check(std::vector<KKeyPoint> &results_kpts, float thres_conf, int actionid, int recid);

        /** 
        * @brief 对所有Pose结果（含boxes）进行NMS处理
        * @param input_boxes      所有boxes
        * @param NMS_THRESH       NMS阈值
        * @param nms_result       NMS处理之后的结果
        * @return None
        */
        static void nms_pose(std::vector<BoxInfo> &input_boxes, float NMS_THRESH,std::vector<int> &nms_result);

        /** 
        * @brief 绘制关键点
        * @param img           原图
        * @param frame_size  分辨率  
        * @param results       关键点结果
        * @param SKELETON      骨骼信息，常量
        * @param KPS_COLORS    关键点颜色信息，常量
        * @param LIMB_COLORS   肢体颜色信息，常量
        * @return None
        */
        static void DrawPred_video(cv::Mat& img,FrameSize frame_size, std::vector<OutputPose>& results,
                    const std::vector<std::vector<unsigned int>> &SKELLTON,
                    const std::vector<std::vector<unsigned int>> &KPS_COLORS,
                    const std::vector<std::vector<unsigned int>> &LIMB_COLORS,float thres_conf );

        static void action_count( cv::Mat& image, std::vector<KKeyPoint> keypoints, float thresh);
};

const std::vector<std::vector<unsigned int>> KPS_COLORS =    // 关键点颜色
        {{0,   255, 0},
         {0,   255, 0},
         {0,   255, 0},
         {0,   255, 0},
         {0,   255, 0},
         {255, 128, 0},
         {255, 128, 0},
         {255, 128, 0},
         {255, 128, 0},
         {255, 128, 0},
         {255, 128, 0},
         {51,  153, 255},
         {51,  153, 255},
         {51,  153, 255},
         {51,  153, 255},
         {51,  153, 255},
         {51,  153, 255}};
 
const std::vector<std::vector<unsigned int>> SKELETON = {{16, 14},  // 骨骼信息
                                                         {14, 12},
                                                         {17, 15},
                                                         {15, 13},
                                                         {12, 13},
                                                         {6,  12},
                                                         {7,  13},
                                                         {6,  7},
                                                         {6,  8},
                                                         {7,  9},
                                                         {8,  10},
                                                         {9,  11},
                                                         {2,  3},
                                                         {1,  2},
                                                         {1,  3},
                                                         {2,  4},
                                                         {3,  5},
                                                         {4,  6},
                                                         {5,  7}};
 
const std::vector<std::vector<unsigned int>> LIMB_COLORS = {{51,  153, 255},  // 肢体信息
                                                            {51,  153, 255},
                                                            {51,  153, 255},
                                                            {51,  153, 255},
                                                            {255, 51,  255},
                                                            {255, 51,  255},
                                                            {255, 51,  255},
                                                            {255, 128, 0},
                                                            {255, 128, 0},
                                                            {255, 128, 0},
                                                            {255, 128, 0},
                                                            {255, 128, 0},
                                                            {0,   255, 0},
                                                            {0,   255, 0},
                                                            {0,   255, 0},
                                                            {0,   255, 0},
                                                            {0,   255, 0},
                                                            {0,   255, 0},
                                                            {0,   255, 0}};

#endif
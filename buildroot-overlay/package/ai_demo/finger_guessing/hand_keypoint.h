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
#ifndef _HAND_KEYPOINT_H
#define _HAND_KEYPOINT_H

#include <iostream>
#include <vector>

#include "utils.h"
#include "ai_base.h"
#include "hand_detection.h"

/**
 * @brief 手部关键点检测
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class HandKeypoint: public AIBase
{
public:
    /**
     * @brief HandKeypoint构造函数，加载kmodel,并初始化kmodel输入、输出
     * @param kmodel_file kmodel文件路径
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    HandKeypoint(const char *kmodel_file,const int debug_mode=0);

    /**
     * @brief HandKeypoint构造函数，加载kmodel,并初始化kmodel输入、输出
     * @param kmodel_file kmodel文件路径
     * @param isp_shape   isp输入大小（chw）
     * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
     * @return None
     */
    // for video
    HandKeypoint(const char *kmodel_file, FrameCHWSize isp_shape, const int debug_mode);

    /**
     * @brief HandKeypoint析构函数
     * @return None
     */
    ~HandKeypoint();

    /**
     * @brief 图片预处理
     * @param ori_img 原始图片
     * @param bbox 原始手掌检测框位置
     * @return None
     */
    void pre_process(cv::Mat ori_img, Bbox &bbox);

    /**
     * @brief 视频流预处理（ai2d for isp）
     * @param img_data 当前视频帧数据
     * @param bbox 原始手掌检测框位置
     * @return None
     */
    void pre_process(runtime_tensor& img_data, Bbox &bbox);

    /**
     * @brief kmodel推理
     * @return None
     */
    void inference();

    /**
     * @param bbox 手掌检测框信息
     * @param results 手掌关键点在原图上的坐标
     * @return None
     */
    void post_process(Bbox &bbox);

    /**
     * @brief 将手部关键点画到原图
     * @param img 原图
     * @param text 显示的文本信息
     * @param bbox 手部关键点输入框在原图的显示
     * @param pic_mode    ture(原图片)，false(osd)
     * @return None
     */
    void draw_keypoints(cv::Mat &img, std::string text, Bbox &bbox, bool pic_mode);


    std::vector<int> results;

    /**
     * @brief 求两个向量之间的夹角
     * @param v1 向量1
     * @param v2 向量2
     * @return 夹角
     */
    double vector_2d_angle(std::vector<double> v1, std::vector<double> v2);

    /**
     * @brief 返回5个手指的角度
     * @return 5个手指的角度
     */
    std::vector<double> hand_angle();

    /**
     * @brief 根据手指角度判断手势类别
     * @param angle_list 5个手指的角度
     * @return 手势类别
     */
    std::string h_gesture(std::vector<double> angle_list);

    /**
     * @brief 读取2进制文件
     * @param file_name 文件路径
     * @param outi      读取到的数据
     * @return None
     */
    static void read_binary_file_bin(std::string file_name,unsigned char *outi);
    
    /**
     * @brief 将bin文件转为Mat
     * @param bin_data_path 文件路径
     * @param mat_width     Mat宽
     * @param mat_height    Mat高
     * @param image_rgb    转换之后的Mat数据
     * @return None
     */
    static void bin_2_mat(std::string bin_data_path, int mat_width, int mat_height, cv::Mat &image_rgb);

private:
    std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
    runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
    runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
    FrameCHWSize isp_shape_;                     // isp对应的地址大小
};
#endif

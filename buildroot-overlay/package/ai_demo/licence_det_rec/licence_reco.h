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

#ifndef _LICENCE_RECO_H
#define _LICENCE_RECO_H

#include "utils.h"
#include "ai_base.h"
#include "licence_det.h"

using namespace std;
using namespace cv;

/**
 * @brief 车牌识别
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class LicenceReco : public AIBase
{
    public:
        /**
        * @brief LicenceReco构造函数，加载kmodel,并初始化kmodel输入、输出和车牌识别字典大小
        * @param kmodel_file kmodel文件路径
        * @param dict_size   车牌识别字典大小
        * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */
        LicenceReco(const char *kmodel_file, int dict_size, const int debug_mode = 1);

        /**
        * @brief LicenceReco析构函数
        * @return None
        */
        ~LicenceReco();

        /**
        * @brief 图片预处理
        * @param ori_img 原始图片
        * @return None
        */
        void pre_process(cv::Mat ori_img);

        /**
        * @brief kmodel推理
        * @return None
        */
        void inference();

        /**
        * @brief kmodel推理结果后处理
        * @param results 后处理之后的字符的十六进制集合
        * @return None
        */
        void post_process(vector<unsigned char> &results);

        /**
         * @brief 透射变换后crop
         * @param src         原始图像
         * @param dst         处理后的图像
         * @param b           车牌检测出的四边形角点对
         * @param vtd         排序后的四边形角点对
         * @return None
         */
        void warppersp(cv::Mat src, cv::Mat& dst, BoxPoint b, std::vector<Point2f>& vtd);

        /**
         * @brief 单图 写文本
         * @param x_offset    作图起始横坐标
         * @param y_offset    作图起始纵坐标
         * @param image       作图的图像
         * @param vec16       文字的16进制表示
         * @return None
         */
        void draw_text_img(int x_offset,int y_offset,cv::Mat& image,vector<unsigned char> vec16);

        /**
         * @brief video 写文本
         * @param x_offset              作图起始横坐标
         * @param y_offset              作图起始纵坐标
         * @param image                 作图的图像
         * @param vec16                 文字的16进制表示
         * @return None
         */
        static void draw_text_video(float x_offset,float y_offset,cv::Mat& src_img,vector<unsigned char> vec16);

    private:
        std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
        runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
        runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
        FrameCHWSize isp_shape_;                     // isp对应的地址大小

        int input_width;        //车牌识别model输入高
        int input_height;       //车牌识别model输入宽
        
        int dict_size;          //车牌识别字典大小

        int flag;               //车牌用于控制是否带空格

        float *output;          //车牌后处理的输入
};
#endif
/* Copyright (c) 2025, Canaan Bright Sight Co., Ltd
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

#ifndef _OCR_BOX_H
#define _OCR_BOX_H

#include "ai_utils.h"
#include "ai_base.h"
#include "clipper.hpp"
#include "parse_args.h"
#include "text_paint.h"

using namespace ClipperLib;
using namespace cv;

/**
 * @brief ocr检测结果结构
 */
typedef struct ocr_det_res
{
    float meanx;
    float meany;
	Point2f vertices[4];
    Point2f ver_src[4];
    float score;
}ocr_det_res;

/**
 * @brief ocr检测
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class OCRBox : public AIBase
{
    public:
        /**
        * @brief AnchorBaseDet构造函数，加载kmodel,并初始化kmodel输入、输出和ocr检测阈值
        * @param args        构建对象需要的参数，config.json文件（包含检测阈值，kmodel路径等）
        * @param isp_shape   输入图像大小（chw）
        * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */
        OCRBox(config_args args, FrameCHWSize isp_shape, const int debug_mode);
        
        /**
        * @brief OCRBox析构函数
        * @return None
        */
        ~OCRBox();

        /**
        * @brief 模型前处理函数，将输入图像转换为模型输入格式
        * @param input_tensor 模型输入tensor
        * @return None
        */
        void pre_process(runtime_tensor &input_tensor);

        /**
        * @brief kmodel推理
        * @return None
        */
        void inference();

        /**
        * @brief kmodel推理结果后处理
        * @param results 后处理之后的基于原始图像的{检测点对{四边形的四个角点}}集合
        * @return None
        */
        void post_process(vector<ocr_det_res> &results);

        /**
        * @brief 绘制检测结果
        * @param draw_frame 绘制结果的图像
        * @param results 基于原始图像的{检测点对{四边形的四个角点}}集合
        * @param ocrrec_results OCR识别字符串结果
        * @param frame_size 原始图像宽高
        * @param writepen 字体渲染器
        * @return None
        */
        static void draw_result(cv::Mat &draw_frame, vector<ocr_det_res> &results, vector<string> &ocrrec_results, FrameCHWSize frame_size,TextRenderer &writepen);

        /**
        * @brief 透视变换操作
        * @param src 输入图像
        * @param dst 输出图像
        * @param b 轮廓里的最小外接矩形-点的集合
        * @param vtd 变换后的点集
        * @return None
        */
        void warppersp(cv::Mat &src, cv::Mat &dst, ocr_det_res &b, std::vector<cv::Point2f> &vtd);

    private:
        /**
        * @brief 获取box操作
        * @param b 轮廓里的最小外接矩形-点的集合
        * @param contours 轮廓-点的集合
        * @return None
        */
        void getBox(ocr_det_res& b, vector<Point> contours);

        /**
        * @brief 获取box分数操作
        * @param src 模型输出转换成的mat
        * @param contours 轮廓-点的集合
        * @param b 轮廓里的最小外接矩形-点的集合
        * @return float 输出box分数值
        */
        float boxScore(Mat src, vector<Point> contours, ocr_det_res& b);
        
        /**
        * @brief 扩展box轮廓操作
        * @param contours 轮廓-点的集合
        * @param b 轮廓扩展后的点集
        * @return None
        */
        void unclip(vector<Point> contours, vector<Point>& b);

        /**
        * @brief 计算点间距离操作
        * @param p0 第一个点
        * @param p1 第二个点
        * @return double 距离值
        */
        double distance(Point p0, Point p1);

        std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
        runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
        FrameCHWSize image_size_;   //输入图像大小（chw）
        FrameCHWSize input_size_;   //ocr检测model输入大小（chw）

        int input_height;   //ocr检测model输入高
        int input_width;    //ocr检测model输入宽
        float threshold;    //ocr检测mask阈值
	    float box_thresh;   //ocr检测阈值，用于过滤检测框
        int flag;           //ocr检测用于控制是否带空格
};
#endif
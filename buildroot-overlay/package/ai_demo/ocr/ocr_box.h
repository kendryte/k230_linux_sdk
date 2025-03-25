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

#ifndef _OCR_BOX_H
#define _OCR_BOX_H

#include "utils.h"
#include "ai_base.h"
#include "clipper.hpp"
#include "text_paint.h"

using namespace ClipperLib;

using namespace std;
using namespace cv;
using cv::Mat;
using std::cout;
using std::endl;
using std::ifstream;
using std::vector;

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
 * @brief 中心点对
 */
typedef struct CenterPrior
{
    int x;
    int y;
    int stride;
}CenterPrior;

/**
 * @brief ocr检测
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class OCRBox : public AIBase
{
    public:
        /**
        * @brief OCRBox构造函数，加载kmodel,并初始化kmodel输入、输出和ocr检测阈值
        * @param kmodel_file kmodel文件路径
        * @param threshold   ocr检测mask阈值， 0.2 - 0.3
        * @param box_thresh  ocr检测阈值，用于过滤检测框
        * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */
        OCRBox(const char *kmodel_file, float threshold, float box_thresh, const int debug_mode = 1);
        
        /**
        * @brief OCRBox构造函数，加载kmodel,并初始化kmodel输入、输出和ocr检测阈值
        * @param kmodel_file kmodel文件路径
        * @param threshold   ocr检测mask阈值， 0.2 - 0.3
        * @param box_thresh  ocr检测阈值，用于过滤检测框
        * @param isp_shape   isp输入大小（chw）
        * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */
        OCRBox(const char *kmodel_file, float threshold, float box_thresh, FrameCHWSize isp_shape,  const int debug_mode);
        
        /**
        * @brief OCRBox析构函数
        * @return None
        */
        ~OCRBox();

        /**
        * @brief 图片预处理
        * @param ori_img 原始图片
        * @return None
        */
        void pre_process(cv::Mat ori_img);

        /**
        * @brief 视频流预处理（ai2d for isp）
        * @return None
        */
        void pre_process(runtime_tensor& img_data);

        /**
        * @brief kmodel推理
        * @return None
        */
        void inference();

        /**
        * @brief kmodel推理结果后处理
        * @param frame_size 原始图像/帧宽高，用于将结果放到原始图像大小
        * @param results 后处理之后的基于原始图像的{检测点对{四边形的四个角点}}集合
        * @return None
        */
        void post_process(FrameSize frame_size, vector<ocr_det_res> &results);

        /**
        * @brief 将ocr检测和识别任务的结果绘制到图像上
        * @param frame         原始图像
        * @param results       ocr检测的结果
        * @param rec_results   ocr识别的结果文本
        * @param writepen      用于绘画文本的组件实例，可以写中文
        * @return None
        */
       static void draw_ocr_image(cv::Mat& frame, vector<ocr_det_res>& results,vector<std::string> &rec_results,ChineseTextRenderer &writepen);

        /**
        * @brief 将ocr检测和识别任务的结果绘制到屏幕的osd中
        * @param frame                 原始图像
        * @param results               ocr检测的结果
        * @param rec_results           ocr识别的结果文本
        * @param osd_frame_size        osd的宽高
        * @param sensor_frame_size     sensor的宽高
        * @param writepen              用于绘画文本的组件实例，可以写中文
        * @return None
        */
        static void draw_ocr_video(cv::Mat& frame, vector<ocr_det_res>& results,vector<std::string> &rec_results, FrameSize osd_frame_size, FrameSize sensor_frame_size,ChineseTextRenderer &writepen);

        /**
        * @brief 透射变换后crop
        * @param src         原始图像
        * @param dst         处理后的图像
        * @param b           车牌检测出的四边形角点对
        * @param vtd         排序后的四边形角点对
        * @return None
        */
        static void warppersp(cv::Mat src, cv::Mat& dst, ocr_det_res b, std::vector<Point2f>& vtd);
            
        
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
        * @param w 输入图像宽度padding的值
        * @param h 输入图像高度padding的值
        * @param frame_size 原始图像宽高
        * @return float 输出box分数值
        */
        float boxScore(Mat src, vector<Point> contours, ocr_det_res& b, int w, int h, FrameSize frame_size);
        
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
        runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
        runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
        FrameCHWSize isp_shape_;                     // isp对应的地址大小

        int input_height;   //ocr检测model输入高
        int input_width;    //ocr检测model输入宽
        float threshold;    //ocr检测mask阈值， 0.2 - 0.3
	    float box_thresh;   //ocr检测阈值，用于过滤检测框
        float *output;      //ocr后处理的输入
};
#endif
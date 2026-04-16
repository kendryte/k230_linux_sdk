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
// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <nncase/functional/ai2d/ai2d_builder.h>
#include "riscv_vector.h"

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::k230;
using namespace nncase::F::k230;

using cv::Mat;
using std::cout;
using std::endl;
using std::ifstream;
using std::vector;

//颜色盘，共80种颜色，类别大于80时取余
const std::vector<cv::Scalar> color_four = {
    cv::Scalar(60, 20, 220, 127),
    cv::Scalar(32, 11, 119, 127),
    cv::Scalar(142, 0, 0, 127),
    cv::Scalar(230, 0, 0, 127),
    cv::Scalar(228, 0, 106, 127),
    cv::Scalar(100, 60, 0, 127),
    cv::Scalar(100, 80, 0, 127),
    cv::Scalar(70, 0, 0, 127),
    cv::Scalar(192, 0, 0, 127),
    cv::Scalar(30, 170, 250, 127),
    cv::Scalar(30, 170, 100, 127),
    cv::Scalar(0, 220, 220, 127),
    cv::Scalar(175, 116, 175, 127),
    cv::Scalar(30, 0, 250, 127),
    cv::Scalar(42, 42, 165, 127),
    cv::Scalar(255, 77, 255, 127),
    cv::Scalar(252, 226, 0, 127),
    cv::Scalar(255, 182, 182, 127),
    cv::Scalar(0, 82, 0, 127),
    cv::Scalar(157, 166, 120, 127),
    cv::Scalar(0, 76, 110, 127),
    cv::Scalar(255, 57, 174, 127),
    cv::Scalar(0, 100, 199, 127),
    cv::Scalar(118, 0, 72, 127),
    cv::Scalar(240, 179, 255, 127),
    cv::Scalar(92, 125, 0, 127),
    cv::Scalar(151, 0, 209, 127),
    cv::Scalar(182, 208, 188, 127),
    cv::Scalar(176, 220, 0, 127),
    cv::Scalar(164, 99, 255, 127),
    cv::Scalar(73, 0, 92, 127),
    cv::Scalar(255, 129, 133, 127),
    cv::Scalar(255, 180, 78, 127),
    cv::Scalar(0, 228, 0, 127),
    cv::Scalar(243, 255, 174, 127),
    cv::Scalar(255, 89, 45, 127),
    cv::Scalar(103, 134, 134, 127),
    cv::Scalar(174, 148, 145, 127),
    cv::Scalar(186, 208, 255, 127),
    cv::Scalar(255, 226, 197, 127),
    cv::Scalar(1, 134, 171, 127),
    cv::Scalar(54, 63, 109, 127),
    cv::Scalar(255, 138, 207, 127),
    cv::Scalar(95, 0, 151, 127),
    cv::Scalar(61, 80, 9, 127),
    cv::Scalar(51, 105, 84, 127),
    cv::Scalar(105, 65, 74, 127),
    cv::Scalar(102, 196, 166, 127),
    cv::Scalar(210, 195, 208, 127),
    cv::Scalar(65, 109, 255, 127),
    cv::Scalar(149, 143, 0, 127),
    cv::Scalar(194, 0, 179, 127),
    cv::Scalar(106, 99, 209, 127),
    cv::Scalar(0, 121, 5, 127),
    cv::Scalar(205, 255, 227, 127),
    cv::Scalar(208, 186, 147, 127),
    cv::Scalar(1, 69, 153, 127),
    cv::Scalar(161, 95, 3, 127),
    cv::Scalar(0, 255, 163, 127),
    cv::Scalar(170, 0, 119, 127),
    cv::Scalar(199, 182, 0, 127),
    cv::Scalar(120, 165, 0, 127),
    cv::Scalar(88, 130, 183, 127),
    cv::Scalar(0, 32, 95, 127),
    cv::Scalar(135, 114, 130, 127),
    cv::Scalar(133, 129, 110, 127),
    cv::Scalar(118, 74, 166, 127),
    cv::Scalar(185, 142, 219, 127),
    cv::Scalar(114, 210, 79, 127),
    cv::Scalar(62, 90, 178, 127),
    cv::Scalar(15, 70, 65, 127),
    cv::Scalar(115, 167, 127, 127),
    cv::Scalar(106, 105, 59, 127),
    cv::Scalar(45, 108, 142, 127),
    cv::Scalar(0, 172, 196, 127),
    cv::Scalar(80, 54, 95, 127),
    cv::Scalar(255, 76, 128, 127),
    cv::Scalar(1, 57, 201, 127),
    cv::Scalar(122, 0, 246, 127),
    cv::Scalar(208, 162, 191, 127)
};

/**
 * @brief 单张/帧图片大小
 */
typedef struct FrameSize
{
    size_t width;  // 宽
    size_t height; // 高
} FrameSize;

/**
 * @brief 单张/帧图片大小
 */
typedef struct FrameCHWSize
{
    size_t channel;  // 通道数
    size_t height; // 高
    size_t width;  // 宽
} FrameCHWSize;

typedef struct YoloConfig
{
    char* model_type="yolov8";
    char* task_type="detect";
    char* task_mode="video";
    char* image_path="test.jpg";
    char* kmodel_path="yolov8n.kmodel";
    float conf_thres=0.35;
    float nms_thres=0.65;
    float mask_thres=0.5;
    int kp_num=17;;
    int kp_dim=3;
    std::string labels_txt_filepath="coco_labels.txt";
    int debug_mode=0;
} YoloConfig;


/**
* YOLO结果综合结构体，适配各个任务
*/
typedef struct YOLOBbox{
	cv::Rect box;       // det、seg、obb均会用到
	float confidence;   // cls、det、seg、obb均会用到
	int index;          // cls、det、seg、obb均会用到
    cv::Mat mask;       // 仅seg会用到
    float angle;        // 仅obb会用到
    float* kps{nullptr};  // 仅pose会用到，关键点数量由kp_num决定
    int kp_num;         // 仅pose会用到，关键点数量
    int kp_dim;         // 仅pose会用到，关键点维度
}YOLOBbox;

// 根据类别数使用模运算循环获取颜色
std::vector<cv::Scalar> getColorsForClasses(int num_classes);

std::vector<std::string> readLabelsFromTxt(std::string labels_txt_path);

void transpose_block_rvv(const float* input0, float* output_det,int box_num, int box_feature_len);

void transpose_block_fast(const float* input0, float* output_det,int box_num, int box_feature_len);

/**
 * @brief 工具类
 * 封装了AI常用的函数，包括二进制文件读取、文件保存、图片预处理等操作
 */
class Utils
{
public:
    /**
     * @brief 读取2进制文件
     * @param file_name 文件路径
     * @return 文件对应类型的数据
     */
    template <class T>
    static vector<T> read_binary_file(const char *file_name)
    {
        ifstream ifs(file_name, std::ios::binary);
        ifs.seekg(0, ifs.end);
        size_t len = ifs.tellg();
        vector<T> vec(len / sizeof(T), 0);
        ifs.seekg(0, ifs.beg);
        ifs.read(reinterpret_cast<char *>(vec.data()), len);
        ifs.close();
        return vec;
    }

    /**
     * @brief 打印数据
     * @param data 需打印数据对应指针
     * @param size 需打印数据大小
     * @return None
     */
    template <class T>
    static void dump(const T *data, size_t size)
    {
        for (size_t i = 0; i < size; i++)
        {
            cout << data[i] << " ";
        }
        cout << endl;
    }

    // 静态成员函数不依赖于类的实例，可以直接通过类名调用
    /**
     * @brief 将数据以2进制方式保存为文件
     * @param file_name 保存文件路径+文件名
     * @param data      需要保存的数据
     * @param size      需要保存的长度
     * @return None
     */
    static void dump_binary_file(const char *file_name, char *data, const size_t size);

    /**
     * @brief 将数据保存为灰度图片
     * @param file_name  保存图片路径+文件名
     * @param frame_size 保存图片的宽、高
     * @param data       需要保存的数据
     * @return None
     */
    static void dump_gray_image(const char *file_name, const FrameSize &frame_size, unsigned char *data);

    /**
     * @brief 将数据保存为彩色图片
     * @param file_name  保存图片路径+文件名
     * @param frame_size 保存图片的宽、高
     * @param data       需要保存的数据
     * @return None
     */
    static void dump_color_image(const char *file_name, const FrameSize &frame_size, unsigned char *data);

    static void padding_resize_one_side_set(FrameSize input_shape, FrameSize output_shape, std::unique_ptr<ai2d_builder> &builder, const cv::Scalar padding);

    static void center_crop_resize_set(FrameSize ori_shape,FrameSize output_shape,std::unique_ptr<ai2d_builder> &builder);
};

#endif

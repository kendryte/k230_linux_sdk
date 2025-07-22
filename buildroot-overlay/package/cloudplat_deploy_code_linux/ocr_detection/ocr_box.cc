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

#include "ocr_box.h"

/**
 * @brief OCRBox 类构造函数
 * @param args         OCR 检测模型的配置信息
 * @param isp_shape    输入图像的尺寸信息（通道、高度、宽度）
 * @param debug_mode   调试模式（用于控制日志打印）
 */
OCRBox::OCRBox(config_args args, FrameCHWSize isp_shape, const int debug_mode)
: AIBase(args.kmodel_path.c_str(), "OCRBox", debug_mode)
{
    model_name_ = "OCRBox";
    threshold = args.mask_thresh;   // 掩膜阈值
    box_thresh = args.box_thresh;   // 文本框得分阈值

    // 获取模型输入的尺寸信息
    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];

    // 保存输入图像尺寸（CHW 格式）
    image_size_ = {isp_shape.channel, isp_shape.height, isp_shape.width};
    input_size_ = {input_shapes_[0][1], input_shapes_[0][2], input_shapes_[0][3]};

    // 获取模型输入 tensor
    ai2d_out_tensor_ = this->get_input_tensor(0);

    // 设置 ai2d 图像预处理模块（padding + resize，固定一边）
    Utils::padding_resize_one_side_set(image_size_, input_size_, ai2d_builder_, cv::Scalar(114, 114, 114));
}

/**
 * @brief 析构函数
 */
OCRBox::~OCRBox()
{
}

/**
 * @brief 图像预处理函数
 * @param input_tensor 原始图像数据张量
 *
 * 功能：将原图 resize/pad 到模型输入尺寸，写入 ai2d_out_tensor_
 */
void OCRBox::pre_process(runtime_tensor &input_tensor)
{
    ScopedTiming st(model_name_ + " pre_process", debug_mode_);
    ai2d_builder_->invoke(input_tensor, ai2d_out_tensor_).expect("error occurred in ai2d running");
}

/**
 * @brief 模型推理函数
 */
void OCRBox::inference()
{
    this->run();         // 执行推理
    this->get_output();  // 获取输出结果
}

/**
 * @brief 后处理函数，根据模型输出生成文本框
 * @param results 输出的 OCR 检测结果向量
 */
void OCRBox::post_process(std::vector<ocr_det_res> &results)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);

    // 将模型输出转为单通道灰度图（掩膜概率）
    float* output = new float[input_size_.width * input_size_.height];
    for (int i = 0; i < input_size_.width * input_size_.height; i++)
        output[i] = p_outputs_[0][2 * i];

    // 创建灰度图并根据阈值生成二值掩膜
    cv::Mat src(input_size_.height, input_size_.width, CV_32FC1, output);
    cv::Mat mask(src > threshold);

    // 查找连通区域轮廓
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(mask, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    int num = contours.size();

    for (int i = 0; i < num; i++)
    {
        if (contours[i].size() < 4)
            continue;

        ocr_det_res b;

        // 获取最小外接矩形
        getBox(b, contours[i]);

        // 对轮廓进行膨胀（unclip）
        std::vector<cv::Point> con;
        unclip(contours[i], con);

        // 重新获取膨胀后的外接框
        getBox(b, con);

        // 计算文本框得分
        float score = boxScore(src, contours[i], b);
        if (score < box_thresh)
            continue;

        b.score = score;

        // 将坐标缩放回原图尺寸（根据 padding resize 的缩放比例）
        float ratiow = 1.0 * input_size_.width / image_size_.width;
        float ratioh = 1.0 * input_size_.height / image_size_.height;
        float ratio = std::min(ratiow, ratioh);

        for (int i = 0; i < 4; i++)
        {
            // 限制坐标合法范围
            b.vertices[i].x = std::clamp<int>(b.vertices[i].x, 0, input_size_.width);
            b.vertices[i].y = std::clamp<int>(b.vertices[i].y, 0, input_size_.height);

            // 映射到原图坐标
            b.vertices[i].x = b.vertices[i].x / ratio;
            b.vertices[i].y = b.vertices[i].y / ratio;

            // 保存原图坐标（备份）
            b.ver_src[i].x = b.vertices[i].x;
            b.ver_src[i].y = b.vertices[i].y;

            // 限制在原图范围内
            b.vertices[i].x = std::clamp<float>(b.vertices[i].x, 0, image_size_.width);
            b.vertices[i].y = std::clamp<float>(b.vertices[i].y, 0, image_size_.height);
        }

        // 添加到结果集合
        results.push_back(b);
    }

    delete[] output;
}

/**
 * @brief 绘制 OCR 检测结果到图像上
 * @param draw_frame   目标图像
 * @param results      OCR 检测结果集合
 * @param frame_size   原图尺寸
 */
void OCRBox::draw_result(cv::Mat &draw_frame, std::vector<ocr_det_res> &results, FrameCHWSize frame_size)
{
    int w_ = draw_frame.cols;
    int h_ = draw_frame.rows;

    for (int i = 0; i < results.size(); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            // 坐标映射到目标图尺寸
            int x1 = int(results[i].vertices[j].x * w_ / frame_size.width);
            int y1 = int(results[i].vertices[j].y * h_ / frame_size.height);
            int x2 = int(results[i].vertices[(j + 1) % 4].x * w_ / frame_size.width);
            int y2 = int(results[i].vertices[(j + 1) % 4].y * h_ / frame_size.height);

            // 绘制框线
            cv::line(draw_frame, cv::Point2d(x1, y1), cv::Point2d(x2, y2), cv::Scalar(255, 0, 0, 255), 3);
        }
    }
}

/**
 * @brief 获取最小外接矩形的四个角点
 * @param b         输出框结构体
 * @param contours  输入轮廓点集合
 */
void OCRBox::getBox(ocr_det_res &b, std::vector<cv::Point> contours)
{
    cv::RotatedRect minrect = cv::minAreaRect(contours);
    cv::Point2f vtx[4];
    minrect.points(vtx);
    for (int i = 0; i < 4; i++)
    {
        b.vertices[i].x = vtx[i].x;
        b.vertices[i].y = vtx[i].y;
    }
}

/**
 * @brief 计算两点之间的欧几里得距离
 */
double OCRBox::distance(cv::Point p0, cv::Point p1)
{
    return std::sqrt((p0.x - p1.x) * (p0.x - p1.x) + (p1.y - p0.y) * (p1.y - p0.y));
}

/**
 * @brief 对轮廓做 unclip 操作（多边形膨胀）
 * @param contours 原始轮廓
 * @param con      输出膨胀后的轮廓
 */
void OCRBox::unclip(std::vector<cv::Point> contours, std::vector<cv::Point> &con)
{
    Path subj;
    Paths solution;
    double dis = 0.0;

    // 构建初始路径
    for (int i = 0; i < contours.size(); i++)
        subj << IntPoint(contours[i].x, contours[i].y);

    // 估计轮廓周长
    for (int i = 0; i < contours.size() - 1; i++)
        dis += distance(contours[i], contours[i + 1]);

    // 计算膨胀距离（与面积成比例）
    double dis1 = (-1 * Area(subj)) * 1.5 / dis;

    // 膨胀路径
    ClipperOffset co;
    co.AddPath(subj, jtSquare, etClosedPolygon);
    co.Execute(solution, dis1);

    Path tmp = solution[0];
    for (int i = 0; i < tmp.size(); i++)
        con.push_back(cv::Point(tmp[i].X, tmp[i].Y));
}

/**
 * @brief 计算文本框区域内的平均得分
 * @param src      原始灰度图
 * @param contours 文本轮廓
 * @param b        OCR 框对象（用于保存中心位置）
 * @return 平均得分
 */
float OCRBox::boxScore(cv::Mat src, std::vector<cv::Point> contours, ocr_det_res &b)
{
    // 获取边界框
    int xmin = input_shapes_[0][3], xmax = 0;
    int ymin = input_shapes_[0][2], ymax = 0;

    for (int i = 0; i < contours.size(); i++)
    {
        xmin = std::floor(std::min(xmin, contours[i].x));
        xmax = std::ceil(std::max(xmax, contours[i].x));
        ymin = std::floor(std::min(ymin, contours[i].y));
        ymax = std::ceil(std::max(ymax, contours[i].y));
    }

    // 平移轮廓用于填充 mask
    for (int i = 0; i < contours.size(); i++)
    {
        contours[i].x -= xmin;
        contours[i].y -= ymin;
    }

    std::vector<std::vector<cv::Point>> vec{contours};

    // 框中心点（映射回原图）
    b.meanx = ((1.0 * xmin + xmax) / 2) / input_shapes_[0][3] * image_size_.width;
    b.meany = ((1.0 * ymin + ymax) / 2) / input_shapes_[0][2] * image_size_.height;

    // 创建掩膜图
    cv::Mat img = cv::Mat::zeros(ymax - ymin + 1, xmax - xmin + 1, CV_8UC1);
    cv::fillPoly(img, vec, cv::Scalar(1));

    // 计算平均得分
    return (float)cv::mean(src(cv::Rect(xmin, ymin, xmax - xmin + 1, ymax - ymin + 1)), img)[0];
}

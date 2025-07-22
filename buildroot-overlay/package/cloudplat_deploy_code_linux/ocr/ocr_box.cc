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
 * @brief 构造函数：初始化 OCR 文本检测模型
 * @param args        配置参数，含模型路径、阈值等
 * @param isp_shape   图像原始尺寸（CHW）
 * @param debug_mode  是否开启调试信息
 */
OCRBox::OCRBox(config_args args, FrameCHWSize isp_shape, const int debug_mode)
: AIBase(args.kmodel_path.c_str(), "OCRBox", debug_mode)
{
    model_name_ = "OCRBox";
    threshold = args.mask_thresh;  // mask 二值化阈值
    box_thresh = args.box_thresh;  // 框置信度阈值

    // 模型输入尺寸
    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];

    // 设置图像尺寸
    image_size_ = {isp_shape.channel, isp_shape.height, isp_shape.width};
    input_size_ = {input_shapes_[0][1], input_shapes_[0][2], input_shapes_[0][3]};

    // 获取模型输入 tensor
    ai2d_out_tensor_ = this->get_input_tensor(0);

    // 配置图像预处理工具：按一边等比缩放，另一边 padding，填充值 114
    Utils::padding_resize_one_side_set(image_size_, input_size_, ai2d_builder_, cv::Scalar(114, 114, 114));
}

OCRBox::~OCRBox() = default;

/**
 * @brief 图像预处理函数，生成模型输入 tensor
 */
void OCRBox::pre_process(runtime_tensor &input_tensor)
{
    ScopedTiming st(model_name_ + " pre_process", debug_mode_);
    ai2d_builder_->invoke(input_tensor, ai2d_out_tensor_).expect("error occurred in ai2d running");
}

/**
 * @brief 模型推理
 */
void OCRBox::inference()
{
    this->run();         // 执行模型
    this->get_output();  // 获取输出
}

/**
 * @brief 模型后处理，解析 mask 输出，生成文本框结果
 * @param results 输出的文本框集合
 */
void OCRBox::post_process(vector<ocr_det_res> &results)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);

    // 将模型输出的 mask 转为 float 矩阵
    float* output = new float[input_size_.width * input_size_.height];
    for (int i = 0; i < input_size_.width * input_size_.height; i++)
        output[i] = p_outputs_[0][2 * i];

    // 构建二值化掩码图
    Mat src(input_size_.height, input_size_.width, CV_32FC1, output);
    Mat mask(src > threshold);

    // 提取轮廓
    vector<vector<cv::Point>> contours;
    vector<cv::Vec4i> hierarchy;
    cv::findContours(mask, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    int num = contours.size();
    for (int i = 0; i < num; i++)
    {
        if (contours[i].size() < 4) continue;

        ocr_det_res b;
        getBox(b, contours[i]);             // 获取最小外接矩形
        vector<cv::Point> con;
        unclip(contours[i], con);           // 解膨胀（放大区域）
        getBox(b, con);                     // 再次获取 box
        float score = boxScore(src, contours[i], b); // 计算得分

        if (score < box_thresh) continue;
        b.score = score;

        // 坐标映射回原图尺度
        float ratiow = 1.0 * input_size_.width / image_size_.width;
        float ratioh = 1.0 * input_size_.height / image_size_.height;
        float ratio = std::min(ratiow, ratioh);

        for (int i = 0; i < 4; i++)
        {
            // 防止越界并映射回原图坐标
            b.vertices[i].x = std::clamp((int)b.vertices[i].x, 0, input_size_.width) / ratio;
            b.vertices[i].y = std::clamp((int)b.vertices[i].y, 0, input_size_.height) / ratio;
            b.ver_src[i] = b.vertices[i];
            b.vertices[i].x = std::clamp(b.vertices[i].x, 0.f, (float)image_size_.width);
            b.vertices[i].y = std::clamp(b.vertices[i].y, 0.f, (float)image_size_.height);
        }

        results.push_back(b);
    }

    delete[] output;
}

/**
 * @brief 可视化绘制检测框及识别文本
 */
void OCRBox::draw_result(cv::Mat &draw_frame, vector<ocr_det_res> &results,
                         vector<string> &ocrrec_results, FrameCHWSize frame_size,
                         TextRenderer &writepen)
{
    int w_ = draw_frame.cols;
    int h_ = draw_frame.rows;

    for (int i = 0; i < results.size(); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            int x1 = int(results[i].vertices[j].x * w_ / frame_size.width);
            int y1 = int(results[i].vertices[j].y * h_ / frame_size.height);
            int x2 = int(results[i].vertices[(j + 1) % 4].x * w_ / frame_size.width);
            int y2 = int(results[i].vertices[(j + 1) % 4].y * h_ / frame_size.height);
            if(draw_frame.channels() == 3){
                cv::line(draw_frame, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 0, 0), 3);
                if (j == 0) {
                    // 在左上角标注文本
                    writepen.putText(draw_frame, ocrrec_results[i], cv::Point(x1, y1 - 40), cv::Scalar(255, 0, 0));
                }
            }
            else if(draw_frame.channels()==4){
                cv::line(draw_frame, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 0, 0,255), 3);
                if (j == 0) {
                    // 在左上角标注文本
                    writepen.putText(draw_frame, ocrrec_results[i], cv::Point(x1, y1 - 20), cv::Scalar(255, 0, 0,255));
                }
            }
            
        }
    }
}

/**
 * @brief 获取最小外接矩形的四个角点
 */
void OCRBox::getBox(ocr_det_res &b, vector<Point> contours)
{
    RotatedRect minrect = minAreaRect(contours);
    Point2f vtx[4];
    minrect.points(vtx);
    for (int i = 0; i < 4; i++) {
        b.vertices[i] = vtx[i];
    }
}

/**
 * @brief 两点之间欧几里得距离
 */
double OCRBox::distance(Point p0, Point p1)
{
    return sqrt((p0.x - p1.x) * (p0.x - p1.x) + (p0.y - p1.y) * (p0.y - p1.y));
}

/**
 * @brief 使用 Clipper 进行外扩（unclip）操作，放大检测区域
 */
void OCRBox::unclip(vector<Point> contours, vector<Point> &con)
{
    Path subj;
    for (const auto& pt : contours)
        subj << IntPoint(pt.x, pt.y);

    double dis = 0.0;
    for (int i = 0; i < contours.size() - 1; i++)
        dis += distance(contours[i], contours[i + 1]);

    double dis1 = (-1.0 * Area(subj)) * 2.5 / dis;

    ClipperOffset co;
    co.AddPath(subj, jtSquare, etClosedPolygon);
    Paths solution;
    co.Execute(solution, dis1);

    if (!solution.empty()) {
        for (const auto& pt : solution[0])
            con.emplace_back(pt.X, pt.Y);
    }
}

/**
 * @brief 计算文本框得分：在 mask 上对应区域平均值
 */
float OCRBox::boxScore(Mat src, vector<cv::Point> contours, ocr_det_res &b)
{
    int xmin = input_shapes_[0][3], xmax = 0;
    int ymin = input_shapes_[0][2], ymax = 0;

    for (auto &pt : contours) {
        xmin = std::min(xmin, pt.x);
        xmax = std::max(xmax, pt.x);
        ymin = std::min(ymin, pt.y);
        ymax = std::max(ymax, pt.y);
    }

    for (auto &pt : contours) {
        pt.x -= xmin;
        pt.y -= ymin;
    }

    vector<vector<Point>> vec = {contours};
    b.meanx = ((xmin + xmax) / 2.0) / input_shapes_[0][3] * image_size_.width;
    b.meany = ((ymin + ymax) / 2.0) / input_shapes_[0][2] * image_size_.height;

    Mat mask = Mat::zeros(ymax - ymin + 1, xmax - xmin + 1, CV_8UC1);
    cv::fillPoly(mask, vec, Scalar(1));
    return (float)cv::mean(src(Rect(xmin, ymin, xmax - xmin + 1, ymax - ymin + 1)), mask)[0];
}

/**
 * @brief 对四个点按 x 排序返回索引
 */
std::vector<size_t> sort_indices(std::vector<cv::Point2f> &vec)
{
    std::vector<size_t> indices(vec.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&vec](size_t i1, size_t i2) {
        return vec[i1].x < vec[i2].x;
    });
    return indices;
}

/**
 * @brief 根据点位置排序返回四个标准顶点（左上、右上、右下、左下）
 */
void find_rectangle_vertices(std::vector<cv::Point2f> &points,
                             cv::Point2f &topLeft, cv::Point2f &topRight,
                             cv::Point2f &bottomRight, cv::Point2f &bottomLeft)
{
    auto sorted_x_id = sort_indices(points);
    auto &left1 = points[sorted_x_id[0]];
    auto &left2 = points[sorted_x_id[1]];
    auto &right1 = points[sorted_x_id[2]];
    auto &right2 = points[sorted_x_id[3]];

    topLeft     = (left1.y < left2.y) ? left1 : left2;
    bottomLeft  = (left1.y < left2.y) ? left2 : left1;
    topRight    = (right1.y < right2.y) ? right1 : right2;
    bottomRight = (right1.y < right2.y) ? right2 : right1;
}

/**
 * @brief 对检测框区域进行透视变换，便于识别模型输入
 */
void OCRBox::warppersp(cv::Mat &src, cv::Mat &dst, ocr_det_res &b, std::vector<cv::Point2f> &vtd)
{
    std::vector<cv::Point2f> box(b.vertices, b.vertices + 4);
    cv::RotatedRect minrect = cv::minAreaRect(box);

    std::vector<cv::Point2f> rect_pts(4);
    minrect.points(rect_pts.data());

    // 排序确定顶点顺序
    find_rectangle_vertices(rect_pts, vtd[0], vtd[1], vtd[2], vtd[3]);

    // 计算透视变换目标矩形大小
    float w = std::max(cv::norm(vtd[1] - vtd[0]), cv::norm(vtd[2] - vtd[1]));
    float h = std::min(cv::norm(vtd[1] - vtd[0]), cv::norm(vtd[2] - vtd[1]));

    std::array<cv::Point2f, 4> dst_pts = {
        cv::Point2f(0, 0), cv::Point2f(w, 0), cv::Point2f(w, h), cv::Point2f(0, h)
    };

    cv::Mat matrix = cv::getPerspectiveTransform(vtd, dst_pts);
    cv::warpPerspective(src, dst, matrix, cv::Size(w, h));
}

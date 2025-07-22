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


#include "anchorbase_det.h"

/**
 * @brief AnchorBaseDet构造函数，初始化多目标检测模块
 * @param args        构建对象需要的参数（如kmodel路径、阈值、类别信息等）
 * @param isp_shape   isp输入图像大小（CHW）
 * @param debug_mode  调试模式（0-关闭调试、1-只显示耗时、2-显示所有信息）
 */
AnchorBaseDet::AnchorBaseDet(config_args args, FrameCHWSize isp_shape, const int debug_mode)
:AIBase(args.kmodel_path.c_str(),"AnchorBaseDet", debug_mode)
{
    ob_det_thresh = args.obj_thresh;          // 目标置信度阈值
    ob_nms_thresh = args.nms_thresh;          // NMS阈值
    labels = args.labels;                     // 类别标签
    nms_option = args.nms_option;             // 是否使用全类别NMS
    num_class = labels.size();                // 类别总数
    memcpy(this->strides, args.strides, sizeof(args.strides));   // 拷贝每层stride
    memcpy(this->anchors, args.anchors, sizeof(args.anchors));   // 拷贝每层anchor
    colors = getColorsForClasses(num_class);  // 获取每个类别对应的颜色
    input_width = input_shapes_[0][3];        // 模型输入宽
    input_height = input_shapes_[0][2];       // 模型输入高

    image_size_ = {isp_shape.channel, isp_shape.height, isp_shape.width};             // 实际图像尺寸
    input_size_ = {input_shapes_[0][1], input_shapes_[0][2], input_shapes_[0][3]};    // 模型输入尺寸
    ai2d_out_tensor_ = this->get_input_tensor(0); // 获取模型输入tensor
    Utils::padding_resize_two_side_set(image_size_, input_size_, ai2d_builder_, cv::Scalar(114,114,114)); // 设置双边padding resize
}

/**
 * @brief AnchorBaseDet析构函数
 */
AnchorBaseDet::~AnchorBaseDet() {}


/**
 * @brief 模型输入预处理（如resize+padding+归一化）
 * @param input_tensor 处理后的输入tensor
 */
void AnchorBaseDet::pre_process(runtime_tensor &input_tensor)
{
    ScopedTiming st(model_name_ + " pre_process", debug_mode_);
    ai2d_builder_->invoke(input_tensor, ai2d_out_tensor_).expect("error occurred in ai2d running");
}


/**
 * @brief 模型推理阶段（前向运行）
 */
void AnchorBaseDet::inference()
{
    this->run();        // 执行模型推理
    this->get_output(); // 获取输出结果
}


/**
 * @brief 模型推理后的后处理逻辑（解码 + NMS）
 * @param results 后处理输出的目标框结果集
 */
void AnchorBaseDet::post_process(vector<ob_det_res> &results)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);

    float* output_0 = p_outputs_[0];
    float* output_1 = p_outputs_[1];
    float* output_2 = p_outputs_[2];

    if (nms_option)
    {
        // 合并所有层输出，再进行一次全局NMS
        vector<ob_det_res> box0 = decode_infer(output_0, 0);
        vector<ob_det_res> box1 = decode_infer(output_1, 1);
        vector<ob_det_res> box2 = decode_infer(output_2, 2);

        results.insert(results.end(), box0.begin(), box0.end());
        results.insert(results.end(), box1.begin(), box1.end());
        results.insert(results.end(), box2.begin(), box2.end());

        nms(results);
    }
    else
    {
        // 对每个类别分别解码并NMS
        vector<vector<ob_det_res>> box0 = decode_infer_class(output_0, 0);
        vector<vector<ob_det_res>> box1 = decode_infer_class(output_1, 1);
        vector<vector<ob_det_res>> box2 = decode_infer_class(output_2, 2);

        for (int i = 0; i < num_class; i++)
        {
            box0[i].insert(box0[i].end(), box1[i].begin(), box1[i].end());
            box0[i].insert(box0[i].end(), box2[i].begin(), box2[i].end());
            nms(box0[i]);
            results.insert(results.end(), box0[i].begin(), box0[i].end());
        }
    }
}


/**
 * @brief 将模型输出的特征图进行解码（适用于NMS全局合并方式）
 * @param data  特征图数据指针
 * @param k     第k个输出层索引
 * @return 解码得到的目标检测框集合
 */
vector<ob_det_res> AnchorBaseDet::decode_infer(float* data, int k)
{
    int stride = strides[k];
    float ratiow = (float)input_width / image_size_.width;
    float ratioh = (float)input_height / image_size_.height;
    float gain = std::min(ratiow, ratioh);

    std::vector<ob_det_res> result;
    int grid_size_w = input_width / stride;
    int grid_size_h = input_height / stride;
    int one_rsize = num_class + 5;

    for (int shift_y = 0; shift_y < grid_size_h; shift_y++)
    {
        for (int shift_x = 0; shift_x < grid_size_w; shift_x++)
        {
            int loc = shift_x + shift_y * grid_size_w;
            for (int i = 0; i < 3; i++)
            {
                float* record = data + (loc * 3 + i) * one_rsize;
                float* cls_ptr = record + 5;
                for (int cls = 0; cls < num_class; cls++)
                {
                    float score = cls_ptr[cls] * record[4];
                    if (score > ob_det_thresh)
                    {
                        // 解码预测框并映射回原图尺寸
                        float cx = ((record[0]) * 2.f - 0.5f + shift_x) * stride;
                        float cy = ((record[1]) * 2.f - 0.5f + shift_y) * stride;
                        float w = pow(record[2] * 2.f, 2) * anchors[k][i][0];
                        float h = pow(record[3] * 2.f, 2) * anchors[k][i][1];
                        cx -= (input_width - image_size_.width * gain) / 2;
                        cy -= (input_height - image_size_.height * gain) / 2;
                        cx /= gain; cy /= gain;
                        w /= gain; h /= gain;

                        ob_det_res box;
                        box.x1 = std::clamp(int(cx - w / 2.f), 0, image_size_.width);
                        box.y1 = std::clamp(int(cy - h / 2.f), 0, image_size_.height);
                        box.x2 = std::clamp(int(cx + w / 2.f), 0, image_size_.width);
                        box.y2 = std::clamp(int(cy + h / 2.f), 0, image_size_.height);
                        box.score = score;
                        box.label_index = cls;
                        box.label = labels[cls];
                        result.push_back(box);
                    }
                }
            }
        }
    }
    return result;
}


/**
 * @brief 类别级别的目标框解码（适用于每类独立NMS方式）
 * @param data  模型输出层数据
 * @param k     第k层索引
 * @return 每个类别对应的一组目标框
 */
vector<vector<ob_det_res>> AnchorBaseDet::decode_infer_class(float* data, int k)
{
    int stride = strides[k];
    float ratiow = (float)input_width / image_size_.width;
    float ratioh = (float)input_height / image_size_.height;
    float gain = std::min(ratiow, ratioh);

    vector<vector<ob_det_res>> result(num_class); // 每个类别单独保存

    int grid_size_w = input_width / stride;
    int grid_size_h = input_height / stride;
    int one_rsize = num_class + 5;

    for (int shift_y = 0; shift_y < grid_size_h; shift_y++)
    {
        for (int shift_x = 0; shift_x < grid_size_w; shift_x++)
        {
            int loc = shift_x + shift_y * grid_size_w;
            for (int i = 0; i < 3; i++)
            {
                float* record = data + (loc * 3 + i) * one_rsize;
                float* cls_ptr = record + 5;
                for (int cls = 0; cls < num_class; cls++)
                {
                    float score = cls_ptr[cls] * record[4];
                    if (score > ob_det_thresh)
                    {
                        float cx = ((record[0]) * 2.f - 0.5f + shift_x) * stride;
                        float cy = ((record[1]) * 2.f - 0.5f + shift_y) * stride;
                        float w = pow(record[2] * 2.f, 2) * anchors[k][i][0];
                        float h = pow(record[3] * 2.f, 2) * anchors[k][i][1];
                        cx -= (input_width - image_size_.width * gain) / 2;
                        cy -= (input_height - image_size_.height * gain) / 2;
                        cx /= gain; cy /= gain;
                        w /= gain; h /= gain;

                        ob_det_res box;
                        box.x1 = std::clamp(int(cx - w / 2.f), 0, image_size_.width);
                        box.y1 = std::clamp(int(cy - h / 2.f), 0, image_size_.height);
                        box.x2 = std::clamp(int(cx + w / 2.f), 0, image_size_.width);
                        box.y2 = std::clamp(int(cy + h / 2.f), 0, image_size_.height);
                        box.score = score;
                        box.label_index = cls;
                        box.label = labels[cls];
                        result[cls].push_back(box);
                    }
                }
            }
        }
    }
    return result;
}

/**
 * @brief 非极大值抑制，移除重叠度高的框
 * @param input_boxes 待抑制的候选框集合
 */
void AnchorBaseDet::nms(vector<ob_det_res>& input_boxes)
{
    std::sort(input_boxes.begin(), input_boxes.end(), [](ob_det_res a, ob_det_res b) { return a.score > b.score; });
    std::vector<float> vArea(input_boxes.size());
    for (size_t i = 0; i < input_boxes.size(); ++i)
    {
        vArea[i] = (input_boxes[i].x2 - input_boxes[i].x1 + 1) * (input_boxes[i].y2 - input_boxes[i].y1 + 1);
    }

    for (size_t i = 0; i < input_boxes.size(); ++i)
    {
        for (size_t j = i + 1; j < input_boxes.size(); )
        {
            float xx1 = std::max(input_boxes[i].x1, input_boxes[j].x1);
            float yy1 = std::max(input_boxes[i].y1, input_boxes[j].y1);
            float xx2 = std::min(input_boxes[i].x2, input_boxes[j].x2);
            float yy2 = std::min(input_boxes[i].y2, input_boxes[j].y2);
            float w = std::max(0.f, xx2 - xx1 + 1);
            float h = std::max(0.f, yy2 - yy1 + 1);
            float inter = w * h;
            float ovr = inter / (vArea[i] + vArea[j] - inter);
            if (ovr >= ob_nms_thresh)
            {
                input_boxes.erase(input_boxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            }
            else j++;
        }
    }
}

/**
 * @brief 在图像上绘制检测结果
 * @param draw_frame 待绘制图像
 * @param image_size 原图尺寸
 * @param results 检测结果集合
 * @param writepen 文本绘制工具
 */
void AnchorBaseDet::draw_result(cv::Mat &draw_frame, FrameCHWSize image_size, vector<ob_det_res> &results, TextRenderer &writepen)
{
    int w_ = draw_frame.cols;
    int h_ = draw_frame.rows;
    for (const auto& box : results)
    {
        int idx = box.label_index;
        float score = box.score;
        int x = int(box.x1 * w_ / image_size.width);
        int y = int(box.y1 * h_ / image_size.height);
        int w = int((box.x2 - box.x1) * w_ / image_size.width);
        int h = int((box.y2 - box.y1) * h_ / image_size.height);
        w = std::min(w_, x + w) - x;
        h = std::min(h_, y + h) - y;
        cv::Rect new_box(x, y, w, h);
        string text = "类别：" + box.label + " 分数：" + std::to_string(score);

        cv::Scalar color = getColorForIndex(idx);
        if (draw_frame.channels() == 3)
        {
            cv::rectangle(draw_frame, new_box, cv::Scalar(color[3], color[2], color[1]), 2, 8);
            writepen.putText(draw_frame, text, cv::Point(MIN(new_box.x + 5, w_), MAX(new_box.y - 10, 0)),
                             cv::Scalar(color[3], color[2], color[1]));
        }
        else if (draw_frame.channels() == 4)
        {
            cv::rectangle(draw_frame, new_box, cv::Scalar(color[3], color[2], color[1], color[0]), 2, 8);
            writepen.putText(draw_frame, text, cv::Point(MIN(new_box.x + 5, w_), MAX(new_box.y - 10, 0)),
                             cv::Scalar(color[3], color[2], color[1], color[0]));
        }
    }
}

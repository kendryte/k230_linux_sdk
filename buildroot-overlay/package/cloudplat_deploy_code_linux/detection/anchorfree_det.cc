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

#include "anchorfree_det.h"

/**
 * @brief AnchorFreeDet构造函数，初始化模型及参数
 * @param args 配置参数，包括模型路径、阈值、标签等
 * @param isp_shape 输入图像尺寸（通道，高，宽）
 * @param debug_mode 调试模式开关
 */
AnchorFreeDet::AnchorFreeDet(config_args args, FrameCHWSize isp_shape, const int debug_mode)
:AIBase(args.kmodel_path.c_str(),"AnchorFreeDet", debug_mode)
{
    ob_det_thresh = args.obj_thresh;      /**< 目标检测置信度阈值 */
    ob_nms_thresh = args.nms_thresh;      /**< NMS（非极大值抑制）阈值 */
    labels = args.labels;                  /**< 类别标签列表 */
    nms_option = args.nms_option;          /**< NMS模式选择（是否按类别分别做NMS） */
    num_class = labels.size();             /**< 类别数量 */
    memcpy(this->strides, args.strides, sizeof(args.strides));  /**< 网络的步长数组复制 */
    input_width = input_shapes_[0][3];    /**< 网络输入图像宽度 */
    input_height = input_shapes_[0][2];   /**< 网络输入图像高度 */

    image_size_ = {isp_shape.channel, isp_shape.height, isp_shape.width};  /**< 输入图像尺寸（CHW） */
    input_size_ = {input_shapes_[0][1], input_shapes_[0][2], input_shapes_[0][3]}; /**< 网络输入尺寸（CHW） */
    ai2d_out_tensor_ = this->get_input_tensor(0);  /**< 获取网络输入tensor */
    // 设置预处理：对输入图像做padding和resize，边缘用灰色填充
    Utils::padding_resize_two_side_set(image_size_, input_size_, ai2d_builder_, cv::Scalar(114,114,114));
}

/**
 * @brief AnchorFreeDet析构函数
 */
AnchorFreeDet::~AnchorFreeDet()
{
}

/**
 * @brief 图像预处理，调用ai2d_builder执行预处理操作
 * @param input_tensor 输入的原始tensor数据
 */
void AnchorFreeDet::pre_process(runtime_tensor &input_tensor){
    ScopedTiming st(model_name_ + " pre_process", debug_mode_);
    ai2d_builder_->invoke(input_tensor, ai2d_out_tensor_).expect("error occurred in ai2d running");
}

/**
 * @brief 推理执行，包括模型运行和输出获取
 */
void AnchorFreeDet::inference()
{
    this->run();
    this->get_output();
}

/**
 * @brief 后处理，包含框的解码和NMS处理
 * @param results 输出检测结果的引用，用于存放最终框
 */
void AnchorFreeDet::post_process(vector<ob_det_res> &results)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    float* output_0 = p_outputs_[0]; /**< 网络输出0 */
    float* output_1 = p_outputs_[1]; /**< 网络输出1 */
    float* output_2 = p_outputs_[2]; /**< 网络输出2 */

    if (nms_option)
    {
        vector<ob_det_res> box0, box1, box2;

        // 解码每个输出，获得检测框
        box0 = decode_infer(output_0, 0);
        box1 = decode_infer(output_1, 1);
        box2 = decode_infer(output_2, 2);

        // 将所有框合并
        results.insert(results.begin(), box0.begin(), box0.end());
        results.insert(results.begin(), box1.begin(), box1.end());
        results.insert(results.begin(), box2.begin(), box2.end());
        
        // 对所有框做NMS
        nms(results);
    }
    else
    {
        vector<vector<ob_det_res>> box0, box1, box2;

        // 按类别解码输出
        box0 = decode_infer_class(output_0, 0);
        box1 = decode_infer_class(output_1, 1);
        box2 = decode_infer_class(output_2, 2);

        // 对每个类别合并各层框并执行NMS，最终合并到results
        for(int i = 0; i < num_class; i++)
        {
            box0[i].insert(box0[i].begin(), box1[i].begin(), box1[i].end());
            box0[i].insert(box0[i].begin(), box2[i].begin(), box2[i].end());
            nms(box0[i]);
            results.insert(results.begin(), box0[i].begin(), box0[i].end());
        }
    }
}

/**
 * @brief 解码网络输出为检测框（不区分类别）
 * @param data 网络输出数据指针
 * @param k 第k个输出层索引
 * @return 返回检测框列表
 */
vector<ob_det_res> AnchorFreeDet::decode_infer(float* data, int k)
{
    float ratiow = (float)input_width / image_size_.width;
    float ratioh = (float)input_height / image_size_.height;
    float gain = ratiow < ratioh ? ratiow : ratioh; /**< 计算缩放比例 */
    std::vector<ob_det_res> result;
    int grid_size_w = input_width / strides[k]; /**< 网格宽度 */
    int grid_size_h = input_height / strides[k]; /**< 网格高度 */
    int one_rsize = num_class + 5; /**< 每个网格点输出数据长度 */
    float cx, cy, w, h;

    for (int shift_y = 0; shift_y < grid_size_h; shift_y++)
    {
        for (int shift_x = 0; shift_x < grid_size_w; shift_x++)
        {
            int loc = shift_x + shift_y * grid_size_w;
            for (int i = 0; i < 1; i++)
            {
                float* record = data + (loc + i) * one_rsize;
                float* cls_ptr = record + 5;
                for (int cls = 0; cls < num_class; cls++)
                {
                    // float score = sigmoid(record[4]);
                    float score = record[4]; /**< 置信度 */
                    if (score > ob_det_thresh)  /**< 置信度大于阈值，保存框 */
                    {
                        cx = ((record[0]) + (float)shift_x) * (float)strides[k];
                        cy = ((record[1]) + (float)shift_y) * (float)strides[k];
                        w = exp((record[2])) * (float)strides[k];
                        h = exp((record[3])) * (float)strides[k];

                        // 逆向缩放和去padding恢复到原图坐标
                        cx -= ((input_width - image_size_.width * gain) / 2);
                        cy -= ((input_height - image_size_.height * gain) / 2);
                        cx /= gain;
                        cy /= gain;
                        w /= gain;
                        h /= gain;

                        ob_det_res box;
                        box.x1 = std::max(0, std::min(int(image_size_.width), int(cx - w / 2.f)));
                        box.y1 = std::max(0, std::min(int(image_size_.height), int(cy - h / 2.f)));
                        box.x2 = std::max(0, std::min(int(image_size_.width), int(cx + w / 2.f)));
                        box.y2 = std::max(0, std::min(int(image_size_.height), int(cy + h / 2.f)));
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
 * @brief 按类别解码网络输出为检测框
 * @param data 网络输出数据指针
 * @param k 第k个输出层索引
 * @return 返回二维检测框数组，每个类别对应一个框列表
 */
vector<vector<ob_det_res>> AnchorFreeDet::decode_infer_class(float* data,  int k)
{
    float ratiow = (float)input_width / image_size_.width;
    float ratioh = (float)input_height / image_size_.height;
    float gain = ratiow < ratioh ? ratiow : ratioh; /**< 计算缩放比例 */
    std::vector<std::vector<ob_det_res>> result;
    for (int i = 0; i < num_class; i++)
    {
        result.push_back(vector<ob_det_res>()); /**< 初始化每个类别的检测框列表 */
    }
    int grid_size_w = input_width / strides[k]; /**< 网格宽度 */
    int grid_size_h = input_height / strides[k]; /**< 网格高度 */
    int one_rsize = num_class + 5; /**< 每个网格点输出长度 */
    float cx, cy, w, h;

    for (int shift_y = 0; shift_y < grid_size_h; shift_y++)
    {
        for (int shift_x = 0; shift_x < grid_size_w; shift_x++)
        {
            int loc = shift_x + shift_y * grid_size_w;
            for (int i = 0; i < 1; i++)
            {
                float* record = data + (loc + i) * one_rsize;
                float* cls_ptr = record + 5;
                for (int cls = 0; cls < num_class; cls++)
                {
                    // float score = sigmoid(record[4]);
                    float score = record[4]; /**< 置信度 */
                    if (score > ob_det_thresh) /**< 置信度阈值判断 */
                    {
                        cx = ((record[0]) + (float)shift_x) * (float)strides[k];
                        cy = ((record[1]) + (float)shift_y) * (float)strides[k];
                        w = exp((record[2])) * (float)strides[k];
                        h = exp((record[3])) * (float)strides[k];

                        // 恢复到原图坐标
                        cx -= ((input_width - image_size_.width * gain) / 2);
                        cy -= ((input_height - image_size_.height * gain) / 2);
                        cx /= gain;
                        cy /= gain;
                        w /= gain;
                        h /= gain;

                        ob_det_res box;
                        box.x1 = std::max(0, std::min(int(image_size_.width), int(cx - w / 2.f)));
                        box.y1 = std::max(0, std::min(int(image_size_.height), int(cy - h / 2.f)));
                        box.x2 = std::max(0, std::min(int(image_size_.width), int(cx + w / 2.f)));
                        box.y2 = std::max(0, std::min(int(image_size_.height), int(cy + h / 2.f)));
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
 * @brief 非极大值抑制（NMS）函数，去除重叠框
 * @param boxes 输入输出检测框列表，函数内部会对其进行修改保留最终框
 */
void AnchorFreeDet::nms(vector<ob_det_res>& boxes)
{
    // 按置信度从大到小排序
    std::sort(boxes.begin(), boxes.end(), [](const ob_det_res& a, const ob_det_res& b) { return a.score > b.score; });
    std::vector<bool> suppressed(boxes.size(), false); /**< 标记是否被抑制 */

    for (size_t i = 0; i < boxes.size(); i++) {
        if (suppressed[i]) continue; /**< 已被抑制则跳过 */
        const ob_det_res& a = boxes[i];
        for (size_t j = i + 1; j < boxes.size(); j++) {
            if (suppressed[j]) continue;
            const ob_det_res& b = boxes[j];
            // 计算两个框的交集坐标
            float xx1 = std::max(a.x1, b.x1);
            float yy1 = std::max(a.y1, b.y1);
            float xx2 = std::min(a.x2, b.x2);
            float yy2 = std::min(a.y2, b.y2);
            float w = std::max(0.f, xx2 - xx1 + 1);
            float h = std::max(0.f, yy2 - yy1 + 1);
            float inter = w * h; /**< 交集面积 */
            float areaA = (a.x2 - a.x1 + 1) * (a.y2 - a.y1 + 1); /**< 框a面积 */
            float areaB = (b.x2 - b.x1 + 1) * (b.y2 - b.y1 + 1); /**< 框b面积 */
            float iou = inter / (areaA + areaB - inter); /**< 计算IOU */

            if (iou >= ob_nms_thresh) {
                suppressed[j] = true; /**< 重叠过大，抑制该框 */
            }
        }
    }

    // 过滤出未被抑制的框
    vector<ob_det_res> filtered_boxes;
    for (size_t i = 0; i < boxes.size(); i++) {
        if (!suppressed[i]) filtered_boxes.push_back(boxes[i]);
    }
    boxes = std::move(filtered_boxes);
}

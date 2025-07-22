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

#include "gfl_det.h"

/**
 * @brief GFLDet构造函数，初始化模型及相关参数
 * @param args 配置参数，包括模型路径、阈值、标签、步长等
 * @param isp_shape 输入图像尺寸（通道，高，宽）
 * @param debug_mode 调试模式标志
 */
GFLDet::GFLDet(config_args args, FrameCHWSize isp_shape, const int debug_mode)
:AIBase(args.kmodel_path.c_str(),"GFLDet", debug_mode)
{
    ob_det_thresh = args.obj_thresh;       /**< 目标检测置信度阈值 */
    ob_nms_thresh = args.nms_thresh;       /**< NMS阈值 */
    labels = args.labels;                   /**< 类别标签 */
    nms_option = args.nms_option;           /**< 是否按类别单独做NMS */
    num_class = labels.size();              /**< 类别数量 */
    reg_max = REG_MAX;                      /**< 回归最大值 */
    memcpy(this->strides, args.strides, sizeof(args.strides)); /**< 复制步长数组 */
    input_width = input_shapes_[0][3];     /**< 网络输入宽 */
    input_height = input_shapes_[0][2];    /**< 网络输入高 */
    generate_grid_center_priors();          /**< 生成各层格点中心prior */

    image_size_ = {isp_shape.channel, isp_shape.height, isp_shape.width};  /**< 输入图像尺寸 */
    input_size_ = {input_shapes_[0][1], input_shapes_[0][2], input_shapes_[0][3]}; /**< 网络输入尺寸 */
    ai2d_out_tensor_ = this->get_input_tensor(0); /**< 获取网络输入tensor */
    // 设置图像预处理（padding + resize，灰色填充）
    Utils::padding_resize_two_side_set(image_size_, input_size_, ai2d_builder_, cv::Scalar(114,114,114));
}

/**
 * @brief GFLDet析构函数
 */
GFLDet::~GFLDet()
{
}

/**
 * @brief 图像预处理，调用ai2d_builder进行处理
 * @param input_tensor 输入tensor数据
 */
void GFLDet::pre_process(runtime_tensor &input_tensor){
    ScopedTiming st(model_name_ + " pre_process", debug_mode_);
    ai2d_builder_->invoke(input_tensor, ai2d_out_tensor_).expect("error occurred in ai2d running");
}

/**
 * @brief 模型推理执行，包含运行和获取输出
 */
void GFLDet::inference()
{
    this->run();
    this->get_output();
}

/**
 * @brief 后处理，解码网络输出并执行NMS
 * @param results 输出检测框结果容器
 */
void GFLDet::post_process(vector<ob_det_res> &results)
{
    ScopedTiming st(model_name_ + " post_process", debug_mode_);
    float* output_0 = p_outputs_[0]; /**< 网络输出0 */
    float* output_1 = p_outputs_[1]; /**< 网络输出1 */
    float* output_2 = p_outputs_[2]; /**< 网络输出2 */

    if(nms_option)  /**< 按所有类别一起NMS */
    {
        vector<ob_det_res> b0, b1, b2;

        decode_infer(output_0, center_priors[0], b0); /**< 解码第0层 */
        decode_infer(output_1, center_priors[1], b1); /**< 解码第1层 */
        decode_infer(output_2, center_priors[2], b2); /**< 解码第2层 */

        // 合并所有层框
        results.insert(results.begin(), b0.begin(), b0.end());
        results.insert(results.begin(), b1.begin(), b1.end());
        results.insert(results.begin(), b2.begin(), b2.end());

        nms(results);  /**< 执行NMS */
    }
    else  /**< 按类别分别NMS */
    {
        vector<vector<ob_det_res>> b0, b1, b2;
        for (int i = 0; i < num_class; i++)
        {
            b0.push_back(vector<ob_det_res>()); /**< 初始化二维容器，按类别存放检测框 */
            b1.push_back(vector<ob_det_res>());
            b2.push_back(vector<ob_det_res>());
        }

        decode_infer_class(output_0, center_priors[0], b0);
        decode_infer_class(output_1, center_priors[1], b1);
        decode_infer_class(output_2, center_priors[2], b2);

        // 合并各层同类框并执行NMS，最终加入results
        for(int i = 0; i < num_class; i++)
        {
            b0[i].insert(b0[i].begin(), b1[i].begin(), b1[i].end());
            b0[i].insert(b0[i].begin(), b2[i].begin(), b2[i].end());
            nms(b0[i]);
            results.insert(results.begin(), b0[i].begin(), b0[i].end());
        }
    }

}

/**
 * @brief 生成不同尺度下的格点中心prior
 * @note 每个尺度对应一个步长strides[i]
 */
void GFLDet::generate_grid_center_priors()
{
    for (int i = 0; i < STAGE_NUM; i++)
    {
        int stride = strides[i];  /**< 当前尺度的步长 */
        int feat_w = ceil((float)input_width / stride);   /**< 计算特征图宽 */
        int feat_h = ceil((float)input_height / stride);  /**< 计算特征图高 */
        for (int y = 0; y < feat_h; y++)
            for (int x = 0; x < feat_w; x++)
            {
                CenterPrior ct;
                ct.x = x;           /**< 格点x坐标 */
                ct.y = y;           /**< 格点y坐标 */
                ct.stride = stride; /**< 当前尺度步长 */
                center_priors[i].push_back(ct); /**< 添加到prior列表 */
            }
        
    }
}

/**
 * @brief 快速计算指数函数，用于sigmoid函数计算中
 * @param x 输入值
 * @return 返回近似exp(x)值
 */
float GFLDet::fast_exp(float x)
{
    union {
        uint32_t i;
        float f;
    } v{};
    v.i = (1 << 23) * (1.4426950409 * x + 126.93490512f); /**< 快速指数近似实现 */
    return v.f;
}

/**
 * @brief sigmoid激活函数
 * @param x 输入值
 * @return sigmoid(x)输出
 */
float GFLDet::sigmoid(float x)
{
    return 1.0f / (1.0f + fast_exp(-x));
}

/**
 * @brief 解码网络单输出层结果，合并所有类别
 * @param pred 网络输出指针
 * @param center_priors 当前尺度的中心prior列表
 * @param results 输出检测框列表
 */
void GFLDet::decode_infer(float* pred, std::vector<CenterPrior>& center_priors,  std::vector<ob_det_res>& results)
{
    float ratiow = (float)input_width / image_size_.width;
    float ratioh = (float)input_height / image_size_.height;
    float gain = ratiow < ratioh ? ratiow : ratioh; /**< 计算缩放比例 */
    const int num_points = center_priors.size(); /**< 格点数量 */
    const int num_channels = num_class + (reg_max + 1) * 4; /**< 每点输出通道数 */
    for (int idx = 0; idx < num_points; idx++)
    {
        int ct_x = center_priors[idx].x;      /**< 当前格点x */
        int ct_y = center_priors[idx].y;      /**< 当前格点y */
        int stride = center_priors[idx].stride; /**< 步长 */
        float score = 0;
        int cur_label = 0;

        // 遍历类别，找最大score和对应类别
        for (int label = 0; label < num_class; label++)
        {
            float sig_score = sigmoid(pred[idx * num_channels + label]); /**< 置信度sigmoid */
            if (sig_score > score)
            {
                score = sig_score;
                cur_label = label;
            }
        }

        if (score > ob_det_thresh) /**< 置信度阈值过滤 */
        {
            const float* bbox_pred = pred + idx * num_channels + num_class; /**< 边框回归起始位置 */
            // 解码预测框，转为ob_det_res格式，加入结果
            results.push_back(disPred2Bbox(bbox_pred, cur_label, score, ct_x, ct_y, stride, reg_max, input_height, input_width, ratiow, ratioh, gain));
        }
    }
}

/**
 * @brief 解码网络单输出层结果，按类别分开存储
 * @param pred 网络输出指针
 * @param center_priors 当前尺度的中心prior列表
 * @param results 按类别存储的检测框二维数组
 */
void GFLDet::decode_infer_class(float* pred, std::vector<CenterPrior>& center_priors,  std::vector<std::vector<ob_det_res>>& results)
{
    float ratiow = (float)input_width / image_size_.width;
    float ratioh = (float)input_height / image_size_.height;
    float gain = ratiow < ratioh ? ratiow : ratioh;
    const int num_points = center_priors.size();
    const int num_channels = num_class + (reg_max + 1) * 4;
    for (int idx = 0; idx < num_points; idx++)
    {
        int ct_x = center_priors[idx].x;
        int ct_y = center_priors[idx].y;
        int stride = center_priors[idx].stride;
        float score = 0;
        int cur_label = 0;

        for (int label = 0; label < num_class; label++)
        {
            float sig_score = sigmoid(pred[idx * num_channels + label]);
            if (sig_score > score)
            {
                score = sig_score;
                cur_label = label;
            }
        }

        if (score > ob_det_thresh)
        {
            const float* bbox_pred = pred + idx * num_channels + num_class;
            ob_det_res tmp_res = disPred2Bbox(bbox_pred, cur_label, score, ct_x, ct_y, stride, reg_max, input_height, input_width, ratiow, ratioh, gain);
            results[tmp_res.label_index].push_back(tmp_res); /**< 根据类别索引存放 */
        }
    }
}

/**
 * @brief 根据disPred分布回归预测转换为检测框
 * @param dfl_det 预测的边框分布回归数据指针
 * @param label 目标类别索引
 * @param score 目标置信度
 * @param x 格点x坐标
 * @param y 格点y坐标
 * @param stride 当前步长
 * @param reg_max 回归最大值
 * @param input_height 网络输入高度
 * @param input_width 网络输入宽度
 * @param ratiow 宽度缩放比例
 * @param ratioh 高度缩放比例
 * @param gain 缩放gain值（用于去padding）
 * @return 返回转换后的检测框ob_det_res
 */
ob_det_res GFLDet::disPred2Bbox(const float*& dfl_det, int label, float score, int x, int y, int stride, int reg_max, int input_height,int input_width,float ratiow, float ratioh, float gain)
{
    float ct_x = x * stride; /**< 格点映射到原图坐标 */
    float ct_y = y * stride;

    // 去除padding并缩放回原图坐标
    ct_x -= ((input_width - image_size_.width * gain) / 2);
    ct_y -= ((input_height - image_size_.height * gain) / 2);
    ct_x /= gain;
    ct_y /= gain;

    std::vector<float> dis_pred;
    dis_pred.resize(4); /**< 存储四个边界距离预测 */

    for (int i = 0; i < 4; i++)
    {
        float dis = 0;
        float* dis_after_sm = new float[reg_max + 1];
        // 对边界回归进行softmax处理，得到概率分布
        activation_function_softmax(dfl_det + i * (reg_max + 1), dis_after_sm, reg_max + 1);
        for (int j = 0; j < reg_max + 1; j++)
            dis += j * dis_after_sm[j];  /**< 计算边界的期望值 */
        
        dis *= stride;  /**< 缩放回原始尺寸 */
        dis_pred[i] = dis;
        delete[] dis_after_sm;
    }
    // 计算框坐标并裁剪到图像范围内
    float xmin = (std::max)(ct_x - dis_pred[0] / gain, .0f);
    float ymin = (std::max)(ct_y - dis_pred[1] / gain, .0f);
    float xmax = (std::min)(ct_x + dis_pred[2] / gain, (float)image_size_.width);
    float ymax = (std::min)(ct_y + dis_pred[3] / gain, (float)image_size_.height);

    return ob_det_res{ xmin, ymin, xmax, ymax, score, label, labels[label] };
}

/**
 * @brief softmax激活函数模板实现
 * @tparam _Tp 数据类型
 * @param src 输入数组指针
 * @param dst 输出数组指针（归一化概率）
 * @param length 数组长度
 * @return 返回0，表示成功
 */
template<typename _Tp>
int GFLDet::activation_function_softmax(const _Tp* src, _Tp* dst, int length)
{
    const _Tp alpha = *std::max_element(src, src + length); /**< 防止指数溢出 */
    _Tp denominator{ 0 };

    for (int i = 0; i < length; ++i)
    {
        dst[i] = fast_exp(src[i] - alpha); /**< 计算exp(x - max) */
        denominator += dst[i];
    }

    for (int i = 0; i < length; ++i)
    {
        dst[i] /= denominator; /**< 归一化 */
    }

    return 0;
}

/**
 * @brief 非极大值抑制（NMS）函数，去除重叠框
 * @param input_boxes 输入输出检测框列表，函数内部会修改为最终结果
 */
void GFLDet::nms(std::vector<ob_det_res>& input_boxes)
{
    // 按置信度从大到小排序
    std::sort(input_boxes.begin(), input_boxes.end(), [](ob_det_res a, ob_det_res b) { return a.score > b.score; });
    std::vector<float> vArea(input_boxes.size()); /**< 存储每个框面积 */

    for (int i = 0; i < int(input_boxes.size()); ++i)
        vArea[i] = (input_boxes.at(i).x2 - input_boxes.at(i).x1 + 1) * (input_boxes.at(i).y2 - input_boxes.at(i).y1 + 1);
    
    for (int i = 0; i < int(input_boxes.size()); ++i)
        for (int j = i + 1; j < int(input_boxes.size());)
        {
            // 计算两个框的交集坐标
            float xx1 = std::max(input_boxes[i].x1, input_boxes[j].x1);
            float yy1 = std::max(input_boxes[i].y1, input_boxes[j].y1);
            float xx2 = std::min(input_boxes[i].x2, input_boxes[j].x2);
            float yy2 = std::min(input_boxes[i].y2, input_boxes[j].y2);
            float w = std::max(float(0), xx2 - xx1 + 1);
            float h = std::max(float(0), yy2 - yy1 + 1);
            float inter = w * h;  /**< 交集面积 */
            float ovr = inter / (vArea[i] + vArea[j] - inter); /**< 计算IOU */
            if (ovr >= ob_nms_thresh) /**< IOU超过阈值，抑制框 */
            {
                input_boxes.erase(input_boxes.begin() + j);
                vArea.erase(vArea.begin() + j);
            }
            else
                j++;
        }
}

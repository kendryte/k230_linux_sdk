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

#ifndef _MULTILABEL_CLS_H
#define _MULTILABEL_CLS_H

#include "ai_utils.h"
#include "ai_base.h"
#include "parse_args.h"
#include "text_paint.h"

/**
 * @brief 多标签分类结果结构
 */
typedef struct multi_lable_res
{
    vector<float> score_vec;//多标签分类的分数集合
    vector<int> id_vec;//多标签分类的判别结果结合
    vector<string> labels;//多标签分类的名称集合
}multi_lable_res;


/**
 * @brief 多标签分类任务
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class MultilabelCls : public AIBase
{
    public:
    /**
    * @brief MultilabelCls构造函数，加载kmodel,并初始化kmodel输入、输出和多标签分类阈值
    * @param args        构建对象需要的参数，config.json文件（包含多标签阈值，kmodel路径等）
    * @param isp_shape   isp输入大小（chw）
    * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
    * @return None
    */
    MultilabelCls(config_args args, FrameCHWSize isp_shape, const int debug_mode);
	
    /**
    * @brief MultilabelCls析构函数
    * @return None
    */
    ~MultilabelCls();

    void pre_process(runtime_tensor &input_tensor);

    /**
    * @brief kmodel推理
    * @return None
    */
    void inference();

    /**
    * @brief kmodel推理结果后处理
    * @param results 后处理之后的基于原始图像的多标签分类结果集合
    * @return None
    */
	void post_process(vector<multi_lable_res>& results);

    /**
    * @brief 绘制多标签分类结果
    * @param draw_frame 绘制结果的图像
    * @param results 多标签分类结果集合
    * @return None
    */
    static void draw_result(cv::Mat &draw_frame, vector<multi_lable_res> &results,TextRenderer &writepen);

    private:

    /**
    * @brief 计算exp
    * @param x 自变量值
    * @return 返回计算exp后的结果
    */
    float fast_exp(float x);

    /**
    * @brief 计算sigmoid
    * @param x 自变量值
    * @return 返回计算sigmoid后的结果
    */
    float sigmoid(float x);

    std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
    runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
    FrameCHWSize image_size_;   //原始图像尺寸（chw）
    FrameCHWSize input_size_;   //模型输入尺寸（chw）

	vector<string> labels; //类别名字
	float cls_thresh;      //分类阈值
    int num_class;         //类别数
};
#endif
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

#ifndef _GFL_DET_H
#define _GFL_DET_H

#include "ai_utils.h"
#include "ai_base.h"
#include "parse_args.h"
#include "result.h"

#define REG_MAX 16
#define STRIDE_NUM 3
#define STAGE_NUM 3

class GFLDet : public AIBase
{
    public:
    /**
    * @brief GFLDet构造函数，加载kmodel,并初始化kmodel输入、输出和多目标检测阈值
    * @param args        构建对象需要的参数，config.json文件（包含检测阈值，kmodel路径等）
    * @param isp_shape   isp输入大小（chw）
    * @param debug_mode  0（不调试）、 1（只显示时间）、2（显示所有打印信息）
    * @return None
    */
    GFLDet(config_args args, FrameCHWSize isp_shape, const int debug_mode);
    
    /**
    * @brief GFLDet析构函数
    * @return None
    */
    ~GFLDet();

    void pre_process(runtime_tensor &input_tensor);

    /**
    * @brief kmodel推理
    * @return None
    */
    void inference();

    /**
    * @brief kmodel推理结果后处理
    * @param frame_size 原始图像/帧宽高，用于将结果放到原始图像大小
    * @param results 后处理之后的基于原始图像的检测结果集合
    * @return None
    */
    void post_process(vector<ob_det_res> &results);

    private:

    /**
    * @brief 计算softmax
    * @param src 源泛型
    * @param dst 目标泛型
    * @param length 数组长度
    * @return 返回0数值
    */
    template<typename _Tp>
	int activation_function_softmax(const _Tp* src, _Tp* dst, int length);

    /**
    * @brief 获得最佳检测框
    * @param dfl_det
    * @param label
    * @param score
    * @param x
    * @param y
    * @param stride
    * @param reg_max
    * @param input_height
    * @param input_width
    * @param ratiow
    * @param ratioh
    * @param gain
    * @return 返回检测框集合
    */
	ob_det_res disPred2Bbox(const float*& dfl_det, int label, float score, int x, int y, int stride, int reg_max, int input_height,int input_width,float ratiow, float ratioh, float gain);
	
    /**
    * @brief 生成中心点
    * @return None
    */
    void generate_grid_center_priors();

    /**
    * @brief 检查结果的初步处理
    * @param pred 模型输出一层的头指针
    * @param center_priors 预设中心点
    * @param results 初步处理后的检测框集合
    * @return None
    */
	void decode_infer(float* pred, std::vector<CenterPrior>& center_priors,  std::vector<ob_det_res>& results);

    /**
    * @brief 检查结果的初步处理
    * @param pred 模型输出一层的头指针
    * @param center_priors 预设中心点
    * @param results 初步处理后的检测框集合
    * @return None
    */
	void decode_infer_class(float* pred, std::vector<CenterPrior>& center_priors,  std::vector<std::vector<ob_det_res>>& results);
	
    /**
    * @brief 对检测结果进行非最大值抑制
    * @param input_boxes 检测框集合
    * @return None
    */
    void nms(std::vector<ob_det_res>& input_boxes);

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
    FrameCHWSize image_size_;   //原始图像大小
    FrameCHWSize input_size_;   //模型输入大小

    float ob_det_thresh;   //检测框分数阈值
    float ob_nms_thresh;   //nms阈值
    vector<string> labels; //类别名字
    int num_class;         //类别数
    int reg_max;           //划分区域值
	int strides[STRIDE_NUM];//每层检测结果的分辨率缩减被数
    vector<CenterPrior> center_priors[3];//预设中心点
    bool nms_option;       //nms选项，区分类内nms和类间nms

    int input_height;      //模型输入高
    int input_width;       //模型输入宽
};
#endif
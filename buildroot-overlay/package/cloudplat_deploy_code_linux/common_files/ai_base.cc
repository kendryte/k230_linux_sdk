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
#include "ai_base.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <nncase/runtime/debug.h>
#include "ai_utils.h"

using std::cout;
using std::endl;
using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::k230;
using namespace nncase::F::k230;
using namespace nncase::runtime::detail;

/**
 * @brief 构造函数，加载 kmodel 模型并初始化输入输出信息。
 * 
 * @param kmodel_file 模型文件路径（.kmodel 格式）
 * @param model_name 模型名称，用于调试信息打印
 * @param debug_mode 调试等级（0=关闭，1=时间打印，2=详细调试信息）
 */
AIBase::AIBase(const char *kmodel_file, const string model_name, const int debug_mode)
    : debug_mode_(debug_mode), model_name_(model_name)
{
    if (debug_mode > 1)
        cout << "kmodel_file:" << kmodel_file << endl;

    std::ifstream ifs(kmodel_file, std::ios::binary);
    kmodel_interp_.load_model(ifs).expect("Invalid kmodel");

    set_input_init();
    set_output_init();
}

/**
 * @brief 析构函数，当前无特殊资源释放逻辑。
 */
AIBase::~AIBase()
{
}

/**
 * @brief 初始化模型输入张量结构和形状信息。
 * 
 * @note 本函数在构造时自动调用，仅初始化一次。
 */
void AIBase::set_input_init()
{
    ScopedTiming st(model_name_ + " set_input init", debug_mode_);

    for (int i = 0; i < kmodel_interp_.inputs_size(); ++i)
    {
        auto desc = kmodel_interp_.input_desc(i);
        auto shape = kmodel_interp_.input_shape(i);

        // 创建 host 端 tensor 并设置为模型输入
        auto tensor = host_runtime_tensor::create(desc.datatype, shape, hrt::pool_shared)
                          .expect("cannot create input tensor");
        kmodel_interp_.input_tensor(i, tensor).expect("cannot set input tensor");

        vector<int> in_shape;
        if (debug_mode_ > 1)
            cout << "input " << std::to_string(i) << " : " << to_string(desc.datatype) << ",";

        for (int j = 0; j < shape.size(); ++j)
        {
            in_shape.push_back(shape[j]);
            if (debug_mode_ > 1)
                cout << shape[j] << ",";
        }

        if (debug_mode_ > 1)
            cout << endl;

        input_shapes_.push_back(in_shape);
    }
}

/**
 * @brief 获取指定索引的输入张量。
 * 
 * @param idx 输入张量索引
 * @return runtime_tensor 输入张量对象
 */
runtime_tensor AIBase::get_input_tensor(size_t idx)
{
    return kmodel_interp_.input_tensor(idx).expect("cannot get input tensor");
}

/**
 * @brief 设置指定索引的输入张量。
 * 
 * @param idx 输入张量索引
 * @param input_tensor 要绑定的输入张量
 */
void AIBase::set_input_tensor(size_t idx, runtime_tensor &input_tensor)
{
    kmodel_interp_.input_tensor(idx, input_tensor).expect("cannot set input tensor");
}

/**
 * @brief 初始化模型输出张量结构和形状信息。
 * 
 * @note 本函数在构造时自动调用，仅初始化一次。
 */
void AIBase::set_output_init()
{
    ScopedTiming st(model_name_ + " set_output_init", debug_mode_);

    for (size_t i = 0; i < kmodel_interp_.outputs_size(); i++)
    {
        auto desc = kmodel_interp_.output_desc(i);
        auto shape = kmodel_interp_.output_shape(i);

        vector<int> out_shape;
        if (debug_mode_ > 1)
            cout << "output " << std::to_string(i) << " : " << to_string(desc.datatype) << ",";

        for (int j = 0; j < shape.size(); ++j)
        {
            out_shape.push_back(shape[j]);
            if (debug_mode_ > 1)
                cout << shape[j] << ",";
        }

        if (debug_mode_ > 1)
            cout << endl;

        output_shapes_.push_back(out_shape);
    }
}

/**
 * @brief 执行一次模型推理。
 * 
 * @throws 如果模型执行失败，将抛出错误。
 */
void AIBase::run()
{
    ScopedTiming st(model_name_ + " run", debug_mode_);
    kmodel_interp_.run().expect("error occurred in running model");
}

/**
 * @brief 从模型中获取输出数据，并以 float* 形式存储在 p_outputs_ 中。
 */
void AIBase::get_output()
{
    ScopedTiming st(model_name_ + " get_output", debug_mode_);

    p_outputs_.clear();

    for (int i = 0; i < kmodel_interp_.outputs_size(); i++)
    {
        auto out = kmodel_interp_.output_tensor(i).expect("cannot get output tensor");

        // 获取 host buffer 并映射成 float* 指针
        auto buf = out.impl()->to_host()
                       .unwrap()
                       ->buffer()
                       .as_host()
                       .unwrap()
                       .map(map_access_::map_read)
                       .unwrap()
                       .buffer();

        float *p_out = reinterpret_cast<float *>(buf.data());
        p_outputs_.push_back(p_out);
    }
}

/**
 * @brief 获取指定索引的输出张量对象。
 * 
 * @param idx 输出张量索引
 * @return runtime_tensor 输出张量对象
 */
runtime_tensor AIBase::get_output_tensor(int idx)
{
    return kmodel_interp_.output_tensor(idx).expect("cannot get current output tensor");
}

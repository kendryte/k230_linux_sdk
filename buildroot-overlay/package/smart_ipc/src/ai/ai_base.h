/* Copyright (c) 2025, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file ai_base.h
 * @brief 定义 AIBase 基类，封装基于 nncase 的 Kmodel 推理逻辑
 * @author 
 * @date 2025
 */

#ifndef AI_BASE_H
#define AI_BASE_H

#include <vector>
#include <string>
#include <fstream>

#include <nncase/runtime/interpreter.h>
#include <nncase/runtime/runtime_op_utility.h>
#include <nncase/runtime/util.h>
#include "scoped_timing.h"

using std::string;
using std::vector;
using namespace nncase::runtime;

/**
 * @class AIBase
 * @brief AI 模型推理基类，封装了 Kmodel 模型的加载、输入输出管理和推理流程。
 *
 * 该类为 nncase 推理框架的封装，提供模型初始化、输入输出张量管理、推理执行等通用接口。
 * 子类可以继承该类并扩展具体模型的前处理与后处理逻辑。
 */
class AIBase
{
public:
    /**
     * @brief 构造函数，加载 Kmodel 模型并初始化输入输出信息。
     * @param kmodel_file Kmodel 模型文件路径
     * @param model_name 模型名称（用于日志标识）
     * @param debug_mode 调试模式（0：无日志，1：打印耗时，2：打印详细信息）
     */
    AIBase(const char *kmodel_file, const string model_name, const int debug_mode = 1);

    /**
     * @brief 析构函数，释放资源。
     */
    ~AIBase();

    /**
     * @brief 获取指定索引的输入张量。
     * @param idx 输入张量的索引（从 0 开始）
     * @return 对应的 runtime_tensor 输入张量对象
     */
    runtime_tensor get_input_tensor(size_t idx);

    /**
     * @brief 设置指定索引的输入张量。
     * @param idx 输入张量索引
     * @param input_tensor 输入张量的引用对象
     */
    void set_input_tensor(size_t idx, runtime_tensor &input_tensor);

    /**
     * @brief 执行一次前向推理。
     */
    void run();

    /**
     * @brief 获取模型推理结果并缓存到 p_outputs_ 中（float*）。
     */
    void get_output();

    /**
     * @brief 获取指定索引的输出张量。
     * @param idx 输出张量索引
     * @return 对应的 runtime_tensor 输出张量对象
     */
    runtime_tensor get_output_tensor(int idx);

protected:
    string model_name_;                        ///< 模型名称（日志用途）
    int debug_mode_;                           ///< 调试等级（0：无日志，1：打印耗时，2：详细信息）
    vector<float *> p_outputs_;                ///< 输出结果的缓存指针列表（每个 float* 指向一个输出张量）
    vector<vector<int>> input_shapes_;         ///< 每个输入张量的形状（维度）
    vector<vector<int>> output_shapes_;        ///< 每个输出张量的形状（维度）

private:
    /**
     * @brief 初始化模型输入信息（推理前调用一次）。
     */
    void set_input_init();

    /**
     * @brief 初始化模型输出信息（推理前调用一次）。
     */
    void set_output_init();

    interpreter kmodel_interp_;               ///< nncase 模型解释器对象
    vector<unsigned char> kmodel_vec_;        ///< Kmodel 模型二进制数据缓存
};

#endif // AI_BASE_H

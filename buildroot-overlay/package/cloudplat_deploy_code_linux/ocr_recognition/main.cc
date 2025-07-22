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
#include <iostream>
#include <thread>
#include "ai_utils.h"
#include "sensor_set.h"
#include "parse_args.h"
#include "sensor_buf_manager.h"
#include "ocr_reco.h"
#include "mmz.h"

using std::cerr;
using std::cout;
using std::endl;
using std::thread;


void print_usage(const char *name)
{
    cout << "Usage: " << name << "<kmodel_det> <obj_thres> <nms_thres> <input_mode> <debug_mode>" << endl
         << "Options:" << endl
         << "  config_path     deploy_config.json路径\n"
         << "  image_path      待推理图片路径\n"
         << "  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
         << "\n"
         << endl;
}

void __attribute__((destructor)) cleanup() {
    std::cout << "Cleaning up memory..." << std::endl;
    shrink_memory_pool();
    kd_mpi_mmz_deinit();
}


/**
 * @brief 主程序入口
 */
int main(int argc, char *argv[])
{
    // 输出编译信息
    std::cout << "程序名：" << argv[0] << " | 构建时间：" << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << "OCR识别任务只支持图片推理，不支持视频流推理!" << std::endl;
    // 检查参数数量是否合法（此处要求 9 个参数）
    if (argc != 4)
    {
        print_usage(argv[0]); // 打印用法说明
        return -1;
    }
    // 参数解析
    std::string config_path(argv[1]);
    std::string image_path(argv[2]);
    int debug_mode = atoi(argv[3]);
    config_args args;
    parse_args(config_path,args,debug_mode);
    // 读取图片
    cv::Mat ori_img = cv::imread(image_path);
    FrameCHWSize image_size={ori_img.channels(),ori_img.rows,ori_img.cols};
        // 创建一个空的向量，用于存储chw图像数据,将读入的hwc数据转换成chw数据
    std::vector<uint8_t> chw_vec;
    std::vector<cv::Mat> bgrChannels(3);
    cv::split(ori_img, bgrChannels);
    for (auto i = 2; i > -1; i--)
    {
        std::vector<uint8_t> data = std::vector<uint8_t>(bgrChannels[i].reshape(1, 1));
        chw_vec.insert(chw_vec.end(), data.begin(), data.end());
    }
    // 创建tensor
    dims_t in_shape { 1, 3, ori_img.rows, ori_img.cols };
    runtime_tensor input_tensor = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("cannot create input tensor");
    auto input_buf = input_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(input_buf.data()), chw_vec.data(), chw_vec.size());
    hrt::sync(input_tensor, sync_op_t::sync_write_back, true).expect("write back input failed");
    OCRReco ocr_rec(args,image_size,debug_mode);
    ocr_rec.pre_process(input_tensor);
    ocr_rec.inference();
    ocr_rec.post_process();
    return 0;
}

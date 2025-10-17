
#include <iostream>
#include <chrono>
#include <fstream>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <nncase/runtime/runtime_tensor.h>
#include <nncase/runtime/interpreter.h>
#include <nncase/runtime/runtime_op_utility.h>
#include <nncase/functional/ai2d/ai2d_builder.h>
#include <nncase/runtime/util.h>
#include "mmz.h"

using std::string;
using std::vector;
using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::k230;
using namespace nncase::F::k230;

void __attribute__((destructor)) cleanup() {
    std::cout << "Cleaning up memory..." << std::endl;
    shrink_memory_pool();
    kd_mpi_mmz_deinit();
}

int main(int argc, char *argv[])
{
    std::cout << "case " << argv[0] << " build " << __DATE__ << " " << __TIME__ << std::endl;
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <image> <debug_mode 0,1,2>" << std::endl;
        return -1;
    }

    int debug_mode=atoi(argv[2]);

    std::string kmodel_file = "test.kmodel";
    interpreter kmodel_interp_; 
    std::ifstream ifs(kmodel_file, std::ios::binary);
    kmodel_interp_.load_model(ifs).expect("Invalid kmodel");

    // 读入图片，并将数据处理成CHW和RGB格式
    cv::Mat ori_img = cv::imread(argv[1]);
    int ori_w = ori_img.cols;
    int ori_h = ori_img.rows;
    std::vector<uint8_t> chw_vec;
    std::vector<cv::Mat> bgrChannels(3);
    cv::split(ori_img, bgrChannels);
    for (auto i = 2; i > -1; i--)
    {
        std::vector<uint8_t> data = std::vector<uint8_t>(bgrChannels[i].reshape(1, 1));
        chw_vec.insert(chw_vec.end(), data.begin(), data.end());
    }

    // 创建AI2D输入tensor，并将CHW_RGB数据拷贝到tensor中，并回写到DDR
    dims_t ai2d_in_shape{1, 3, ori_h, ori_w};
    runtime_tensor ai2d_in_tensor = host_runtime_tensor::create(typecode_t::dt_uint8, ai2d_in_shape, hrt::pool_shared).expect("cannot create input tensor");
    auto input_buf = ai2d_in_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<char *>(input_buf.data()), chw_vec.data(), chw_vec.size());
    hrt::sync(ai2d_in_tensor, sync_op_t::sync_write_back, true).expect("write back input failed");

    // resize参数
    int out_w=640;
    int out_h=320;
    int size=out_w*out_h;

    // 创建AI2D输出tensor
    dims_t ai2d_out_shape{1, 3, out_h, out_w};
    runtime_tensor ai2d_out_tensor = host_runtime_tensor::create(typecode_t::dt_uint8, ai2d_out_shape, hrt::pool_shared).expect("cannot create input tensor");

    // 设置AI2D参数，AI2D支持5种预处理方法，crop/shift/pad/resize/affine。这里开启resize
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, ai2d_in_tensor.datatype(), ai2d_out_tensor.datatype()};
    ai2d_crop_param_t crop_param{false, 0, 0, 0, 0};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, ai2d_pad_mode::constant, {0,0,0}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};

    // 构造ai2d_builder
    ai2d_builder builder(ai2d_in_shape, ai2d_out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param);
    builder.build_schedule();
    // 执行ai2d，实现从ai2d_in_tensor->ai2d_out_tensor的预处理过程
    builder.invoke(ai2d_in_tensor,ai2d_out_tensor).expect("error occurred in ai2d running");
    //获取处理结果，并将结果存成图片
    auto output_buf = ai2d_out_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    cv::Mat image_r = cv::Mat(out_h, out_w, CV_8UC1, output_buf.data());
    cv::Mat image_g = cv::Mat(out_h, out_w, CV_8UC1, output_buf.data()+size);
    cv::Mat image_b = cv::Mat(out_h, out_w, CV_8UC1, output_buf.data()+2*size);
    
    std::vector<cv::Mat> color_vec(3);
    color_vec.clear();
    color_vec.push_back(image_b);
    color_vec.push_back(image_g);
    color_vec.push_back(image_r);
    cv::Mat color_img;
    cv::merge(color_vec, color_img);
    cv::imwrite("test_resize.jpg", color_img);

    return 0;
}
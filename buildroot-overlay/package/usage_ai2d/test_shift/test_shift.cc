
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
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <debug_mode:0,1,2>" << std::endl;
        return -1;
    }

    int debug_mode=atoi(argv[1]);

    // 创建一张位深为16的原始数据，初始化为240
    cv::Mat ori_img(320, 320, CV_16UC3, cv::Scalar(240, 240, 240));
    cv::imwrite("ori_img.jpg",ori_img);

    //hwc,bgr
    int ori_w = ori_img.cols;
    int ori_h = ori_img.rows;
    
    // 创建AI2D输入tensor
    dims_t ai2d_in_shape{1,ori_h, ori_w,3};
    runtime_tensor ai2d_in_tensor = host_runtime_tensor::create(typecode_t::dt_uint16, ai2d_in_shape, hrt::pool_shared).expect("cannot create input tensor");
    auto input_buf = ai2d_in_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    memcpy(reinterpret_cast<uint16_t *>(input_buf.data()), ori_img.data , ori_h*ori_w*3*sizeof(uint16_t));
    hrt::sync(ai2d_in_tensor, sync_op_t::sync_write_back, true).expect("write back input failed");

    int out_w=ori_w;
    int out_h=ori_h;
    // 创建AI2D输出tensor
    dims_t ai2d_out_shape{1,out_h, out_w,3};
    runtime_tensor ai2d_out_tensor = host_runtime_tensor::create(typecode_t::dt_uint16, ai2d_out_shape, hrt::pool_shared).expect("cannot create input tensor");

    // 设置AI2D参数，AI2D支持5种预处理方法，crop/shift/pad/resize/affine。这里开启shift,右移1位，数据变为原来的1/2
    ai2d_datatype_t ai2d_dtype{ai2d_format::RAW16, ai2d_format::RAW16, ai2d_in_tensor.datatype(), ai2d_out_tensor.datatype()};
    ai2d_crop_param_t crop_param{false, 0, 0, 0, 0};
    ai2d_shift_param_t shift_param{true, 1};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, ai2d_pad_mode::constant, {0,0,0}};
    ai2d_resize_param_t resize_param{false, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};

    // 构造ai2d_builder
    ai2d_builder builder(ai2d_in_shape, ai2d_out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param);
    builder.build_schedule();
    // 执行ai2d，实现从ai2d_in_tensor->ai2d_out_tensor的预处理过程
    builder.invoke(ai2d_in_tensor,ai2d_out_tensor).expect("error occurred in ai2d running");

    //获取处理结果，并将结果存成图片
    auto output_buf = ai2d_out_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
    cv::Mat image_r = cv::Mat(out_h, out_w, CV_16UC3, output_buf.data());
    cv::imwrite("test_shift.jpg", image_r);

    return 0;
}
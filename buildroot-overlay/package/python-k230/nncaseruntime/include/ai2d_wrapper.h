#include <chrono>
#include <csignal>
#include <exception>
#include <fstream>
#include <iostream>
#include <nncase/functional/ai2d/ai2d_builder.h>
// #include <nncase/runtime/interpreter.h>
#include <nncase/runtime/runtime_op_utility.h>
#include <nncase/runtime/util.h>
#include <string>

#include "mmz.h"

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::F::k230;

namespace nncaseruntime_AI2D_api{

class NNCASE_API AI2D{
public:
    AI2D();
    result<void> build(dims_t input_shape, dims_t output_shape);
    result<void> run(runtime_tensor &input_tensor, runtime_tensor &output_tensor);
    void set_datatype(ai2d_format src_format, ai2d_format dst_format, typecode_t src_type, typecode_t dst_type);
    void set_crop_param(bool crop_flag, int32_t start_x, int32_t start_y, int32_t width, int32_t height);
    void set_shift_param(bool shift_flag, int32_t shift_val);
    void set_pad_param(bool pad_flag, paddings_t paddings, ai2d_pad_mode pad_mode, std::vector<int32_t> pad_val);
    void set_resize_param(bool resize_flag, ai2d_interp_method interp_method, ai2d_interp_mode interp_mode);
    void set_affine_param(bool affine_flag, ai2d_interp_method interp_method, uint32_t cord_round, uint32_t bound_ind, int32_t bound_val, uint32_t bound_smooth, std::vector<float> M);

private:
    ai2d_builder* ai2d_builder_;
    ai2d_datatype_t ai2d_dtype_;
    ai2d_crop_param_t crop_param_;
    ai2d_shift_param_t shift_param_;
    ai2d_pad_param_t pad_param_;
    ai2d_resize_param_t resize_param_;
    ai2d_affine_param_t affine_param_;
};
}
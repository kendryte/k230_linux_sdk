#include "ai2d_wrapper.h"

using namespace nncaseruntime_AI2D_api;

AI2D::AI2D()
{
    ai2d_dtype_ = {ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, dt_uint8, dt_uint8};
    crop_param_ = { false, 0, 0, 0, 0 };
    shift_param_ = { false, 0 };
    pad_param_ = { false, { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } }, ai2d_pad_mode::constant, { 0, 0, 0 } };
    resize_param_ = { false, ai2d_interp_method::tf_nearest, ai2d_interp_mode::none };
    affine_param_ = { false, ai2d_interp_method::tf_nearest, 0, 0, 0, 0, { 0, 0, 0, 0, 0, 0 } };
}

void AI2D::set_datatype(ai2d_format src_format, ai2d_format dst_format, nncase::typecode_t src_type, nncase::typecode_t dst_type)
{
    ai2d_dtype_ = { src_format, dst_format, src_type, dst_type };
}

void AI2D::set_crop_param(bool crop_flag, int32_t start_x, int32_t start_y, int32_t width, int32_t height)
{
    crop_param_ = { crop_flag, start_x, start_y, width, height };
}

void AI2D::set_shift_param(bool shift_flag, int32_t shift_val)
{
    shift_param_ = { shift_flag, shift_val };
}

void AI2D::set_pad_param(bool pad_flag, paddings_t paddings, ai2d_pad_mode pad_mode, std::vector<int32_t> pad_val)
{
    pad_param_ = { pad_flag, paddings, pad_mode, pad_val };
}

void AI2D::set_resize_param(bool resize_flag, ai2d_interp_method interp_method, ai2d_interp_mode interp_mode)
{
    resize_param_ = { resize_flag, interp_method, interp_mode };
}

void AI2D::set_affine_param(bool affine_flag, ai2d_interp_method interp_method, uint32_t cord_round, uint32_t bound_ind, int32_t bound_val, uint32_t bound_smooth, std::vector<float> M)
{
    affine_param_ = { affine_flag, interp_method, cord_round, bound_ind, bound_val, bound_smooth, M };
}

result<void> AI2D::build(dims_t input_shape, dims_t output_shape)
{
    if(input_shape[3]<=32 && pad_param_.paddings[3].before>0)
        throw std::runtime_error("[ERROR] ai2d pad: input width is <=32, the left pad should not be set. You can set the right pad first, then set the left pad.");

    ai2d_builder_ = new ai2d_builder(input_shape, output_shape, ai2d_dtype_, crop_param_, shift_param_, pad_param_, resize_param_, affine_param_);
    ai2d_builder_->build_schedule().expect("error occurred in ai2d building");
    return ok();
}

result<void> AI2D::run(runtime_tensor &input_tensor, runtime_tensor &output_tensor)
{
    ai2d_builder_->invoke(input_tensor, output_tensor).expect("error occurred in ai2d running");
    return ok();
}

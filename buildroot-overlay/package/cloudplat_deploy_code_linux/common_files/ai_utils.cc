#include <iostream>
#include "ai_utils.h"

using std::ofstream;
using std::vector;

// 根据类别数使用模运算循环获取颜色
std::vector<cv::Scalar> getColorsForClasses(int num_classes) {
    std::vector<cv::Scalar> colors;
    int num_available_colors = color_four.size(); 
    for (int i = 0; i < num_classes; ++i) {
        colors.push_back(color_four[i % num_available_colors]);
    }
    return colors;
}

cv::Scalar getColorForIndex(int idx) {
    int num_available_colors = color_four.size(); 
    return color_four[idx % num_available_colors];
}

auto cache = cv::Mat::zeros(1, 1, CV_32FC1);

void Utils::read_binary_file_bin(std::string file_name,unsigned char *outi)
{
    std::ifstream ifs(file_name, std::ios::binary);
    ifs.seekg(0, ifs.end);
    size_t len = ifs.tellg();
    ifs.seekg(0, ifs.beg);
    ifs.read((char *)(outi), len);
    ifs.close();
}

void Utils::bin_2_mat(std::string bin_data_path, int mat_width, int mat_height, cv::Mat &image_argb)
{
    unsigned char *bin_data = new unsigned char[mat_width*mat_height*4];
    read_binary_file_bin(bin_data_path,bin_data);
    std::vector<Mat> image_argb_vec;
    image_argb_vec.push_back(cv::Mat(mat_height, mat_width, CV_8UC1, bin_data));
    image_argb_vec.push_back(cv::Mat(mat_height, mat_width, CV_8UC1, bin_data + 1 * mat_width * mat_height));
    image_argb_vec.push_back(cv::Mat(mat_height, mat_width, CV_8UC1, bin_data + 2 * mat_width * mat_height));
    image_argb_vec.push_back(cv::Mat(mat_height, mat_width, CV_8UC1, bin_data + 3 * mat_width * mat_height));
    cv::merge(image_argb_vec, image_argb);
    delete[] bin_data;
}

void Utils::bin_2_mat_bgra(std::string bin_data_path, int mat_width, int mat_height, cv::Mat &image_bgra)
{
    unsigned char *bin_data = new unsigned char[mat_width*mat_height*4];
    read_binary_file_bin(bin_data_path,bin_data);
    std::vector<Mat> image_bgra_vec;
    image_bgra_vec.push_back(cv::Mat(mat_height, mat_width, CV_8UC1, bin_data + 3 * mat_width * mat_height));
    image_bgra_vec.push_back(cv::Mat(mat_height, mat_width, CV_8UC1, bin_data + 2 * mat_width * mat_height));
    image_bgra_vec.push_back(cv::Mat(mat_height, mat_width, CV_8UC1, bin_data + 1 * mat_width * mat_height));
    image_bgra_vec.push_back(cv::Mat(mat_height, mat_width, CV_8UC1, bin_data));
    cv::merge(image_bgra_vec, image_bgra);
    delete[] bin_data;
}

void Utils::dump_binary_file(const char *file_name, char *data, const size_t size)
{
    // eg:Utils::dump_binary_file(out_name.c_str(),reinterpret_cast<char *>(p_outputs_[i]),each_output_size_by_byte_[i+1]-each_output_size_by_byte_[i]);
    std::ofstream outf;
    outf.open(file_name, std::ofstream::binary);
    outf.write(data, size);
    outf.close();
}

void Utils::dump_gray_image(const char *file_name, const FrameCHWSize &frame_size, unsigned char *data)
{
    cv::Mat gray_image = cv::Mat(frame_size.height, frame_size.width, CV_8UC1, data);
    cv::imwrite(file_name, gray_image);
}

void Utils::dump_color_image(const char *file_name, const FrameCHWSize &frame_size, unsigned char *data)
{
    cv::Mat image_r = cv::Mat(frame_size.height, frame_size.width, CV_8UC1, data);
    cv::Mat image_g = cv::Mat(frame_size.height, frame_size.width, CV_8UC1, data+frame_size.height*frame_size.width);
    cv::Mat image_b = cv::Mat(frame_size.height, frame_size.width, CV_8UC1, data+2*frame_size.height*frame_size.width);
    
    std::vector<cv::Mat> color_vec(3);
    color_vec.clear();
    color_vec.push_back(image_b);
    color_vec.push_back(image_g);
    color_vec.push_back(image_r);

    cv::Mat color_img;
    cv::merge(color_vec, color_img);
    cv::imwrite(file_name, color_img);
}

void Utils::padding_resize_one_side_set(FrameCHWSize input_shape, FrameCHWSize output_shape, std::unique_ptr<ai2d_builder> &builder, const cv::Scalar padding)
{
    float ratiow = (float)output_shape.width / input_shape.width;
    float ratioh = (float)output_shape.height / input_shape.height;
    float ratio = ratiow < ratioh ? ratiow : ratioh;
    int new_w = (int)(ratio * input_shape.width);
    int new_h = (int)(ratio * input_shape.height);
    int top = 0;
    int bottom = output_shape.height - new_h;
    int left = 0;
    int right = output_shape.width - new_w;
    // run ai2d
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, typecode_t::dt_uint8, typecode_t::dt_uint8};
    ai2d_crop_param_t crop_param{false, 0, 0, 0, 0};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{true, {{0, 0}, {0, 0}, {top, bottom}, {left, right}}, ai2d_pad_mode::constant, {padding[0], padding[1], padding[2]}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};

    dims_t in_shape = {1,input_shape.channel,input_shape.height,input_shape.width};
    dims_t out_shape = {1,output_shape.channel,output_shape.height,output_shape.width};
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
}

void Utils::padding_resize_two_side_set(FrameCHWSize input_shape, FrameCHWSize output_shape, std::unique_ptr<ai2d_builder> &builder, const cv::Scalar padding)
{
    float ratiow = (float)output_shape.width / input_shape.width;
    float ratioh = (float)output_shape.height / input_shape.height;
    float ratio = ratiow < ratioh ? ratiow : ratioh;
    int new_w = (int)(ratio * input_shape.width);
    int new_h = (int)(ratio * input_shape.height);
    int top = (output_shape.height - new_h)/2;
    int bottom = (output_shape.height - new_h)/2;
    int left = (output_shape.width - new_w)/2;
    int right = (output_shape.width - new_w)/2;
    // run ai2d
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, typecode_t::dt_uint8, typecode_t::dt_uint8};
    ai2d_crop_param_t crop_param{false, 0, 0, 0, 0};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{true, {{0, 0}, {0, 0}, {top, bottom}, {left, right}}, ai2d_pad_mode::constant, {padding[0], padding[1], padding[2]}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};

    dims_t in_shape = {1,input_shape.channel,input_shape.height,input_shape.width};
    dims_t out_shape = {1,output_shape.channel,output_shape.height,output_shape.width};
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
}

void Utils::crop_resize_padding_one_side_set(FrameCHWSize input_shape, FrameCHWSize output_shape,int x,int y,int w,int h, std::unique_ptr<ai2d_builder> &builder, const cv::Scalar padding){
    float ratiow = (float)output_shape.width / w;
    float ratioh = (float)output_shape.height / h;
    float ratio = ratiow < ratioh ? ratiow : ratioh;
    int new_w = (int)(ratio * w);
    int new_h = (int)(ratio * h);
    int top = 0;
    int bottom = output_shape.height - new_h;
    int left = 0;
    int right = output_shape.width - new_w;
    // run ai2d
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, typecode_t::dt_uint8, typecode_t::dt_uint8};
    ai2d_crop_param_t crop_param{true, x, y, w, h};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{true, {{0, 0}, {0, 0}, {top, bottom}, {left, right}}, ai2d_pad_mode::constant, {padding[0], padding[1], padding[2]}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};

    dims_t in_shape = {1,input_shape.channel,input_shape.height,input_shape.width};
    dims_t out_shape = {1,output_shape.channel,output_shape.height,output_shape.width};
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
}

void Utils::center_crop_resize_set(FrameCHWSize input_shape,FrameCHWSize output_shape,std::unique_ptr<ai2d_builder> &builder){
    int min_m = input_shape.width<input_shape.height?input_shape.width:input_shape.height;
    int top = int((input_shape.height-min_m)*0.5);
    int left = int((input_shape.width-min_m)*0.5);
    // run ai2d
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, typecode_t::dt_uint8, typecode_t::dt_uint8};
    ai2d_crop_param_t crop_param{true, left, top, min_m, min_m};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, ai2d_pad_mode::constant, {0, 0, 0}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};
    dims_t in_shape = {1,input_shape.channel,input_shape.height,input_shape.width};
    dims_t out_shape = {1,output_shape.channel,output_shape.height,output_shape.width};
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
}

void Utils::crop_resize_set(FrameCHWSize input_shape,FrameCHWSize output_shape,int x,int y,int w,int h,std::unique_ptr<ai2d_builder> &builder){
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, typecode_t::dt_uint8, typecode_t::dt_uint8};
    ai2d_crop_param_t crop_param{true, x, y, w, h};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, ai2d_pad_mode::constant, {0, 0, 0}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};
    dims_t in_shape = {1,input_shape.channel,input_shape.height,input_shape.width};
    dims_t out_shape = {1,output_shape.channel,output_shape.height,output_shape.width};
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
}

void Utils::crop_resize_out2RGBP_out2HWC_set(FrameCHWSize input_shape,FrameCHWSize output_shape,int x,int y,int w,int h,std::unique_ptr<ai2d_builder> &builder){
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::RGB_packed, typecode_t::dt_uint8, typecode_t::dt_uint8};
    ai2d_crop_param_t crop_param{true, x, y, w, h};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, ai2d_pad_mode::constant, {0, 0, 0}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};
    dims_t in_shape = {1,input_shape.channel,input_shape.height,input_shape.width};
    dims_t out_shape = {1,output_shape.height,output_shape.width,3};
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
}

 void Utils::crop_set(FrameCHWSize input_shape,int x,int y,int w,int h,std::unique_ptr<ai2d_builder> &builder){
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, typecode_t::dt_uint8, typecode_t::dt_uint8};
    ai2d_crop_param_t crop_param{true, x, y, w, h};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, ai2d_pad_mode::constant, {0, 0, 0}};
    ai2d_resize_param_t resize_param{false, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};
    dims_t in_shape = {1,input_shape.channel,input_shape.height,input_shape.width};
    dims_t out_shape = {1,3,h,w};
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
 }

void Utils::resize_set(FrameCHWSize input_shape, FrameCHWSize output_shape, std::unique_ptr<ai2d_builder> &builder){
    // run ai2d
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, typecode_t::dt_uint8, typecode_t::dt_uint8};
    ai2d_crop_param_t crop_param{false, 0, 0, 0, 0};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, ai2d_pad_mode::constant, {0, 0, 0}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};
    dims_t in_shape = {1,input_shape.channel,input_shape.height,input_shape.width};
    dims_t out_shape = {1,output_shape.channel,output_shape.height,output_shape.width};
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
}

void Utils::ratio_resize_set(FrameCHWSize input_shape, int max_size, std::unique_ptr<ai2d_builder> &builder){
    float ratio_w=(float)max_size/input_shape.width;
    float ratio_h=(float)max_size/input_shape.height;
    float ratio=ratio_w<ratio_h?ratio_w:ratio_h;
    int new_w=(int)(ratio*input_shape.width);
    int new_h=(int)(ratio*input_shape.height);
    // run ai2d
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, typecode_t::dt_uint8, typecode_t::dt_uint8};
    ai2d_crop_param_t crop_param{false, 0, 0, 0, 0};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, ai2d_pad_mode::constant, {0, 0, 0}};
    ai2d_resize_param_t resize_param{true, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{false, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {0.5, 0.1, 0.0, 0.1, 0.5, 0.0}};
    dims_t in_shape = {1,input_shape.channel,input_shape.height,input_shape.width};
    dims_t out_shape = {1,3,new_h,new_w};
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
}

void Utils::affine_set(FrameCHWSize input_shape, FrameCHWSize output_shape, std::unique_ptr<ai2d_builder> &builder,float *affine_matrix){
    ai2d_datatype_t ai2d_dtype{ai2d_format::NCHW_FMT, ai2d_format::NCHW_FMT, typecode_t::dt_uint8, typecode_t::dt_uint8};
    ai2d_crop_param_t crop_param{false, 0, 0, 0, 0};
    ai2d_shift_param_t shift_param{false, 0};
    ai2d_pad_param_t pad_param{false, {{0, 0}, {0, 0}, {0, 0}, {10, 0}}, ai2d_pad_mode::constant, {255, 10, 5}};
    ai2d_resize_param_t resize_param{false, ai2d_interp_method::tf_bilinear, ai2d_interp_mode::half_pixel};
    ai2d_affine_param_t affine_param{true, ai2d_interp_method::cv2_bilinear, 0, 0, 127, 1, {affine_matrix[0], affine_matrix[1], affine_matrix[2], affine_matrix[3], affine_matrix[4], affine_matrix[5]}};

    dims_t in_shape = {1,input_shape.channel,input_shape.height,input_shape.width};
    dims_t out_shape = {1,output_shape.channel,output_shape.height,output_shape.width};
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
}

cv::Mat Utils::padding_resize(cv::Mat &img, FrameCHWSize &frame_size, cv::Scalar &padding_value)
{
    // width:dst_width
    int ori_w = img.cols;
    int ori_h = img.rows;
    float ratiow = (float)frame_size.width / ori_w;
    float ratioh = (float)frame_size.height / ori_h;
    float ratio = ratiow < ratioh ? ratiow : ratioh;
    int new_w = (int)(ratio * ori_w);
    int new_h = (int)(ratio * ori_h);
    float dw = (float)(frame_size.width - new_w) / 2;
    float dh = (float)(frame_size.height - new_h) / 2;
    int top = (int)(roundf(0 - 0.1));
    int bottom = (int)(roundf(dh * 2 + 0.1));
    int left = (int)(roundf(0 - 0.1));
    int right = (int)(roundf(dw * 2 - 0.1));
    cv::Mat cropped_img;
    {
        if ((new_w != frame_size.width) || (new_h != frame_size.height))
        {
            cv::resize(img, cropped_img, cv::Size(new_w, new_h), cv::INTER_AREA);
        }
    }
    {
        // 104, 117, 123,BGR
        cv::copyMakeBorder(cropped_img, cropped_img, top, bottom, left, right, cv::BORDER_CONSTANT, padding_value);
    }
    return cropped_img;
}

cv::Mat Utils::resize(cv::Mat &img, FrameCHWSize &frame_size)
{
    cv::Mat cropped_img;
    cv::resize(img, cropped_img, cv::Size(frame_size.width, frame_size.height), cv::INTER_LINEAR);
    return cropped_img;
}

cv::Mat Utils::bgr_to_rgb(cv::Mat &ori_img)
{
    cv::Mat rgb_img;
    cv::cvtColor(ori_img, rgb_img, cv::COLOR_BGR2RGB);
    return rgb_img;
}
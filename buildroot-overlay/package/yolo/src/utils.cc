#include <iostream>
#include "utils.h"

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

std::vector<std::string> readLabelsFromTxt(std::string labels_txt_path) {
    std::vector<std::string> labels;
    std::ifstream file(labels_txt_path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << labels_txt_path << std::endl;
        return labels;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            // 去掉末尾的 '\r'（Windows CRLF 兼容）
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            labels.push_back(line);
        }
    }
    return labels;
}

void transpose_block_rvv(const float* input0, float* output_det,
                     int box_num, int box_feature_len)
{
    const int BLOCK = 32;
    for(int i = 0; i < box_feature_len; i += BLOCK)
    {
        int i_max = std::min(i + BLOCK, box_feature_len);
        for(int j = 0; j < box_num; j += BLOCK)
        {
            int j_max = std::min(j + BLOCK, box_num);
            for(int ii = i; ii < i_max; ii++)
            {
                const float* src = input0 + ii * box_num + j;
                float* dst = output_det + j * box_feature_len + ii;
                int remain = j_max - j;
                size_t stride = box_feature_len * sizeof(float); // stride in bytes

                int jj = 0;
                while (remain > 0)
                {
                    size_t vl = vsetvl_e32m8(remain);
                    // 连续加载源数据
                    vfloat32m8_t vsrc = vle32_v_f32m8(src + jj, vl);
                    // 按固定步长写入目标（转置）
                    vsse32_v_f32m8(dst + jj * box_feature_len, stride, vsrc, vl);

                    jj += vl;
                    remain -= vl;
                }
            }
        }
    }
}

void transpose_block_fast(const float* input0, float* output_det,
                          int box_num, int box_feature_len)
{
    const int TILE = 8;
    for (int i = 0; i < box_feature_len; i += TILE)
    {
        int i_max = std::min(i + TILE, box_feature_len);
        for (int j = 0; j < box_num; j += TILE)
        {
            int j_max = std::min(j + TILE, box_num);
            int h = i_max - i;
            int w = j_max - j;

            // small stack buffer to hold the tile in row-major: buf[row][col]
            float buf[TILE][TILE] = {0};

            // temporary vector register reused for each row load
            for (int r = 0; r < h; ++r)
            {
                const float* src = input0 + (i + r) * box_num + j;
                // set vector length for e32 (float32)
                size_t vl = vsetvl_e32m1(w);
                vfloat32m1_t vtmp = vle32_v_f32m1(src, vl);
                // store vector into a small temp array (buf[r])
                // use vse32 to write out 'vl' floats into buf[r]
                vse32_v_f32m1(buf[r], vtmp, vl);
            }

            // write transposed tile from buf to output_det
            // output position: output_det[(j + c) * box_feature_len + (i + r)]
            for (int r = 0; r < h; ++r)
            {
                for (int c = 0; c < w; ++c)
                {
                    output_det[(j + c) * box_feature_len + (i + r)] = buf[r][c];
                }
            }
        }
    }
}


auto cache = cv::Mat::zeros(1, 1, CV_32FC1);
void Utils::dump_binary_file(const char *file_name, char *data, const size_t size)
{
    // eg:Utils::dump_binary_file(out_name.c_str(),reinterpret_cast<char *>(p_outputs_[i]),each_output_size_by_byte_[i+1]-each_output_size_by_byte_[i]);
    std::ofstream outf;
    outf.open(file_name, std::ofstream::binary);
    outf.write(data, size);
    outf.close();
}

void Utils::dump_gray_image(const char *file_name, const FrameSize &frame_size, unsigned char *data)
{
    cv::Mat gray_image = cv::Mat(frame_size.height, frame_size.width, CV_8UC1, data);
    cv::imwrite(file_name, gray_image);
}

void Utils::dump_color_image(const char *file_name, const FrameSize &frame_size, unsigned char *data)
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

void Utils::padding_resize_one_side_set(FrameSize input_shape, FrameSize output_shape, std::unique_ptr<ai2d_builder> &builder, const cv::Scalar padding)
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

    dims_t in_shape = {1,3,input_shape.height,input_shape.width};
    dims_t out_shape = {1,3,output_shape.height,output_shape.width};
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
}

void Utils::center_crop_resize_set(FrameSize input_shape,FrameSize output_shape,std::unique_ptr<ai2d_builder> &builder){
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
    dims_t in_shape = {1,3,input_shape.height,input_shape.width};
    dims_t out_shape = {1,3,output_shape.height,output_shape.width};
    builder.reset(new ai2d_builder(in_shape, out_shape, ai2d_dtype, crop_param, shift_param, pad_param, resize_param, affine_param));
    builder->build_schedule();
}
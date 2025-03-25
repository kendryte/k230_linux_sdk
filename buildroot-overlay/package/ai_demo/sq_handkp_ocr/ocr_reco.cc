/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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

#include "ocr_reco.h"

OCRReco::OCRReco(const char *kmodel_file, int dict_size, const int debug_mode)
:dict_size(dict_size), AIBase(kmodel_file,"OCRReco", debug_mode)
{
    model_name_ = "OCRReco";
    flag = 0;

    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];

    output = new float[input_width * dict_size / 4];

    ai2d_out_tensor_ = this -> get_input_tensor(0);
}

OCRReco::OCRReco(const char *kmodel_file, int dict_size, FrameCHWSize isp_shape, const int debug_mode)
:dict_size(dict_size), AIBase(kmodel_file,"OCRReco", debug_mode)
{
    model_name_ = "OCRReco";
    flag = 0;

    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];

    output = new float[input_width * dict_size / 4];
    isp_shape_ = isp_shape;

    ai2d_out_tensor_ = this -> get_input_tensor(0);
    Utils::resize(isp_shape_, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_, false);
}

OCRReco::~OCRReco()
{
    // delete[] output;
}

void OCRReco::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;

    Utils::bgr2rgb_and_hwc2chw(ori_img, chw_vec);
    Utils::padding_resize_one_side({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(0, 0, 0));
}

void OCRReco::pre_process(runtime_tensor& img_data)
{
    ScopedTiming st(model_name_ + " pre_process video", debug_mode_);
    ai2d_builder_->invoke(img_data, ai2d_out_tensor_).expect("error occurred in ai2d running");
}

void OCRReco::inference()
{
    this->run();
    this->get_output();
}

void OCRReco::post_process(vector<string> &results)
{
    output = p_outputs_[0];

	int size = input_width / 4;

    // std::cout << size << std::endl;

	ifstream dict(DICT);
	vector<string> txt;
	txt.push_back("blank");
	while (!dict.eof())
	{
		string line;
		while (getline(dict, line))
		{
			txt.push_back(line);
		}
	}
	txt.push_back("");


	vector<int> result;
	for (int i = 0; i < size; i++)
	{
		int max_index = std::max_element(output + i * dict_size, output + i * dict_size + dict_size) - (output + i * dict_size);
        // int max_index = std::max_element(pred,pred+output_shapes_[0][3]) - pred;
		result.push_back(max_index);
	}

	for (int i = 0; i < size; i++)
	{
		if (result[i] == 0)
			continue;
		if (i > 0 && result[i - 1] == result[i])
			continue;
		results.push_back(txt[result[i]]);
	}
}

std::vector<size_t> sort_indices(const std::vector<cv::Point2f>& vec) 
{
	std::vector<std::pair<cv::Point2f, size_t>> indexedVec;
	indexedVec.reserve(vec.size());

	// 创建带有索引的副本
	for (size_t i = 0; i < vec.size(); ++i) {
		indexedVec.emplace_back(vec[i], i);
	}

	// 按值对副本进行排序
	std::sort(indexedVec.begin(), indexedVec.end(),
		[](const auto& a, const auto& b) {
		return a.first.x < b.first.x;
	});

	// 提取排序后的索引
	std::vector<size_t> sortedIndices;
	sortedIndices.reserve(vec.size());
	for (const auto& element : indexedVec) {
		sortedIndices.push_back(element.second);
	}

	return sortedIndices;
}

void find_rectangle_vertices(const std::vector<cv::Point2f>& points, cv::Point2f& topLeft, cv::Point2f& topRight, cv::Point2f& bottomRight, cv::Point2f& bottomLeft) 
{
    //先按照x排序,比较左右，再按照y比较上下
	auto sorted_x_id = sort_indices(points);

	if (points[sorted_x_id[0]].y < points[sorted_x_id[1]].y)
	{
		topLeft = points[sorted_x_id[0]];
		bottomLeft = points[sorted_x_id[1]];
	}
	else
	{
		topLeft = points[sorted_x_id[1]];
		bottomLeft = points[sorted_x_id[0]];
	}

	if (points[sorted_x_id[2]].y < points[sorted_x_id[3]].y)
	{
        bottomRight = points[sorted_x_id[3]];
		topRight = points[sorted_x_id[2]];
	}
	else
	{ 
        bottomRight = points[sorted_x_id[2]];
		topRight = points[sorted_x_id[3]];
	}
}

void OCRReco::warppersp(cv::Mat src, cv::Mat& dst, Boxb b)
{
    cv::Mat rotation;
    vector<cv::Point> con;
    for(auto i : b.ver_src)
        con.push_back(i);

    cv::RotatedRect minrect = minAreaRect(con);
    std::vector<cv::Point2f> vtx(4),vtd(4),vt(4);
    minrect.points(vtx.data());

    find_rectangle_vertices(vtx, vtd[0], vtd[1], vtd[2], vtd[3]);
    
    float tmp_w = cv::norm(vtd[1]-vtd[0]);
    float tmp_h = cv::norm(vtd[2]-vtd[1]);
    float w = std::max(tmp_w,tmp_h);
    float h = std::min(tmp_w,tmp_h);

    vt[0].x = 0;
    vt[0].y = 0;
    vt[1].x = w;//w
    vt[1].y = 0;
    vt[2].x = w;
    vt[2].y = h;
    vt[3].x = 0;
    vt[3].y = h;//h
    rotation = cv::getPerspectiveTransform(vtd, vt);

    warpPerspective(src, dst, rotation, cv::Size(w, h));
}
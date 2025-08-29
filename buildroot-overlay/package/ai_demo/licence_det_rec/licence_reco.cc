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

#include "setting.h"
#include "licence_reco.h"
#include "text_paint.h"
#include "setting.h"

LicenceReco::LicenceReco(const char *kmodel_file, int dict_size, const int debug_mode)
:dict_size(dict_size), AIBase(kmodel_file,"LicenceReco", debug_mode)
{
    model_name_ = "LicenceReco";
    flag = 0;

    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];

    output = new float[input_width * dict_size / 4];

    ai2d_out_tensor_ = this -> get_input_tensor(0);
}

LicenceReco::~LicenceReco()
{
    // delete[] output;
}

void LicenceReco::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::hwc_to_chw(ori_img, chw_vec);
    Utils::resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, ai2d_out_tensor_);
}

void LicenceReco::inference()
{
    // this->set_input_tensor(0, ai2d_out_tensor_);
    this->run();
    this->get_output();
}

void LicenceReco::post_process(string &results)
{
    results="";
    output = p_outputs_[0];

	int size = input_width / 4;

	vector<int> result;
	for (int i = 0; i < size; i++)
	{
		float maxs = -10.f;
		int index = -1;
		for (int j = 0; j < dict_size; j++)
		{
			if (maxs < output[i * dict_size + j])
			{
				index = j;
				maxs = output[i * dict_size + j];
			}
		}
		result.push_back(index);
	}

    for (int i = 0; i < size; i++){
		if (result[i] >= 0 && result[i] != 0 && !(i > 0 && result[i-1] == result[i]))
		{
			results+=dict[result[i]-1];
		}
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

void LicenceReco::warppersp(cv::Mat src, cv::Mat& dst, BoxPoint b, std::vector<cv::Point2f>& vtd)
{
    Mat rotation;
    vector<Point> con;
    for(auto i : b.vertices)
        con.push_back(i);

    RotatedRect minrect = minAreaRect(con);
    std::vector<cv::Point2f> vtx(4),vt(4);
    minrect.points(vtx.data());

    find_rectangle_vertices(vtx, vtd[0], vtd[1], vtd[2], vtd[3]);
    
    //w,h tmp_w=dist(p1,p0),tmp_h=dist(p1,p2)
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

    warpPerspective(src, dst, rotation, Size(w, h));
}


void LicenceReco::draw_text(float x_offset,float y_offset,cv::Mat& src_img,string &result,TextRenderer& writepen)
{
    int src_w_osd = src_img.cols;
    int src_h_osd = src_img.rows;

	if(src_img.channels() == 3){
		int x=int(x_offset);
		int y=int(y_offset);
		writepen.putText(src_img, result, cv::Point(x, y - 10), cv::Scalar(255, 0, 0));
	}
	else if(src_img.channels()==4){
		int x=int(x_offset*src_w_osd/SENSOR_WIDTH);
		int y=int(y_offset*src_h_osd/SENSOR_HEIGHT);
		writepen.putText(src_img, result, cv::Point(x, y - 10), cv::Scalar(255, 0, 0,255));
	}
    
}
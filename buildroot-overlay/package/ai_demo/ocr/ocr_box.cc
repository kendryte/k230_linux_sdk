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

#include "ocr_box.h"

OCRBox::OCRBox(const char *kmodel_file, float threshold, float box_thresh, const int debug_mode)
:threshold(threshold), box_thresh(box_thresh), AIBase(kmodel_file,"OCRBox", debug_mode)
{
    model_name_ = "OCRBox";
    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];
    output = new float[input_shapes_[0][2]*input_shapes_[0][3]];
    ai2d_out_tensor_ = this -> get_input_tensor(0);
}

OCRBox::OCRBox(const char *kmodel_file, float threshold, float box_thresh, FrameCHWSize isp_shape,  const int debug_mode)
:threshold(threshold), box_thresh(box_thresh), AIBase(kmodel_file,"OCRBox", debug_mode)
{
    model_name_ = "OCRBox";
    input_width = input_shapes_[0][3];
    input_height = input_shapes_[0][2];
    output = new float[input_shapes_[0][2]*input_shapes_[0][3]];
    isp_shape_ = isp_shape;
    ai2d_out_tensor_ = this -> get_input_tensor(0);
    Utils::padding_resize(isp_shape, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_builder_, ai2d_in_tensor_, ai2d_out_tensor_, cv::Scalar(0, 0, 0),false);
}


OCRBox::~OCRBox()
{
    delete[] output;
}

// ai2d for image
void OCRBox::pre_process(cv::Mat ori_img)
{
    ScopedTiming st(model_name_ + " pre_process image", debug_mode_);
    std::vector<uint8_t> chw_vec;
    Utils::bgr2rgb_and_hwc2chw(ori_img, chw_vec);
    Utils::padding_resize({ori_img.channels(), ori_img.rows, ori_img.cols}, chw_vec, {input_shapes_[0][3], input_shapes_[0][2]}, ai2d_out_tensor_, cv::Scalar(0, 0, 0));
}

// ai2d for video
void OCRBox::pre_process(runtime_tensor& img_data)
{
    ScopedTiming st(model_name_ + " pre_process video", debug_mode_);
    ai2d_builder_->invoke(img_data,ai2d_out_tensor_).expect("error occurred in ai2d running");
}

void OCRBox::inference()
{
    this->run();
    this->get_output();
}

void OCRBox::post_process(FrameSize frame_size, vector<ocr_det_res> &results)
{   
    for(int i = 0; i < input_height * input_width; i++)
      output[i] = p_outputs_[0][2*i];
    int h;
    int w;
    int row = frame_size.height;
    int col = frame_size.width;
    float ratio_w = 1.0 * input_width / col;
    float ratio_h = 1.0 * input_height / row;
    float ratio = min(ratio_h, ratio_w);
    int unpad_w = (int)round(col * ratio);
    int unpad_h = (int)round(row * ratio);
    float dw = (1.0 * input_width - unpad_w);
    float dh = (1.0 * input_height - unpad_h);
    h = int(round((dh - 0.1) / 2));
    w = int(round((dw - 0.1) / 2 ));
    Mat src(input_height, input_width, CV_32FC1, output);
    Mat mask(src > threshold);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    cv::findContours(mask, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    int num = contours.size();
    for(int i = 0; i < num; i++)
    {
        if(contours[i].size() < 4)
            continue;
        ocr_det_res b;
        getBox(b, contours[i]);
        vector<Point> con;
        unclip(contours[i], con);
        getBox(b, con);
        float score = boxScore(src, contours[i], b, w, h, frame_size);
        if (score < box_thresh)
            continue;
        b.score = score;
        float ratiow = 1.0 * input_width / frame_size.width;
        float ratioh = 1.0 * input_height / frame_size.height;
        for(int i = 0; i < 4; i++)
        {
            b.vertices[i].x = max(min((int)b.vertices[i].x, input_width), 0);
            b.vertices[i].y = max(min((int)b.vertices[i].y, input_height), 0);
            b.vertices[i].x = (b.vertices[i].x - w) / (input_width - 2 * w) * frame_size.width;
            b.vertices[i].y = (b.vertices[i].y - h) / (input_height - 2 * h) * frame_size.height;
            b.ver_src[i].x = b.vertices[i].x;
            b.ver_src[i].y = b.vertices[i].y;
            b.vertices[i].x = max((float)0, min(b.vertices[i].x, (float)frame_size.width));
            b.vertices[i].y = max((float)0, min(b.vertices[i].y, (float)frame_size.height));
        }
        results.push_back(b);
    }
}

void OCRBox::getBox(ocr_det_res& b, vector<Point> contours)
{
    RotatedRect minrect = minAreaRect(contours);
    Point2f vtx[4];
    minrect.points(vtx);
    for(int i = 0; i < 4; i++)
    {
        b.vertices[i].x = vtx[i].x;
        b.vertices[i].y = vtx[i].y;
    }
}

double OCRBox::distance(Point p0, Point p1)
{
    return sqrt((p0.x - p1.x) * (p0.x - p1.x) + (p1.y - p0.y) * (p1.y - p0.y));
}

void OCRBox::unclip(vector<Point> contours, vector<Point>& con)
{
    Path subj;
    Paths solution;
    double dis = 0.0;
    for(int i = 0; i < contours.size(); i++)
        subj << IntPoint(contours[i].x, contours[i].y);
    for(int i = 0; i < contours.size() - 1; i++)
        dis += distance(contours[i], contours[i+1]);
    double dis1 = (-1 * Area(subj)) * 1.5 / dis;
    ClipperOffset co;
    co.AddPath(subj, jtSquare, etClosedPolygon);
    co.Execute(solution, dis1);
    Path tmp = solution[0];
    for(int i = 0; i < tmp.size(); i++)
    {
        Point p(tmp[i].X, tmp[i].Y);
        con.push_back(p);
    }
    for(int i = 0; i < con.size(); i++)
        subj << IntPoint(con[i].x, con[i].y);
}

float OCRBox::boxScore(Mat src, vector<Point> contours, ocr_det_res& b, int w, int h, FrameSize frame_size)
{
    int xmin = input_width;
    int xmax = 0;
    int ymin = input_height;
    int ymax = 0;
    for(int i = 0; i < contours.size(); i++)
    {
        xmin = floor((contours[i].x < xmin ? contours[i].x : xmin));
        xmax = ceil((contours[i].x > xmax ? contours[i].x : xmax));
        ymin = floor((contours[i].y < ymin ? contours[i].y : ymin));
        ymax = ceil((contours[i].y > ymax ? contours[i].y : ymax));
    }
    for(int i = 0; i < contours.size(); i++)
    {
        contours[i].x = contours[i].x - xmin;
        contours[i].y = contours[i].y - ymin;
    }
    vector<vector<Point>> vec;
    vec.clear();
    vec.push_back(contours);
    float ratiow = 1.0 * input_width / frame_size.width;
    float ratioh = 1.0 * input_height / frame_size.height;
    b.meanx = ((1.0 * xmin + xmax) / 2 - w) / (input_width - 2 * w) * frame_size.width;
    b.meany = ((1.0 * ymin + ymax) / 2 - h) / (input_height - 2 * h) * frame_size.height;
    Mat img = Mat::zeros(ymax - ymin + 1, xmax - xmin + 1, CV_8UC1);
    cv::fillPoly(img, vec, Scalar(1));
    return (float)cv::mean(src(Rect(xmin, ymin, xmax-xmin+1, ymax-ymin+1)), img)[0];  
}


void expandRectangle(Point2f& topLeft, Point2f& topRight, Point2f& bottomRight, Point2f& bottomLeft, float scaleFactor_width,float scaleFactor_high, int maxWidth, int maxHeight) {
    // Calculate the center of the original rectangle

    Point2f newTopLeft(scaleFactor_width * topLeft.x - (scaleFactor_width-1)*topRight.x, scaleFactor_high * topLeft.y - (scaleFactor_high-1)*bottomLeft.y);
    Point2f newTopRight(scaleFactor_width * topRight.x - (scaleFactor_width-1)*topLeft.x, scaleFactor_high * topRight.y - (scaleFactor_high-1)*bottomRight.y);
    Point2f newBottomRight(scaleFactor_width * bottomRight.x - (scaleFactor_width-1)*bottomLeft.x, scaleFactor_high * bottomRight.y - (scaleFactor_high-1)*topRight.y);
    Point2f newBottomLeft(scaleFactor_width * bottomLeft.x - (scaleFactor_width-1)*bottomRight.x, scaleFactor_high * bottomLeft.y - (scaleFactor_high-1)*topLeft.y);

    // Check and adjust vertices to stay within specified boundaries
    if (newTopLeft.x < 0) newTopLeft.x = 0;
    if (newTopLeft.y < 0) newTopLeft.y = 0;
    if (newTopRight.x > maxWidth) newTopRight.x = maxWidth;
    if (newTopRight.y < 0) newTopRight.y = 0;
    if (newBottomRight.x > maxWidth) newBottomRight.x = maxWidth;
    if (newBottomRight.y > maxHeight) newBottomRight.y = maxHeight;
    if (newBottomLeft.x < 0) newBottomLeft.x = 0;
    if (newBottomLeft.y > maxHeight) newBottomLeft.y = maxHeight;

    // Update the input points with the adjusted vertices
    topLeft = newTopLeft;
    topRight = newTopRight;
    bottomRight = newBottomRight;
    bottomLeft = newBottomLeft;
    
}

std::vector<size_t> sort_indices(const std::vector<Point2f>& vec) 
{
	std::vector<std::pair<Point2f, size_t>> indexedVec;
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

void find_rectangle_vertices(const std::vector<Point2f>& points, Point2f& topLeft, Point2f& topRight, Point2f& bottomRight, Point2f& bottomLeft) 
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

void OCRBox::draw_ocr_image(cv::Mat& frame, vector<ocr_det_res>& results,vector<std::string> &rec_results,ChineseTextRenderer &writepen)
{
    double fontsize = (frame.cols * frame.rows * 1.0) / (1100 * 1200);
    
    for(int i = 0; i < results.size(); i++)
    {   
        std::vector<cv::Point> vec;
        vec.clear();
        for(int j = 0; j < 4; j++)
        {
            vec.push_back(results[i].vertices[j]);
        }
        cv::RotatedRect rect = minAreaRect(vec);
        std::vector<Point2f> ver(4),vtd(4);
        rect.points(ver.data());
        int maxWidth = frame.cols;
        int maxHeight = frame.rows;
        float scaleFactor_width = 1.025;
        float scaleFactor_higt = 1.15;
        
        find_rectangle_vertices(ver, vtd[0], vtd[1], vtd[2], vtd[3]);
        expandRectangle(vtd[0], vtd[1], vtd[2], vtd[3],scaleFactor_width,scaleFactor_higt,maxWidth,maxHeight);

        for(int c = 0; c < 4; c++)
            line(frame, ver[c], ver[(c + 1) % 4], Scalar(255, 0, 0), 3);
        std::string text = "score:" + std::to_string(round(results[i].score * 100) / 100.0).substr(0, 4);
        cv::putText(frame, text, cv::Point(results[i].meanx, results[i].meany), 
        cv::FONT_HERSHEY_SIMPLEX, fontsize, cv::Scalar(255, 255, 0), 2);

        writepen.putChineseText(frame, rec_results[i], cv::Point(vtd[3].x,vtd[3].y+20), cv::Scalar(0, 0, 255));

    }
}

void OCRBox::draw_ocr_video(cv::Mat& frame, vector<ocr_det_res>& results,vector<std::string> &rec_results, FrameSize osd_frame_size, FrameSize sensor_frame_size,ChineseTextRenderer &writepen)
{
    for(int i = 0; i < results.size(); i++)
    {   
        std::vector<cv::Point> vec;
        vec.clear();
        for(int j = 0; j < 4; j++)
        {
            cv::Point tmp = results[i].vertices[j];
            tmp.x = (float(tmp.x)/sensor_frame_size.width)*osd_frame_size.width;
            tmp.y = (float(tmp.y)/sensor_frame_size.height)*osd_frame_size.height;
            vec.push_back(tmp);
        }
        cv::RotatedRect rect = minAreaRect(vec);
        std::vector<Point2f> ver(4),vtd(4);
        rect.points(ver.data());
        int maxWidth = frame.cols;
        int maxHeight = frame.rows;
        float scaleFactor_width = 1.025;
        float scaleFactor_higt = 1.15;
        
        find_rectangle_vertices(ver, vtd[0], vtd[1], vtd[2], vtd[3]);
        expandRectangle(vtd[0], vtd[1], vtd[2], vtd[3],scaleFactor_width,scaleFactor_higt,maxWidth,maxHeight);

        for(int c = 0; c < 4; c++){
            line(frame, vtd[c], vtd[(c + 1) % 4], Scalar(0, 0, 255), 3);
        }
        writepen.putChineseText(frame, rec_results[i], cv::Point(vtd[3].x,vtd[3].y+20), cv::Scalar(0, 0, 255));
    }
}

void OCRBox::warppersp(cv::Mat src, cv::Mat& dst, ocr_det_res b, std::vector<Point2f>& vtd)
{
    Mat rotation;
    vector<Point> con;
    for(auto i : b.vertices)
        con.push_back(i);

    RotatedRect minrect = minAreaRect(con);
    std::vector<Point2f> vtx(4),vt(4);
    minrect.points(vtx.data());

    find_rectangle_vertices(vtx, vtd[0], vtd[1], vtd[2], vtd[3]);

    int maxWidth = src.cols;
    int maxHeight = src.rows;
    float scaleFactor_width = 1.025;
    float scaleFactor_higt = 1.15;
    expandRectangle(vtd[0], vtd[1], vtd[2], vtd[3],scaleFactor_width,scaleFactor_higt,maxWidth,maxHeight);

    
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
    rotation = getPerspectiveTransform(vtd, vt);

    warpPerspective(src, dst, rotation, Size(w, h));
}

// void OCRBox::warppersp(cv::Mat src, cv::Mat& dst, ocr_det_res b, std::vector<cv::Point2f>& vtd) {
//     // 预先分配空间
//     std::vector<cv::Point> con(4);
//     std::vector<cv::Point2f> vtx(4), vt(4);

//     // 将 vertices 转换为 Point 类型
//     for (size_t i = 0; i < 4; ++i) {
//         con[i] = b.vertices[i];
//     }

//     // 计算最小外接矩形
//     cv::RotatedRect minrect = cv::minAreaRect(con);
//     minrect.points(vtx.data());

//     // 找到矩形顶点
//     find_rectangle_vertices(vtx, vtd[0], vtd[1], vtd[2], vtd[3]);

//     // 扩展矩形
//     int maxWidth = src.cols;
//     int maxHeight = src.rows;
//     float scaleFactor_width = 1.025f;
//     float scaleFactor_height = 1.15f;
//     expandRectangle(vtd[0], vtd[1], vtd[2], vtd[3], scaleFactor_width, scaleFactor_height, maxWidth, maxHeight);

//     // 计算变换后的宽度和高度
//     float tmp_w = cv::norm(vtd[1] - vtd[0]);
//     float tmp_h = cv::norm(vtd[2] - vtd[1]);
//     float w = std::max(tmp_w, tmp_h);
//     float h = std::min(tmp_w, tmp_h);

//     // 定义目标矩形的四个顶点
//     vt[0] = {0, 0};
//     vt[1] = {w, 0};
//     vt[2] = {w, h};
//     vt[3] = {0, h};

//     // 获取透视变换矩阵
//     cv::Mat rotation = cv::getPerspectiveTransform(vtd, vt);

//     // 进行透视变换
//     cv::warpPerspective(src, dst, rotation, cv::Size(w, h));
// }
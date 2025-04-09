#include <iostream>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;

template<typename T>
T SumVector(vector<T>& vec)
{
    T res = 0;
    for (size_t i=0; i<vec.size(); i++)
    {
        res += vec[i];
    }
    return res;
}

vector<double> vevtor_time_0;
vector<double> vevtor_time_1;
vector<double> vevtor_time_2;



void resize_img_det(const cv::Mat &img, cv::Mat &resize_img,
                    const int resize_h,  const int resize_w) {
    int w = img.cols;
    int h = img.rows;
    float ratio = 1.f;
    //int max_wh = w >= h ? w : h;
    if (w <= resize_w && h <= resize_h){
        cv::copyMakeBorder(img, resize_img, 0, int(resize_h - img.rows), 0,
                           int(resize_w - img.cols), cv::BORDER_CONSTANT,
                           {127, 127, 127});
    }
    else {
        if (h/resize_h > w/resize_w) {
            ratio = float(resize_h) / float(h);
        }
        else {
            ratio = float(resize_w) / float(w);
        }
        int re_h = int(float(h) * ratio);
        int re_w = int(float(w) * ratio);
        re_h = std::max(int(round(float(re_h) / 32) * 32), 32);
        re_w = std::max(int(round(float(re_w) / 32) * 32), 32);
        cv::resize(img, resize_img, cv::Size(re_w, re_h));
        //std::cout<<resize_img.size()<<re_h<<re_w<<std::endl;
    }
    if (resize_img.cols<resize_w){
        cv::copyMakeBorder(resize_img, resize_img, 0, 0, 0,
                           int(resize_w - resize_img.cols), cv::BORDER_CONSTANT,
                           {127, 127, 127});
    }
    if (resize_img.rows<resize_h){
        cv::copyMakeBorder(resize_img, resize_img, 0, int(resize_h - resize_img.rows), 0,
                           0, cv::BORDER_CONSTANT,
                           {127, 127, 127});
    }
}

void normalize(cv::Mat *im) {
    std::vector<float> mean = {0.485f, 0.456f, 0.406f};
    std::vector<float> scale = {1 / 0.229f, 1 / 0.224f, 1 / 0.225f};
    bool is_scale = true;
    double e = 1.0;
    if (is_scale) {
        e /= 255.0;
    }
    (*im).convertTo(*im, CV_32FC3, e);
    std::vector<cv::Mat> bgr_channels(3);
    cv::split(*im, bgr_channels);
    for (auto i = 0; i < bgr_channels.size(); i++) {
        bgr_channels[i].convertTo(bgr_channels[i], CV_32FC1, 1.0 * scale[i],
                                  (0.0 - mean[i]) * scale[i]);
    }
    cv::merge(bgr_channels, *im);

}

void permute(const cv::Mat *im, float *data) {
    int rh = im->rows;
    int rw = im->cols;
    int rc = im->channels();
    for (int i = 0; i < rc; ++i) {
        cv::extractChannel(*im, cv::Mat(rh, rw, CV_32FC1, data + i * rh * rw), i);
    }
}

void det_preprocess(Mat& stitch, vector<float>& input_data_det, int input_height, int input_width){
    Mat image_resize;
    auto start = std::chrono::steady_clock::now();
    resize_img_det(stitch, image_resize, input_height, input_width);
    auto stop = std::chrono::steady_clock::now();
    double duration = std::chrono::duration<double, std::milli>(stop - start).count();
    std::cout << "resize&pad duration: " << duration << " ms" << std::endl;
    vevtor_time_0.push_back(duration);
    auto start_2 = std::chrono::steady_clock::now();
    normalize(&image_resize);
    auto stop_2 = std::chrono::steady_clock::now();
    double duration_2 = std::chrono::duration<double, std::milli>(stop_2 - start_2).count();
    std::cout << "normalize duration: " << duration_2 << " ms" << std::endl;
    vevtor_time_1.push_back(duration_2);
    auto start_3 = std::chrono::steady_clock::now();
    permute(&image_resize, input_data_det.data());
    auto stop_3 = std::chrono::steady_clock::now();
    double duration_3 = std::chrono::duration<double, std::milli>(stop_3 - start_3).count();
    std::cout << "permute duration: " << duration_3 << " ms" << std::endl;
    vevtor_time_2.push_back(duration_3);
    return ;
}

int main(int argc, char *argv[]) {
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <image> <round>" << std::endl;
        exit(1);
    }
    /*
    // 预处理
    Mat image = imread("/Users/bron/CLionProjects/time_test_temp/debug_image_stitch.jpg", IMREAD_COLOR);
    int input_height = 128;
    int input_width = 640;
    vector<float> input_data_det(1*3*input_height*input_width, 0.f);
    for(int i=0; i<200; i++) {
        det_preprocess(image, input_data_det, input_height, input_width);
    }

    // avg time step_0(resize&pad): 0.0281723
    // avg time step_1(normalize): 0.232809
    // avg time step_2(permute): 0.133781
    cout << "avg time step_0(resize&pad): " << SumVector(vevtor_time_0) / vevtor_time_0.size() << endl;
    cout << "avg time step_1(normalize): " << SumVector(vevtor_time_1) / vevtor_time_1.size() << endl;
    cout << "avg time step_2(permute): " << SumVector(vevtor_time_2) / vevtor_time_2.size() << endl;
    */

    /* // 测试积分图计算
    Mat image = imread("/Users/bron/CLionProjects/time_test_temp/test1.jpg", IMREAD_GRAYSCALE);
    cv::resize(image, image, cv::Size(1280, 1080));
    //cv::imwrite("/Users/bron/CLionProjects/time_test_temp/test1_1280x1080_grey.jpg", image);
    Mat imageIntegral;

    int round = 100;
    for(int i=0; i<round; i++){

        auto start = std::chrono::steady_clock::now();
        integral(image,imageIntegral,CV_32F); //计算积分图
        auto stop = std::chrono::steady_clock::now();
        double duration = std::chrono::duration<double, std::milli>(stop - start).count();
        vevtor_time_0.push_back(duration);
        std::cout << i << " integral: " << duration << " ms" << std::endl;
        //cv::imwrite("/Users/bron/CLionProjects/time_test_temp/test1_1280x1080_grey_integral.jpg", imageIntegral);

        auto start_2 = std::chrono::steady_clock::now();
        normalize(imageIntegral,imageIntegral,0,255,CV_MMX);  //归一化，方便显示
        auto stop_2 = std::chrono::steady_clock::now();
        double duration_2 = std::chrono::duration<double, std::milli>(stop_2 - start_2).count();
        vevtor_time_2.push_back(duration_2);
        std::cout << i << " norm: " << duration_2 << " ms" << std::endl;
        //cv::imwrite("/Users/bron/CLionProjects/time_test_temp/test1_1280x1080_grey_integral_norm.jpg", imageIntegral);
    }

    cout << "avg time step_0(inter): " << SumVector(vevtor_time_0) / vevtor_time_0.size() << endl;
    //cout << "avg time step_1(normalize): " << SumVector(vevtor_time_1) / vevtor_time_1.size() << endl;
    cout << "avg time step_2(norm): " << SumVector(vevtor_time_2) / vevtor_time_2.size() << endl;


    cv::imwrite("/Users/bron/CLionProjects/time_test_temp/test1_1280x1080_grey_integral_norm.jpg", imageIntegral);
    */

    /*
    //测试中值滤波
    Mat image = imread("/Users/bron/CLionProjects/time_test_temp/test1.jpg", IMREAD_GRAYSCALE);
    cv::resize(image, image, cv::Size(1280, 1080));
    //cv::imwrite("/Users/bron/CLionProjects/time_test_temp/test1_1280x1080_grey.jpg", image);

    //median filte

    int round = 100;
    for(int i=0; i<round; i++){
        Mat resutl;
        auto start_2 = std::chrono::steady_clock::now();
        cv::medianBlur(image, resutl, 3);
        auto stop_2 = std::chrono::steady_clock::now();
        double duration_2 = std::chrono::duration<double, std::milli>(stop_2 - start_2).count();
        std::cout << i << " mediablur: " << duration_2 << " ms" << std::endl;
        vevtor_time_2.push_back(duration_2);
    }
    cout << "avg time step_2(media_blur): " << SumVector(vevtor_time_2) / vevtor_time_2.size() << endl;
    //cv::imwrite("/Users/bron/CLionProjects/time_test_temp/test1_1280x1080_grey_mediablur.jpg", resutl);
    */

    /*
    //直方图统计
    Mat image = imread("/Users/bron/CLionProjects/time_test_temp/test1.jpg", IMREAD_GRAYSCALE);
    cv::resize(image, image, cv::Size(1280, 1080));

    int round = 100;
    for(int i=0; i<round; i++){
        int hist_size = 255;   // 每个bin的像素值范围是15
        float range[] = {0, 255} ;
        const float* hist_range = {range};
        bool uniform = true;
        bool accumulate = false;
        Mat hist;
        auto start_2 = std::chrono::steady_clock::now();
        calcHist(&image, 1, 0, Mat(), hist, 1, &hist_size, &hist_range, uniform, accumulate);
        auto stop_2 = std::chrono::steady_clock::now();
        double duration_2 = std::chrono::duration<double, std::milli>(stop_2 - start_2).count();
        std::cout << i << " hist: " << duration_2 << " ms" << std::endl;
        vevtor_time_2.push_back(duration_2);
    }


//    cout << "hist" << "10" << ": " << hist.at<float>(10) << endl;
//    cout << "hist" << "100" << ": " << hist.at<float>(100) << endl;
//    cout << "hist" << "200" << ": " << hist.at<float>(200) << endl;
//    for(int i = 0; i < hist_size; i++) {
//        int num = hist.at<float>(i);
//        cout << "hist" << i << ": " << hist.at<float>(i) << endl;
//    }
    cout << "avg time step_2(media_blur): " << SumVector(vevtor_time_2) / vevtor_time_2.size() << endl;
    */

    /*
    //仿射变换
    Mat image = imread("/Users/bron/CLionProjects/time_test_temp/test1.jpg", IMREAD_GRAYSCALE);
    cv::resize(image, image, cv::Size(1280, 1080));
    //创建仿射变换目标图像与原图像尺寸类型相同
    Mat warp_dstImage;
    warp_dstImage = Mat::zeros(image.rows, image.cols, image.type());

    Point center = Point(warp_dstImage.cols/2, warp_dstImage.rows/2);
    double angle = -15.0;
    double scale = 0.6;

    int round = 100;
    for(int i=0; i<round; i++){
        //计算旋转矩阵
        Mat rot_mat(2, 3, CV_32FC1);
        rot_mat = getRotationMatrix2D(center, angle, scale);
        Mat rotate_dstImage;
        auto start_2 = std::chrono::steady_clock::now();
        warpAffine(image, rotate_dstImage, rot_mat, image.size());
        auto stop_2 = std::chrono::steady_clock::now();
        double duration_2 = std::chrono::duration<double, std::milli>(stop_2 - start_2).count();
        std::cout << i << " warpaffine: " << duration_2 << " ms" << std::endl;
        vevtor_time_2.push_back(duration_2);
        cv::imwrite("/Users/bron/CLionProjects/time_test_temp/test1_1280x1080_grey_rotate.jpg", rotate_dstImage);
    }
    cout << "avg time step_2(warpaffine): " << SumVector(vevtor_time_2) / vevtor_time_2.size() << endl;
    */


    //梯度方向角
    Mat image = imread(argv[1], IMREAD_GRAYSCALE);
    cv::resize(image, image, cv::Size(1280, 1080));
    int round = atoi(argv[2]);
    for(int i=0; i<round; i++){
        Mat dx, dy, angle;
        Sobel(image, dx, CV_32F, 1, 0, 3);
        Sobel(image, dy, CV_32F, 0, 1, 3);
        // 计算梯度角度(弧度制)
        //phase(dx, dy, angle);
        //cout << "梯度方向角(弧度制): " << angle(Rect(0, 0, 10, 1)) << endl;
        // 计算梯度角度(角度制)
        auto start_2 = std::chrono::steady_clock::now();
        phase(dx, dy, angle, true);
        auto stop_2 = std::chrono::steady_clock::now();
        double duration_2 = std::chrono::duration<double, std::milli>(stop_2 - start_2).count();
        // std::cout << i << " angle: " << duration_2 << " ms" << std::endl;
        vevtor_time_2.push_back(duration_2);
        // cout << "梯度方向角(角度制): " << angle(Rect(0, 0, 10, 1)) << endl;
    }
    cout << "avg time step_2(angle): " << SumVector(vevtor_time_2) / vevtor_time_2.size() << endl;

    return 0;
}

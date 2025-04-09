#include <iostream>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include <opencv2/opencv.hpp>
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

    Mat image = imread(argv[1], IMREAD_GRAYSCALE);
    cv::resize(image, image, cv::Size(1280, 1080));
    //cv::imwrite("/Users/bron/CLionProjects/time_test_temp/test1_1280x1080_grey.jpg", image);
    Mat imageIntegral;

    int round = atoi(argv[2]);
    for(int i=0; i<round; i++){

        auto start = std::chrono::steady_clock::now();
        integral(image,imageIntegral,CV_32F); //计算积分图
        auto stop = std::chrono::steady_clock::now();
        double duration = std::chrono::duration<double, std::milli>(stop - start).count();
        vevtor_time_0.push_back(duration);
        // std::cout << i << " integral: " << duration << " ms" << std::endl;
        //cv::imwrite("/Users/bron/CLionProjects/time_test_temp/test1_1280x1080_grey_integral.jpg", imageIntegral);

        // auto start_2 = std::chrono::steady_clock::now();
        // //normalize(imageIntegral,imageIntegral,0,255,CV_MINMAX);  //归一化，方便显示
        // normalize(imageIntegral,imageIntegral,0,255,CV_MMX);  //归一化，方便显示
        // auto stop_2 = std::chrono::steady_clock::now();
        // double duration_2 = std::chrono::duration<double, std::milli>(stop_2 - start_2).count();
        // vevtor_time_2.push_back(duration_2);
        // std::cout << i << " norm: " << duration_2 << " ms" << std::endl;
        //cv::imwrite("/Users/bron/CLionProjects/time_test_temp/test1_1280x1080_grey_integral_norm.jpg", imageIntegral);
    }

    cout << "avg time step_0(inter): " << SumVector(vevtor_time_0) / vevtor_time_0.size() << endl;
    //cout << "avg time step_1(normalize): " << SumVector(vevtor_time_1) / vevtor_time_1.size() << endl;
    //cout << "avg time step_2(norm): " << SumVector(vevtor_time_2) / vevtor_time_2.size() << endl;

    // cv::imwrite("./test1_1280x1080_grey_integral_norm.jpg", imageIntegral);

    return 0;
}

#include <chrono>
#include <fstream>
#include <iostream>
#include <opencv2/videoio.hpp>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <stdlib.h>
#include <nncase/runtime/runtime_op_utility.h>
#include <nncase/runtime/interpreter.h>
#include <nncase/runtime/util.h>
#include "mobile_retinaface.h"
#include <opencv2/opencv.hpp>

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::detail;

#define img_channels  3
#define img_rows  1080
#define img_cols  1920

std::atomic<bool> ai_stop(false);

void ai_proc(const char *kmodel_file, const char *video_device)
{
    // input data
    size_t paddr = 0;
    void *vaddr = nullptr;

    auto cap = cv::VideoCapture(atoi(video_device));
    if (!cap.isOpened()) {
        std::cerr << "can't open video device" <<std::endl;
        return;
    }
    if (!cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('B', 'G', '3', 'P'))) {
        std::cerr << "can't set video fourcc to BG3P" <<std::endl;
        return;
    }
    if (!cap.set(cv::CAP_PROP_FRAME_WIDTH, img_cols)) {
        std::cerr << "can't set video frame width" <<std::endl;
        return;
    }
    if (!cap.set(cv::CAP_PROP_FRAME_HEIGHT, img_rows)) {
        std::cerr << "can't set video frame height" <<std::endl;
        return;
    }

    MobileRetinaface model(kmodel_file, img_channels, img_rows, img_cols);
    size_t idx = 0;
    cv::Mat frame;
    while (!ai_stop)
    {
        cap >> frame;
        // run kpu
        if (frame.empty()) {
            std::cout << "no frame" << std::endl;
            continue;
        }
        std::vector<unsigned char> data(frame.data, frame.data + img_rows * img_cols * img_channels);
        model.run(data);
        auto result = model.get_result();

        std::cout << "Number of faces detected: " << result.boxes.size() << std::endl;

        for (size_t i = 0; i < result.boxes.size(); i++) {
            auto box = result.boxes[i];
            auto landmark = result.landmarks[i];

            std::cout << "Face " << i + 1 << ":" << std::endl;
            std::cout << "  Bounding box: "
                      << "(" << box.x1 << ", " << box.y1 << ") to "
                      << "(" << box.x2 << ", " << box.y2 << ")" << std::endl;

            std::cout << "  Landmarks:" << std::endl;
            for (int j = 0; j < 5; j++) {
                std::cout << "    Point " << j + 1 << ": "
                          << "(" << landmark.points[2 * j] << ", " << landmark.points[2 * j + 1] << ")" << std::endl;
            }
            std::cout << std::endl;
        }

        if (result.boxes.size() > 0) {
            std::cout << "----------------------------------------" << std::endl;
            idx++;
            idx %= 10;
            std::cout << "Processing count: " << idx << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    std::cout << "case " << argv[0] << " built at " << __DATE__ << " " << __TIME__ << std::endl;
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <kmodel> <video device>" << std::endl;
        return -1;
    }

    ai_proc(argv[1], argv[2]);

    return 0;
}

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <stdlib.h>
#include <nncase/runtime/runtime_op_utility.h>
// #include <runtime_op_utility.h>
#include "mobile_retinaface.h"
#ifdef LINUX_RUNTIME
#include "mmz.h"
#else
#include "mpi_sys_api.h"
#endif

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::detail;

#define img_channels  3
#define img_rows  624
#define img_cols  1024

std::atomic<bool> ai_stop(false);

void ai_proc(const char *kmodel_file, const char *image_file)
{
    // input data
    size_t paddr = 0;
    void *vaddr = nullptr;
    // cv::Mat img = cv::imread(image_file);
    // int ret = kd_mpi_sys_mmz_alloc_cached(&paddr, &vaddr, "allocate", "anonymous", img.total() * img.elemSize());
    // if (ret)
    // {
    //     std::cerr << "physical_memory_block::allocate failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
    //     std::abort();
    // }

    // auto vec = hwc2chw(img);
    // memcpy(reinterpret_cast<char *>(vaddr), vec.data(), vec.size());

    auto in_data = read_binary_file<unsigned char>(image_file);
    MobileRetinaface model(kmodel_file, img_channels, img_rows, img_cols);
    size_t idx = 0;
    while (!ai_stop)
    {
        // run kpu
        model.run(in_data);
        auto result = model.get_result();

        std::cout << "Detection results for image: " << image_file << std::endl;
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

        std::cout << "----------------------------------------" << std::endl;

        if (result.boxes.size() > 0) {
            idx++;
            idx %= 10;
            std::cout << "Processing count: " << idx << std::endl;
        }

        ai_stop=1;
    }

    // // free memory
    // ret = kd_mpi_sys_mmz_free(paddr, vaddr);
    // if (ret)
    // {
    //     std::cerr << "free failed: ret = " << ret << ", errno = " << strerror(errno) << std::endl;
    //     std::abort();
    // }
}

int main(int argc, char *argv[])
{
    std::cout << "case " << argv[0] << " built at " << __DATE__ << " " << __TIME__ << std::endl;
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <kmodel> <rgb_image>" << std::endl;
        return -1;
    }

    std::cout << "Press 'q + enter' to exit!!!" << std::endl;
    std::thread t(ai_proc, argv[1], argv[2]);
    while (getchar() != 'q')
    {
        usleep(10000);
    }

    ai_stop = true;
    t.join();

    return 0;
}
#include <cassert>
#include <cstring>
#include <iostream>
#include <linux/videodev2.h>
#include <mutex>
#include <opencv2/videoio.hpp>
#include <cstdio>
#include <sys/select.h>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <stdlib.h>
#include <nncase/runtime/runtime_op_utility.h>
#include <nncase/runtime/interpreter.h>
#include <nncase/runtime/util.h>
#include "mobile_retinaface.h"
#include "mmz.h"
#include "util.h"
#include <opencv2/opencv.hpp>
#include <display.h>
#include <v4l2-drm.h>
#include <unistd.h>
#include <fcntl.h>
#include <thead.h>
#include <vector>

using namespace nncase;
using namespace nncase::runtime;
using namespace nncase::runtime::detail;
using namespace std;

#define img_channels  3

#define img_cols  1920
#define img_rows  1080


static mutex face_result_mutex;
static vector<face_coordinate> face_result;
static volatile unsigned kpu_frame_count = 0;

atomic<bool> ai_stop(false);

static void ai_proc_opencv(const char *kmodel_file, int video_device)
{
    // input data
    size_t paddr = 0;
    void *vaddr = nullptr;
    face_result_mutex.lock();
    face_result_mutex.unlock();

    auto cap = cv::VideoCapture(video_device);
    if (!cap.isOpened()) {
        cerr << "can't open video device" << endl;
        return;
    }
    if (!cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('B', 'G', '3', 'P'))) {
        cerr << "can't set video fourcc to BG3P" <<endl;
        return;
    }
    if (!cap.set(cv::CAP_PROP_FRAME_WIDTH, img_cols)) {
        cerr << "can't set video frame width" <<endl;
        return;
    }
    if (!cap.set(cv::CAP_PROP_FRAME_HEIGHT, img_rows)) {
        cerr << "can't set video frame height" <<endl;
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
            cout << "no frame" << endl;
            continue;
        }
        model.run(frame);
        auto result = model.get_result();
        face_result_mutex.lock();
        face_result = result.boxes;
        face_result_mutex.unlock();
        kpu_frame_count += 1;
    }
}

static struct display* display;

// zero copy, use less memory
static void ai_proc_dmabuf(const char *kmodel_file, int video_device) {
    struct v4l2_drm_context context;
    struct v4l2_drm_video_buffer buffer;
    #define BUFFER_NUM 3

    // wait display_proc running
    face_result_mutex.lock();
    face_result_mutex.unlock();

    v4l2_drm_default_context(&context);
    context.device = video_device;
    context.display = false;
    context.width = img_cols;
    context.height = img_rows;
    context.video_format = v4l2_fourcc('B', 'G', '3', 'P');
    context.buffer_num = BUFFER_NUM;
    if (v4l2_drm_setup(&context, 1, NULL)) {
        cerr << "v4l2_drm_setup error" << endl;
        return;
    }
    if (v4l2_drm_start(&context)) {
        cerr << "v4l2_drm_start error" << endl;
        return;
    }

    // create tensors
    std::vector<std::tuple<int, void*>> tensors;
    for (unsigned i = 0; i < BUFFER_NUM; i++) {
        tensors.push_back({context.buffers[i].fd, context.buffers[i].mmap});
    }

    MobileRetinaface model(kmodel_file, img_channels, img_rows, img_cols, tensors);

    while (!ai_stop) {
        int ret = v4l2_drm_dump(&context, 1000);
        if (ret) {
            perror("v4l2_drm_dump error");
            continue;
        }
        model.run_dmabuf(context.vbuffer.index);
        auto result = model.get_result();
        if (display) {
            face_result_mutex.lock();
            face_result = result.boxes;
            face_result_mutex.unlock();
        } else {
            char c;
            ssize_t n = read(STDIN_FILENO, &c, 1);
            if ((n > 0) && (c == 'q')) {
                ai_stop.store(true);
            }
            if (result.boxes.size()) {
                printf("get %lu face(s)\n", result.boxes.size());
                for (auto box: result.boxes) {
                    printf("(x1: %d, y1: %d, x2: %d, y2: %d)\n", box.x1, box.y1, box.x2, box.y2);
                }
            }
        }
        kpu_frame_count += 1;
        v4l2_drm_dump_release(&context);
    }
    v4l2_drm_stop(&context);
}

static struct timeval tv, tv2;
struct display_buffer* draw_buffer;

int frame_handler(struct v4l2_drm_context *context, bool displayed) {
    static bool first_frame = true;
    if (first_frame) {
        face_result_mutex.unlock();
        first_frame = false;
    }

    static unsigned response = 0, display_frame_count = 0;
    response += 1;
    if (displayed) {
        if (context[0].buffer_hold[context[0].wp] >= 0) {
            // draw result on context[i].display_buffers[context[i].buffer_hold[context[i].wp]]
            static struct display_buffer* last_drawed_buffer = nullptr;
            auto buffer = context[0].display_buffers[context[0].buffer_hold[context[0].wp]];
            if (buffer != last_drawed_buffer) {
                auto img = cv::Mat(draw_buffer->height, draw_buffer->width, CV_8UC4, draw_buffer->map);
                face_result_mutex.lock();
                // cv::rectangle(img, cv::Point(16, 80), cv::Point(160, 300), cv::Scalar(255, 255, 255, 255), 2);
                memset(draw_buffer->map,0, draw_buffer->size);

                for (auto& box: face_result) {

                    if(draw_buffer->width > draw_buffer->height)
                    {
                        cv::rectangle(
                        img,
                        cv::Point(box.x1 * draw_buffer->width / img_cols, box.y1 * draw_buffer->height / img_rows),
                        cv::Point(box.x2 * draw_buffer->width / img_cols, box.y2 * draw_buffer->height / img_rows),
                        cv::Scalar(0, 255, 0, 255), 2
                        );
                    }
                    else {
                        uint32_t line_x_start = ((uint32_t)box.y2) * draw_buffer->height / img_cols;
                        uint32_t line_y_start = ((uint32_t)box.x1) * draw_buffer->width / img_rows;
                        uint32_t line_x_end = ((uint32_t)box.y1) * draw_buffer->height / img_cols;
                        uint32_t line_y_end = ((uint32_t)box.x2) * draw_buffer->width / img_rows;

                        cv::rectangle(
                        img,
                        cv::Point( display->width - line_x_start, line_y_start),
                        cv::Point( display->width - line_x_end, line_y_end),
                        cv::Scalar(0, 255, 0, 255), 2
                        );

                    }
                }

                face_result_mutex.unlock();
                last_drawed_buffer = buffer;
                // flush cache
                thead_csi_dcache_clean_invalid_range(buffer->map, buffer->size);

                // static bool first = 1;
                // if (first) {
                //     first = 0;
                //     cv::imwrite("test.png", img);
                //     printf("write test.png\n");
                // }
                display_update_buffer(draw_buffer, 0, 0);
                
            }
        }
        display_frame_count += 1;
    }

    // FPS counter
    gettimeofday(&tv2, NULL);
    uint64_t duration = 1000000 * (tv2.tv_sec - tv.tv_sec) + tv2.tv_usec - tv.tv_usec;
    if (duration >= 1000000) {
        fprintf(stderr, " poll: %.2f, ", response * 1000000. / duration);
        response = 0;
        if (display) {
            fprintf(stderr, "display: %.2f, ", display_frame_count * 1000000. / duration);
            display_frame_count = 0;
        }
        fprintf(stderr, "camera: %.2f, ", context[0].frame_count * 1000000. / duration);
        context[0].frame_count = 0;
        fprintf(stderr, "KPU: %.2f", kpu_frame_count * 1000000. / duration);
        kpu_frame_count = 0;
        fprintf(stderr, "          \r");
        fflush(stderr);
        gettimeofday(&tv, NULL);
    }

    // key
    char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if ((n > 0) && (c != '\n')) {
        return c;
    }
    if ((n < 0) && (errno != EAGAIN)) {
        return -1;
    }
    return 0;
}

static void display_proc(int video_device) {
    struct v4l2_drm_context context;
    v4l2_drm_default_context(&context);
    context.device = video_device;


    if(display->width > display->height)
    {
        context.width = display->width;
        context.height = (display->width * img_rows / img_cols) & 0xfff8;
        context.video_format = V4L2_PIX_FMT_NV12;
        context.display_format = 0; // auto
        context.drm_rotation = rotation_0;
        // context.buffer_num = 5;

    }
    else {
        context.width = display->height;
        context.height = display->width;
        context.video_format = V4L2_PIX_FMT_NV12;
        context.display_format = 0; // auto
        context.drm_rotation = rotation_90;
    }
    
    if (v4l2_drm_setup(&context, 1, &display)) {
        cerr << "v4l2_drm_setup error" << endl;
        return;
    }

    struct display_plane* plane = display_get_plane(display, DRM_FORMAT_ARGB8888);
    draw_buffer = display_allocate_buffer(plane,  display->width, display->height);
    display_commit_buffer(draw_buffer, 0, 0);

    cout << "press 'q' to exit" << endl;
    cout << "press 'd' to save a picture" << endl;
    gettimeofday(&tv, NULL);
    v4l2_drm_run(&context, 1, frame_handler);
    if (display) {
        // free plane for argb
        display_free_plane(plane);
        display_exit(display);
    }
    ai_stop.store(true);
    return;
}


void __attribute__((destructor)) cleanup() {
    std::cout << "Cleaning up memory..." << std::endl;
    shrink_memory_pool();
    kd_mpi_mmz_deinit();
}

int main(int argc, char *argv[])
{   
    cout << "case " << argv[0] << " built at " << __DATE__ << " " << __TIME__ << endl;
    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " <kmodel" << endl;
        return -1;
    }

    display = display_init(0);
    if (!display) {
        cerr << "display_init error, disable display" << endl;
    }

    // set stdin non-block
    int flag = fcntl(STDIN_FILENO, F_GETFL);
    flag |= O_NONBLOCK;
    if (fcntl(STDIN_FILENO, F_SETFL, flag)) {
        cerr << "can't set stdin non-block" << endl;
        return -1;
    }

    face_result_mutex.lock();
    auto ai_thread = thread(ai_proc_dmabuf, argv[1], 2);
    // auto display_thread = thread(display_proc, 1);
    if (display) {
        display_proc(1);
    } else {
        face_result_mutex.unlock();
    }
    // ai_proc(argv[1], 2);
    ai_thread.join();
    // display_thread.join();

    // set stdin block
    flag = fcntl(STDIN_FILENO, F_GETFL);
    flag &= ~O_NONBLOCK;
    fcntl(STDIN_FILENO, F_SETFL, flag);
    return 0;
}

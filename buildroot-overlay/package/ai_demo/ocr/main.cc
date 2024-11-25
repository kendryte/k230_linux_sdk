/* Copyright (c) 2024, Canaan Bright Sight Co., Ltd
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
#include <iostream>
#include <thread>
#include "utils.h"
#include "vi_vo.h"
#include "dma_buf_manager.h"
#include "ocr_box.h"
#include "ocr_reco.h"
#include "text_paint.h"

using std::cerr;
using std::cout;
using std::endl;
using std::thread;

static std::mutex result_mutex;
static vector<ocr_det_res> results;
static vector<vector<Point2f>> vtds;
static vector<std::string> rec_results;
ChineseTextRenderer writepen("SourceHanSansSC-Normal-Min.ttf",24);

std::atomic<bool> ai_stop(false);
static volatile unsigned kpu_frame_count = 0;
static struct display* display;
static struct timeval tv, tv2;

#define dict_len 6549

void print_usage(const char *name)
{
    cout << "Usage: " << name << "<kmodel_det> <threshold> <box_thresh> <input_mode> <kmodel_reco> <debug_mode>" << endl
         << "Options:" << endl
         << "  kmodel_det      ocr检测kmodel路径\n"
         << "  threshold       置信度阈值:影响检测框的大小，置信度阈值越小，检测框越大，也更容易检测到文字。\n"
         << "  box_thresh      Box阈值：影响检测框的多少，最后输出的检测框分数小于Box阈值的会被剔除，大于Box阈值的会保留，过大的Box阈值导致漏检，过小的Box阈值导致误检。\n" 
         << "  input_mode      本地图片(图片路径)/ 摄像头(None) \n"
         << "  kmodel_reco     ocr识别kmodel路径 \n"
         << "  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
         << "\n"
         << endl;
}

// zero copy, use less memory
static void ai_proc_dmabuf(char *argv[], int video_device) {
    struct v4l2_drm_context context;
    struct v4l2_drm_video_buffer buffer;
    #define BUFFER_NUM 3

    // wait display_proc running
    result_mutex.lock();
    result_mutex.unlock();

    v4l2_drm_default_context(&context);
    context.device = video_device;
    context.display = false;
    context.width = SENSOR_WIDTH;
    context.height = SENSOR_HEIGHT;
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

    OCRBox ocrbox(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[6]));
    OCRReco ocrreco(argv[5],dict_len,atoi(argv[6]));
    std::vector<cv::Mat> sensor_bgr(3);
    cv::Mat ori_img;
    std::vector<unsigned char> ocr_result; 

    // create tensors
    std::vector<std::tuple<int, void*>> tensors;
    for (unsigned i = 0; i < BUFFER_NUM; i++) {
        tensors.push_back({context.buffers[i].fd, context.buffers[i].mmap});
    }
    DMABufManager dma_buf = DMABufManager({SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},tensors);
    
    while (!ai_stop) {
        int ret = v4l2_drm_dump(&context, 1000);
        if (ret) {
            perror("v4l2_drm_dump error");
            continue;
        }
        runtime_tensor& img_data = dma_buf.get_buf_for_index(context.vbuffer.index);
        ocrbox.pre_process(img_data);
        ocrbox.inference();
        result_mutex.lock();
        results.clear();
        ocrbox.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, results);

        sensor_bgr.clear();
        void* data=context.buffers[0].mmap;

        cv::Mat ori_img_B = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, data);
        cv::Mat ori_img_G = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, data + 1 * SENSOR_WIDTH * SENSOR_HEIGHT);
        cv::Mat ori_img_R = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, data + 2 * SENSOR_WIDTH * SENSOR_HEIGHT);

        // 检查每个图像是否非空
        if (ori_img_B.empty() || ori_img_G.empty() || ori_img_R.empty()) {
            std::cout << "One or more of the channel images is empty." << std::endl;
            continue;
        }
        sensor_bgr.push_back(ori_img_B);
        sensor_bgr.push_back(ori_img_G);
        sensor_bgr.push_back(ori_img_R);
        cv::merge(sensor_bgr, ori_img);

        rec_results.clear();
        for(int i = 0; i < results.size(); i++)
        {
            std::vector<Point> vec(4); 
            std::vector<Point2f> sort_vtd(4); 
            for (int j = 0; j < 4; ++j) {
                vec[j] = results[i].vertices[j];
            }
            cv::RotatedRect rect = cv::minAreaRect(vec);
            cv::Point2f ver[4];
            rect.points(ver);
            cv::Mat crop;
            ocrbox.warppersp(ori_img, crop, results[i], sort_vtd);
            vtds.push_back(sort_vtd);

            ocrreco.pre_process(crop);
            ocrreco.inference();
            std::string final_res=ocrreco.post_process();
            rec_results.push_back(final_res);
        }

        result_mutex.unlock();
        kpu_frame_count += 1;
        v4l2_drm_dump_release(&context);
    }
    v4l2_drm_stop(&context);
}

//display
int frame_handler(struct v4l2_drm_context *context, bool displayed) 
{
    
    static bool first_frame = true;
    if (first_frame) {
        result_mutex.unlock();
        first_frame = false;
    }

    static unsigned response = 0, display_frame_count = 0;
    response += 1;
    if (displayed) 
    {
        if (context[0].buffer_hold[context[0].wp] >= 0) 
        {
            static struct display_buffer* last_drawed_buffer = nullptr;
            auto buffer = context[0].display_buffers[context[0].buffer_hold[context[0].wp]];
            if (buffer != last_drawed_buffer) {
                auto img = cv::Mat(buffer->height, buffer->width, CV_8UC3, buffer->map);
                result_mutex.lock();
                OCRBox::draw_ocr_video(img, results, rec_results, {img.cols, img.rows}, {SENSOR_WIDTH, SENSOR_HEIGHT},writepen);
                result_mutex.unlock();
                last_drawed_buffer = buffer;
                // flush cache
                thead_csi_dcache_clean_invalid_range(buffer->map, buffer->size);
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

static void display_proc(int video_device) 
{
    struct v4l2_drm_context context;
    v4l2_drm_default_context(&context);
    context.device = video_device;
    context.width = display->width;
    context.height = (display->width * SENSOR_HEIGHT / SENSOR_WIDTH) & 0xfff8;
    context.video_format = V4L2_PIX_FMT_BGR24;
    context.display_format = 0; // auto
    if (v4l2_drm_setup(&context, 1, &display)) {
        std::cerr << "v4l2_drm_setup error" << std::endl;
        return;
    }
    std::cout << "press 'q' to exit" << std::endl;
    std::cout << "press 'd' to save a picture" << std::endl;
    gettimeofday(&tv, NULL);
    v4l2_drm_run(&context, 1, frame_handler);
    if (display) {
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
    std::cout << "case " << argv[0] << " built at " << __DATE__ << " " << __TIME__ << std::endl;
    if (argc != 7)
    {
        print_usage(argv[0]);
        return -1;
    }

    if (strcmp(argv[4], "None") == 0)
    {
        std::cout<<"isp start"<<std::endl;
        display = display_init(0);
        if (!display) {
            cerr << "display_init error, exit" << endl;
            return -1;
        }

        // set stdin non-block
        int flag = fcntl(STDIN_FILENO, F_GETFL);
        flag |= O_NONBLOCK;
        if (fcntl(STDIN_FILENO, F_SETFL, flag)) {
            cerr << "can't set stdin non-block" << endl;
            return -1;
        }

        result_mutex.lock();
        auto ai_thread = thread(ai_proc_dmabuf, argv, 2);
        display_proc(1);
        ai_thread.join();

        // set stdin block
        flag = fcntl(STDIN_FILENO, F_GETFL);
        flag &= ~O_NONBLOCK;
        fcntl(STDIN_FILENO, F_SETFL, flag);
    }
    else
    {
        ChineseTextRenderer writepen("SourceHanSansSC-Normal-Min.ttf", 24);
        vector<ocr_det_res> results;
        cv::Mat ori_img = cv::imread(argv[4]);
        cv::Mat draw_img = ori_img.clone();
        int ori_w = ori_img.cols;
        int ori_h = ori_img.rows;
        OCRBox ocrbox(argv[1], atof(argv[2]), atof(argv[3]), atoi(argv[6]));
        OCRReco ocrreco(argv[5],dict_len,atoi(argv[6]));
        ocrbox.pre_process(ori_img);
        ocrbox.inference();
        ocrbox.post_process({ori_w, ori_h}, results);
        vector<string> rec_results;

        for(int i = 0; i < results.size(); i++)
        {
            vector<Point> vec;
            vector<Point2f> sort_vtd(4);
            vec.clear();
            for(int j = 0; j < 4; j++)
                vec.push_back(results[i].vertices[j]);
            cv::RotatedRect rect = cv::minAreaRect(vec);
            cv::Point2f ver[4];
            rect.points(ver);
            cv::Mat crop;
            ocrbox.warppersp(ori_img, crop, results[i], sort_vtd);
            ocrreco.pre_process(crop);
            ocrreco.inference();
            std::string text=ocrreco.post_process();
            rec_results.push_back(text);
        }
        ocrbox.draw_ocr_image(draw_img, results,rec_results,writepen);
        results.clear();
        cv::imwrite("ocr_result.jpg", draw_img);

    }
    return 0;
}
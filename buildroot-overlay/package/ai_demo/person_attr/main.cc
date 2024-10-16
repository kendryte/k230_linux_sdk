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
#include "person_detect.h"
#include "pulc.h"

using std::cerr;
using std::cout;
using std::endl;
using std::thread;

static std::mutex result_mutex;
std::atomic<bool> ai_stop(false);
static volatile unsigned kpu_frame_count = 0;
static struct display* display;
static struct timeval tv, tv2;

int osd_width = -1,osd_height =-1;
cv::Mat osd_frame;

void print_usage(const char *name)
{
    cout << "Usage: " << name << "<kmodel> <pd_thresh> <nms_thresh> <input_mode> <attr_kmodel> <pulc_thresh> <glasses_thresh> <hold_thresh> <debug_mode>" << endl
         << "For example: " << endl
         << " [for img] ./person_attr.elf person_attr_yolov5n.kmodel 0.5 0.45 hrnet_demo.jpg person_pulc.kmodel 0.5 0.5 0.5 0" << endl
         << " [for isp] ./person_attr.elf person_attr_yolov5n.kmodel 0.5 0.45 None person_pulc.kmodel 0.5 0.5 0.5 0" << endl
         << "Options:" << endl
         << " 1> kmodel    行人检测kmodel文件路径 \n"
         << " 2> pd_thresh  行人检测阈值\n"
         << " 3> nms_thresh  NMS阈值\n"
         << " 4> input_mode      本地图片(图片路径)/ 摄像头(None) \n"
         << " 5> attr_kmodel 属性识别kmodel文件路径 \n"
         << " 6> pulc_thresh 属性识别阈值 \n"
         << " 7> glasses_thresh 是否配戴眼镜阈值 \n"
         << " 8> hold_thresh 是否持物阈值 \n"
         << " 9> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
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

    // create tensors
    std::vector<std::tuple<int, void*>> tensors;
    for (unsigned i = 0; i < BUFFER_NUM; i++) {
        tensors.push_back({context.buffers[i].fd, context.buffers[i].mmap});
    }
    DMABufManager dma_buf = DMABufManager({SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},tensors);

    personDetect pd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[9]));
    float pulc_thresh = atof(argv[6]);
    float glasses_thresh = atof(argv[7]);
    float hold_thresh = atof(argv[8]);
    Pulc pul(argv[5], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},pulc_thresh, glasses_thresh, hold_thresh, atoi(argv[9]));
    vector<BoxInfo> results;

    while (!ai_stop) {
        int ret = v4l2_drm_dump(&context, 1000);
        if (ret) {
            perror("v4l2_drm_dump error");
            continue;
        }
        auto img_buff = dma_buf.get_buf_for_index(context.vbuffer.index);
        pd.pre_process(img_buff);
        pd.inference();
        result_mutex.lock();
        results.clear();
        pd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, results);
        
        osd_frame = cv::Mat(osd_height, osd_width, CV_8UC3, cv::Scalar(0, 0, 0));

        std::string text;
        cv::Point origin;
        Bbox crop_box;
        std::string attr_results;

        for (auto r : results)
        {
            ScopedTiming st("draw boxes", atoi(argv[9]));
            text = pd.labels[r.label] + ":" + std::to_string(round(r.score * 100) / 100).substr(0,4);

            int x1 =  r.x1 / SENSOR_WIDTH * osd_width;
            int y1 =  r.y1 / SENSOR_HEIGHT  * osd_height;

            int w = (r.x2-r.x1) / SENSOR_WIDTH * osd_width;
            int h = (r.y2-r.y1) / SENSOR_HEIGHT  * osd_height;

            cv::rectangle(osd_frame, cv::Rect( x1,y1,w,h ), cv::Scalar(255,0, 0), 2, 2, 0); // ARGB
            
            crop_box = { r.x1,r.y1,(r.x2-r.x1),(r.y2-r.y1) };
            pul.pre_process(img_buff, crop_box );
            pul.inference();
            attr_results = pul.post_process();

            origin.x = r.x1;
            origin.y = r.y1 + 20;

            float fontScale = 2;
            int with = 80;
            string gender = pul.GetGender();
            origin.y += with;
            cv::putText(osd_frame, gender, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 255, 255), 1, 8, 0);
            string age = pul.GetAge();
            origin.y += with;
            cv::putText(osd_frame, age, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 255, 255), 1, 8, 0);
            string direction = pul.GetDirection();
            origin.y += with;
            cv::putText(osd_frame, direction, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 255, 255), 1, 8, 0);
           
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
                osd_frame.copyTo(img,osd_frame);
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
    osd_width = context.width,osd_height = context.height;
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
    if (argc != 10)
    {
        print_usage(argv[0]);
        return -1;
    }

    if (strcmp(argv[4], "None") == 0)
    {
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
        cv::Mat ori_img = cv::imread(argv[4]);
        int ori_w = ori_img.cols;
        int ori_h = ori_img.rows;
        personDetect pd(argv[1], atof(argv[2]),atof(argv[3]), atoi(argv[9]));
        pd.pre_process(ori_img);
        pd.inference();

        vector<BoxInfo> results;
        pd.post_process({ori_w, ori_h}, results);
        
        float pulc_thresh = atof(argv[6]);
        float glasses_thresh = atof(argv[7]);
        float hold_thresh = atof(argv[8]);
        Pulc pul( argv[5],pulc_thresh,glasses_thresh,hold_thresh,atoi(argv[9]) );
        std::string attr_results;

        
        for (auto r : results)
        {
            ScopedTiming st("draw boxes", atoi(argv[9]));
            std::string text = pd.labels[r.label] + ":" + std::to_string(round(r.score * 100) / 100).substr(0,4);
            std::cout << "text = " << text << std::endl;
            cv::rectangle(ori_img, cv::Rect(r.x1, r.y1, r.x2 - r.x1 + 1, r.y2 - r.y1 + 1), cv::Scalar(0, 0, 255), 2, 2, 0);
            cv::Point origin;

            origin.x = r.x1;
            origin.y = r.y1 - 20;

            cv::putText(ori_img, text, origin, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 255, 255), 1, 8, 0);
            pul.pre_process(ori_img, r);
            pul.inference();
            attr_results = pul.post_process();

            float fontScale = 0.5;
            int with = 25;
            string gender = pul.GetGender();
            origin.y += with;
            cv::putText(ori_img, gender, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 255, 0, 255), 1, 8, 0);
            string age = pul.GetAge();
            origin.y += with;
            cv::putText(ori_img, age, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 255, 0, 255), 1, 8, 0);
            string direction = pul.GetDirection();
            origin.y += with;
            cv::putText(ori_img, direction, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 255, 0, 255), 1, 8, 0);
            string glasses = pul.GetGlasses();
            origin.y += with;
            cv::putText(ori_img, glasses, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 255, 0, 255), 1, 8, 0);
            string bag = pul.GetBag();
            origin.y += with;
            cv::putText(ori_img, bag, origin, cv::FONT_HERSHEY_COMPLEX, fontScale, cv::Scalar(255, 255, 0, 255), 1, 8, 0);
           

        }

        cv::imwrite("person_attr_result.jpg", ori_img);
    }
    return 0;
}
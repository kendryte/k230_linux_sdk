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
#include "BYTETracker.h"

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

const float color_list[80][3] =
{
    {0.000, 0.447, 0.741},
    {0.850, 0.325, 0.098},
    {0.929, 0.694, 0.125},
    {0.494, 0.184, 0.556},
    {0.466, 0.674, 0.188},
    {0.301, 0.745, 0.933},
    {0.635, 0.078, 0.184},
    {0.300, 0.300, 0.300},
    {0.600, 0.600, 0.600},
    {1.000, 0.000, 0.000},
    {1.000, 0.500, 0.000},
    {0.749, 0.749, 0.000},
    {0.000, 1.000, 0.000},
    {0.000, 0.000, 1.000},
    {0.667, 0.000, 1.000},
    {0.333, 0.333, 0.000},
    {0.333, 0.667, 0.000},
    {0.333, 1.000, 0.000},
    {0.667, 0.333, 0.000},
    {0.667, 0.667, 0.000},
    {0.667, 1.000, 0.000},
    {1.000, 0.333, 0.000},
    {1.000, 0.667, 0.000},
    {1.000, 1.000, 0.000},
    {0.000, 0.333, 0.500},
    {0.000, 0.667, 0.500},
    {0.000, 1.000, 0.500},
    {0.333, 0.000, 0.500},
    {0.333, 0.333, 0.500},
    {0.333, 0.667, 0.500},
    {0.333, 1.000, 0.500},
    {0.667, 0.000, 0.500},
    {0.667, 0.333, 0.500},
    {0.667, 0.667, 0.500},
    {0.667, 1.000, 0.500},
    {1.000, 0.000, 0.500},
    {1.000, 0.333, 0.500},
    {1.000, 0.667, 0.500},
    {1.000, 1.000, 0.500},
    {0.000, 0.333, 1.000},
    {0.000, 0.667, 1.000},
    {0.000, 1.000, 1.000},
    {0.333, 0.000, 1.000},
    {0.333, 0.333, 1.000},
    {0.333, 0.667, 1.000},
    {0.333, 1.000, 1.000},
    {0.667, 0.000, 1.000},
    {0.667, 0.333, 1.000},
    {0.667, 0.667, 1.000},
    {0.667, 1.000, 1.000},
    {1.000, 0.000, 1.000},
    {1.000, 0.333, 1.000},
    {1.000, 0.667, 1.000},
    {0.333, 0.000, 0.000},
    {0.500, 0.000, 0.000},
    {0.667, 0.000, 0.000},
    {0.833, 0.000, 0.000},
    {1.000, 0.000, 0.000},
    {0.000, 0.167, 0.000},
    {0.000, 0.333, 0.000},
    {0.000, 0.500, 0.000},
    {0.000, 0.667, 0.000},
    {0.000, 0.833, 0.000},
    {0.000, 1.000, 0.000},
    {0.000, 0.000, 0.167},
    {0.000, 0.000, 0.333},
    {0.000, 0.000, 0.500},
    {0.000, 0.000, 0.667},
    {0.000, 0.000, 0.833},
    {0.000, 0.000, 1.000},
    {0.000, 0.000, 0.000},
    {0.143, 0.143, 0.143},
    {0.286, 0.286, 0.286},
    {0.429, 0.429, 0.429},
    {0.571, 0.571, 0.571},
    {0.714, 0.714, 0.714},
    {0.857, 0.857, 0.857},
    {0.000, 0.447, 0.741},
    {0.314, 0.717, 0.741},
    {0.50, 0.5, 0}
};


void print_usage(const char *name)
{
    cout << "Usage: " << name << " <kmodel> <pd_thresh> <nms_thresh> <input_mode> <debug_mode> <fps> <buffer>" << endl
         << "For example: " << endl
         << " [for isp] ./bytetrack.elf bytetrack_yolov5n.kmodel 0.5 0.45 None 0 24 30" << endl
         << " [for img] ./bytetrack.elf bytetrack_yolov5n.kmodel 0.5 0.45 277 0 24 30" << endl
         << "Options:" << endl
         << " 1> kmodel    bytetrack行人检测kmodel文件路径 \n"
         << " 2> pd_thresh  行人检测阈值 \n"
         << " 3> nms_thresh  NMS阈值 \n"
         << " 4> input_mode       图像 (Number) or 摄像头(None) \n"
         << " 5> debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试 \n"
         << " 6> fps         帧率 \n" 
         << " 7> buffer      容忍帧数，即超过多少帧之后无法匹配上某个track，就认为该track丢失 \n"
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

    
    personDetect pd(argv[1], atof(argv[2]),atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[5]));
    
    // create tensors
    std::vector<std::tuple<int, void*>> tensors;
    for (unsigned i = 0; i < BUFFER_NUM; i++) {
        tensors.push_back({context.buffers[i].fd, context.buffers[i].mmap});
    }
    DMABufManager dma_buf = DMABufManager({SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},tensors);

    std::vector<BoxInfo> results;
    std::vector<Object> objects;

    // int fps = 24;
    int fps = atoi(argv[6]);
    int buffer_track = atoi(argv[7]);
    BYTETracker tracker(fps, buffer_track);

    while (!ai_stop) {
        int ret = v4l2_drm_dump(&context, 1000);
        if (ret) {
            perror("v4l2_drm_dump error");
            continue;
        }

        {
            ScopedTiming st("results and objects clear ", atoi(argv[5]));
            results.clear();
            objects.clear();
        }

        pd.pre_process(dma_buf.get_buf_for_index(context.vbuffer.index));
        pd.inference();
        result_mutex.lock();
        results.clear();
        pd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, results);
        
        osd_frame = cv::Mat(osd_height, osd_width, CV_8UC3, cv::Scalar(0, 0, 0));
        std::string text;
        cv::Point origin;

        for (auto res : results)
        {
            ScopedTiming st("results transfer ", atoi(argv[5]));
            
            Object obj{ {res.x1,res.y1,res.x2- res.x1,res.y2- res.y1},res.label,res.score };
            objects.push_back(obj);

        }
        
        std::vector<STrack> output_stracks;
        {
            ScopedTiming st("tracker.update ", atoi(argv[5]));
            output_stracks = tracker.update(objects);
        }

        for (int i = 0; i < output_stracks.size(); i++)
        {
            ScopedTiming st("draw boxes", atoi(argv[5]));
            std::vector<float> tlwh = output_stracks[i].tlwh;
            bool vertical = tlwh[2] / tlwh[3] > 1.6;
            if (tlwh[2] * tlwh[3] > 20 && !vertical)
            {
                cv::Scalar s = tracker.get_color(output_stracks[i].track_id);

                int x1 =  tlwh[0] / SENSOR_WIDTH * osd_width;
                int y1 =  tlwh[1] / SENSOR_HEIGHT  * osd_height;

                int w = tlwh[2] / SENSOR_WIDTH * osd_width;
                int h = tlwh[3] / SENSOR_HEIGHT  * osd_height;
                
                cv::putText(osd_frame, format("%d", output_stracks[i].track_id), Point(x1, y1 + 5), 0, 2, Scalar(255, 0, 0), 2, LINE_AA);
                cv::rectangle(osd_frame, Rect(x1,y1,w,h), s, 2);
            }
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
    if (argc != 8)
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
        personDetect pd(argv[1], atof(argv[2]),atof(argv[3]),  atoi(argv[5]));
        std::vector<BoxInfo> results;
        std::vector<Object> objects;

        // int fps = 24;
        int fps = atoi(argv[6]);
        int buffer = atoi(argv[7]);
        BYTETracker tracker(fps, buffer);

        int num_frames = 0;
        int total_ms = 0;

        int fileNum = atoi(argv[4]);

        while(true)
        {

            objects.clear();
            results.clear();
            if (num_frames > fileNum)
                break;

            std::string frame_pth =  "bytetrack_data/images/" + std::to_string(num_frames) + ".jpg";

            cv::Mat img = cv::imread(frame_pth);

            pd.pre_process(img);
            pd.inference();
            pd.post_process({img.cols, img.rows}, results);

            cv::Point origin;

            for (auto res : results)
            {
                ScopedTiming st("results transfer ", atoi(argv[5]));
                
                Object obj{ {res.x1,res.y1,res.x2- res.x1,res.y2- res.y1},res.label,res.score };
                objects.push_back(obj);

            }

            std::vector<STrack> output_stracks = tracker.update(objects);
            {
                ScopedTiming st("tracker.update ", atoi(argv[5]));
                output_stracks = tracker.update(objects);
            }

            for (int i = 0; i < output_stracks.size(); i++)
            {
                ScopedTiming st("draw boxes", atoi(argv[5]));
                std::vector<float> tlwh = output_stracks[i].tlwh;
                bool vertical = tlwh[2] / tlwh[3] > 1.6;
                if (tlwh[2] * tlwh[3] > 20 && !vertical)
                {
                    Scalar s = tracker.get_color_img(output_stracks[i].track_id);

                    putText(img, format("%d", output_stracks[i].track_id), Point(tlwh[0], tlwh[1] - 5), 0, 0.6, Scalar(0, 0, 255), 2, LINE_AA);
                    rectangle(img, Rect(tlwh[0], tlwh[1], tlwh[2], tlwh[3]), s, 2);
                }
            }
            putText(img, format("frame: %d  num: %d", num_frames, output_stracks.size()), 
                    Point(0, 30), 0, 0.6, Scalar(0, 0, 255), 2, LINE_AA);
            
            std::string out_path = "bytetrack_data/output/" + std::to_string(num_frames) + ".jpg";

            cv::imwrite(out_path, img);

            num_frames++;
        }
    }
    return 0;
}
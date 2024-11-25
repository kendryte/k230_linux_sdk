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
#include "hand_detection.h"
#include "hand_keypoint.h"
#include "dynamic_gesture.h"

using std::cerr;
using std::cout;
using std::endl;
using std::thread;

static std::mutex result_mutex;
int osd_width = -1,osd_height =-1;
cv::Mat osd_frame;
std::atomic<bool> ai_stop(false);
static volatile unsigned kpu_frame_count = 0;
static struct display* display;
static struct timeval tv, tv2;

void print_usage(const char *name)
{
	cout << "Usage: " << name << "<kmodel_det> <obj_thresh> <nms_thresh> <kmodel_kp> <kmodel_gesture> <debug_mode>" << endl
		 << "Options:" << endl
         << "  kmodel_det       手掌检测kmodel路径\n"
         << "  obj_thresh       手掌检测阈值\n"
         << "  nms_thresh       手掌检测非极大值抑制阈值\n"
		 << "  kmodel_kp        手势关键点检测kmodel路径\n"
		 << "  kmodel_gesture   动态手势识别kmodel路径\n"
		 << "  debug_mode       是否需要调试, 0、1、2分别表示不调试、简单调试、详细调试\n"
		 << "\n"
		 << endl;
}

float calculateIntDidtance(cv::Point pt1, cv::Point pt2) {
    return cv::sqrt((pt1.x - pt2.x) * (pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y));
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

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[6]));
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[6]));
    DynamicGesture Dag(argv[5], atoi(argv[6]));

    // create tensors
    std::vector<std::tuple<int, void*>> tensors;
    for (unsigned i = 0; i < BUFFER_NUM; i++) {
        tensors.push_back({context.buffers[i].fd, context.buffers[i].mmap});
    }
    DMABufManager dma_buf = DMABufManager({SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},tensors);
    
    std::vector<BoxInfo> results;

    enum state {TRIGGER,UP,RIGHT,DOWN,LEFT,MIDDLE} cur_state_ = TRIGGER, pre_state_ = TRIGGER, draw_state_ = TRIGGER;

    int idx_ = 0;
    int idx = 0;
    std::vector<int> history ={2};
    std::vector<vector<float>> history_logit ;
    std::vector<int> vec_flag;
    std::chrono::steady_clock::time_point m_start; // 计时开始时间
	std::chrono::steady_clock::time_point m_stop;  // 计时结束时间
    std::chrono::steady_clock::time_point s_start; // 图片显示计时开始时间
    std::chrono::steady_clock::time_point s_stop;  // 图片显示计时结束时间

    int bin_width = 150;
    int bin_height = 216;
    cv::Mat shang_argb;
    DynamicGesture::bin_2_mat("shang.bin", bin_width, bin_height, shang_argb);
    cv::Mat xia_argb;
    DynamicGesture::bin_2_mat("xia.bin", bin_width, bin_height, xia_argb);
    cv::Mat zuo_argb;
    DynamicGesture::bin_2_mat("zuo.bin", bin_height, bin_width, zuo_argb);
    cv::Mat you_argb;
    DynamicGesture::bin_2_mat("you.bin", bin_height, bin_width, you_argb);

    while (!ai_stop) {
        int ret = v4l2_drm_dump(&context, 1000);
        if (ret) {
            perror("v4l2_drm_dump error");
            continue;
        }
        
        runtime_tensor& img_data = dma_buf.get_buf_for_index(context.vbuffer.index);
        
        result_mutex.lock();
        osd_frame = cv::Mat(osd_height, osd_width, CV_8UC3, cv::Scalar(0, 0, 0));

        if (cur_state_== TRIGGER)
        {
            ScopedTiming st("trigger time", atoi(argv[6]));
            results.clear();

            hd.pre_process(img_data);
            hd.inference();
            hd.post_process(results);

            for (auto r: results)
            {
                int w = r.x2 - r.x1 + 1;
                int h = r.y2 - r.y1 + 1;
 
                int length = std::max(w,h)/2;
                int cx = (r.x1+r.x2)/2;
                int cy = (r.y1+r.y2)/2;
                int ratio_num = 1.26*length;

                int x1_1 = std::max(0,cx-ratio_num);
                int y1_1 = std::max(0,cy-ratio_num);
                int x2_1 = std::min(SENSOR_WIDTH-1, cx+ratio_num);
                int y2_1 = std::min(SENSOR_HEIGHT-1, cy+ratio_num);
                int w_1 = x2_1 - x1_1 + 1;
                int h_1 = y2_1 - y1_1 + 1;
                
                struct Bbox bbox = {x:x1_1,y:y1_1,w:w_1,h:h_1};
                hk.pre_process(img_data, bbox);
                hk.inference();
                hk.post_process(bbox);

                std::vector<double> angle_list = hk.hand_angle();
                std::string gesture = hk.h_gesture(angle_list);

                if ((gesture == "five") ||(gesture == "yeah"))
                {
                    double v1_x = hk.results[24] - hk.results[0];
                    double v1_y = hk.results[25] - hk.results[1];
                    double v2_x = 1.0; 
                    double v2_y = 0.0;

                    // 掌根到中指指尖向量和（1，0）向量的夹角
                    double v1_norm = std::sqrt(v1_x * v1_x + v1_y * v1_y);
                    double v2_norm = std::sqrt(v2_x * v2_x + v2_y * v2_y);
                    double dot_product = v1_x * v2_x + v1_y * v2_y;
                    double cos_angle = dot_product / (v1_norm * v2_norm);
                    double angle = std::acos(cos_angle) * 180 / M_PI;

                    if (v1_y>0)
                    {
                        angle = 360-angle;
                    }

                    if ((70.0<=angle) && (angle<110.0))
                    {
                        if ((pre_state_ != UP) || (pre_state_ != MIDDLE))
                        {
                            vec_flag.push_back(pre_state_);
                        }
                        if ((vec_flag.size()>10)||(pre_state_ == UP) || (pre_state_ == MIDDLE) ||(pre_state_ == TRIGGER))
                        {
                            cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_width,bin_height));
                            shang_argb.copyTo(copy_image); 
                            cur_state_ = UP;
                        }
                    }
                    else if ((110.0<=angle) && (angle<225.0))
                    {
                        // 中指向右(实际方向)
                        if (pre_state_ != RIGHT)
                        {
                            vec_flag.push_back(pre_state_);
                        }
                        if ((vec_flag.size()>10)||(pre_state_ == RIGHT)||(pre_state_ == TRIGGER))
                        {
                            cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_height,bin_width));
                            you_argb.copyTo(copy_image); 
                            cur_state_ = RIGHT;
                        }
                    }
                    else if((225.0<=angle) && (angle<315.0))
                    {
                        if (pre_state_ != DOWN)
                        {
                            vec_flag.push_back(pre_state_);
                        }
                        if ((vec_flag.size()>10)||(pre_state_ == DOWN)||(pre_state_ == TRIGGER))
                        {
                            cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_width,bin_height));
                            xia_argb.copyTo(copy_image); 
                            cur_state_ = DOWN;
                        }
                    }
                    else
                    {
                        if (pre_state_ != LEFT)
                        {
                            vec_flag.push_back(pre_state_);
                        }
                        if ((vec_flag.size()>10)||(pre_state_ == LEFT)||(pre_state_ == TRIGGER))
                        {
                            cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_height,bin_width));
                            zuo_argb.copyTo(copy_image);
                            cur_state_ = LEFT;
                        }
                    }
                    m_start = std::chrono::steady_clock::now();
                }
            }
            history_logit.clear();
        }
        else if (cur_state_ != TRIGGER)
        {
            ScopedTiming st("swipe time", atoi(argv[6]));
            {
                int matsize = SENSOR_WIDTH * SENSOR_HEIGHT;
                cv::Mat ori_img;
                auto img_data_buf = img_data.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
                cv::Mat ori_img_R = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, img_data_buf.data());
                cv::Mat ori_img_G = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, img_data_buf.data() + 1 * matsize);
                cv::Mat ori_img_B = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, img_data_buf.data() + 2 * matsize);
                std::vector<cv::Mat> sensor_rgb;
                sensor_rgb.push_back(ori_img_R);
                sensor_rgb.push_back(ori_img_G);
                sensor_rgb.push_back(ori_img_B);
                cv::merge(sensor_rgb, ori_img);

                Dag.pre_process(ori_img);
                Dag.inference();
                Dag.post_process();

                vector<float> avg_logit;
                {
                    vector<float> output;
                    Dag.get_out(output);
                    history_logit.push_back(output);

                    for(int j=0;j<27;j++)
                    {
                        float sum = 0.0;
                        for (int i=0;i<history_logit.size();i++)
                        {
                            sum += history_logit[i][j];
                        }
                        avg_logit.push_back(sum / history_logit.size());
                    }
                    idx_ = std::distance(avg_logit.begin(), std::max_element(avg_logit.begin(), avg_logit.end()));
                }

                idx = Dag.process_output(idx_, history);

                if (idx!=idx_)
                {
                    vector<float> history_logit_last = history_logit.back();
                    history_logit.clear();
                    history_logit.push_back(history_logit_last);
                }

                if (cur_state_ == UP)
                {
                    cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_width,bin_height));
                    shang_argb.copyTo(copy_image); 
                    if ((idx==15) || (idx==10))
                    {
                        vec_flag.clear();
                        if (((avg_logit[idx] >= 0.7) && (history_logit.size() >= 2)) || ((avg_logit[idx] >= 0.3) && (history_logit.size() >= 4)))
                        {
                            s_start = std::chrono::steady_clock::now();
                            cur_state_ = TRIGGER;
                            draw_state_ = DOWN;
                            history.clear();
                        }
                        pre_state_ = UP;
                    }else if ((idx==25)||(idx==26)) 
                    {
                        vec_flag.clear();
                        if (((avg_logit[idx] >= 0.4) && (history_logit.size() >= 2)) || ((avg_logit[idx] >= 0.3) && (history_logit.size() >= 3)))
                        {
                            s_start = std::chrono::steady_clock::now();
                            cur_state_ = TRIGGER;
                            draw_state_ = MIDDLE;
                            history.clear();
                        }
                        pre_state_ = MIDDLE;
                    }else
                    {
                        history_logit.clear();
                    }
                }
                else if (cur_state_ == RIGHT)
                {
                    cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_height,bin_width));
                    you_argb.copyTo(copy_image); 
                    if  ((idx==16)||(idx==11)) 
                    {
                        vec_flag.clear();
                        if (((avg_logit[idx] >= 0.4) && (history_logit.size() >= 2)) || ((avg_logit[idx] >= 0.3) && (history_logit.size() >= 3)))
                        {
                            s_start = std::chrono::steady_clock::now();
                            cur_state_ = TRIGGER;
                            draw_state_ = RIGHT;
                            history.clear();
                        }
                        pre_state_ = RIGHT;
                    }else
                    {
                        history_logit.clear();
                    }
                }
                else if (cur_state_ == DOWN)
                {
                    cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_width,bin_height));
                    xia_argb.copyTo(copy_image); 
                    if  ((idx==18)||(idx==13))
                    {
                        vec_flag.clear();
                        if (((avg_logit[idx] >= 0.4) && (history_logit.size() >= 2)) || ((avg_logit[idx] >= 0.3) && (history_logit.size() >= 3)))
                        {
                            s_start = std::chrono::steady_clock::now();
                            cur_state_ = TRIGGER;
                            draw_state_ = UP;
                            history.clear();
                        }
                        pre_state_ = DOWN;
                    }else
                    {
                        history_logit.clear();
                    }
                }
                else if (cur_state_ == LEFT)
                {
                    cv::Mat copy_image = osd_frame(cv::Rect(0,0,bin_height,bin_width));
                    zuo_argb.copyTo(copy_image);
                    if ((idx==17)||(idx==12))
                    {
                        vec_flag.clear();
                        if (((avg_logit[idx] >= 0.4) && (history_logit.size() >= 2)) || ((avg_logit[idx] >= 0.3) && (history_logit.size() >= 3)))
                        {
                            s_start = std::chrono::steady_clock::now();
                            cur_state_ = TRIGGER;
                            draw_state_ = LEFT;
                            history.clear();
                        }
                        pre_state_ = LEFT;
                    }else
                    {
                        history_logit.clear();
                    }
                }
            }
            m_stop = std::chrono::steady_clock::now();
			double elapsed_ms = std::chrono::duration<double, std::milli>(m_stop - m_start).count();

            if ((cur_state_ != TRIGGER) &&(elapsed_ms>2000))
            {
                cur_state_ = TRIGGER;
                pre_state_ = TRIGGER;
            }
        }
        s_stop = std::chrono::steady_clock::now();
        double elapsed_ms_show = std::chrono::duration<double, std::milli>(s_stop - s_start).count();

        if (elapsed_ms_show<1000)
        {
            if (draw_state_ == UP)
            {
                cv::putText(osd_frame, "UP", cv::Point(osd_width*3/7,osd_height/2),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 195, 0), 2);
            } else if (draw_state_ == RIGHT)
            {
                cv::putText(osd_frame, "LEFT", cv::Point(osd_width*3/7,osd_height/2),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 195, 0), 2);
            }else if (draw_state_ == DOWN)
            {
                cv::putText(osd_frame, "DOWN", cv::Point(osd_width*3/7,osd_height/2),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 195, 0), 2);
            }else if (draw_state_ == LEFT)
            {
                cv::putText(osd_frame, "RIGHT", cv::Point(osd_width*3/7,osd_height/2),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 195, 0), 2);
            }else if (draw_state_ == MIDDLE)
            {
                cv::putText(osd_frame, "MIDDLE", cv::Point(osd_width*3/7,osd_height/2),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 195, 0), 2);
            }

        }else
        {
            draw_state_ = TRIGGER;
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
    if (argc != 7)
    {
        print_usage(argv[0]);
        return -1;
    }
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
    return 0;
}
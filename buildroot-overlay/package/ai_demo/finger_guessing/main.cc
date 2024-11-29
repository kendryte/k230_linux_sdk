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
#include "sensor_buf_manager.h"
#include "hand_detection.h"
#include "hand_keypoint.h"

using std::cerr;
using std::cout;
using std::endl;
using std::thread;

static std::mutex result_mutex;
int osd_width = -1,osd_height =-1;
cv::Mat osd_frame;
cv::Mat osd_frame_tmp;
std::atomic<bool> ai_stop(false);
static volatile unsigned kpu_frame_count = 0;
static struct display* display;
static struct timeval tv, tv2;

void print_usage(const char *name)
{
	cout << "Usage: " << name << "<kmodel_det> <obj_thresh> <nms_thresh> <kmodel_kp> <guess_mode> <debug_mode>" << endl
		 << "Options:" << endl
		 << "  kmodel_det      手掌检测kmodel路径\n"
         << "  obj_thresh      手掌检测阈值\n"
         << "  nms_thresh      手掌检测非极大值抑制阈值\n"
		 << "  kmodel_kp       手势关键点检测kmodel路径\n"
         << "  guess_mode      石头剪刀布的游戏模式 0(玩家稳赢) 1(玩家必输) 奇数n(n局定输赢)\n"
		 << "  debug_mode      是否需要调试, 0、1、2分别表示不调试、简单调试、详细调试\n"
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

    
    // 读取石头剪刀布的bin文件数据 并且转换为mat类型
    int bu_width = 400;
    int bu_height = 400;
    cv::Mat image_bu_argb;
    HandKeypoint::bin_2_mat("bu.bin", bu_width, bu_height, image_bu_argb);

    int shitou_width = 400;
    int shitou_height = 400;
    cv::Mat image_shitou_argb;
    HandKeypoint::bin_2_mat("shitou.bin", shitou_width, shitou_height, image_shitou_argb);

    int jiandao_width = 400;
    int jiandao_height = 400;
    cv::Mat image_jiandao_argb;
    HandKeypoint::bin_2_mat("jiandao.bin", jiandao_width, jiandao_height, image_jiandao_argb);

    // 设置游戏模式
    static int MODE = atoi(argv[5]);
    int counts_guess = -1;
    int player_win = 0;
    int k230_win = 0;
    bool sleep_end = false;
    bool set_stop_id = true;
    std::vector<std::string> LIBRARY = {"fist","yeah","five"};

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[6]));
    sync();
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[6]));
    sync();

    std::vector<BoxInfo> results;

    // create tensors
    std::vector<std::tuple<int, void*>> tensors;
    for (unsigned i = 0; i < BUFFER_NUM; i++) {
        tensors.push_back({context.buffers[i].fd, context.buffers[i].mmap});
    }
    SensorBufManager sensor_buf = SensorBufManager({SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},tensors);

    while (!ai_stop) {
        int ret = v4l2_drm_dump(&context, 1000);
        if (ret) {
            perror("v4l2_drm_dump error");
            continue;
        }
        
        runtime_tensor& img_data = sensor_buf.get_buf_for_index(context.vbuffer.index);
        
        hd.pre_process(img_data);
        hd.inference();
        result_mutex.lock();
        results.clear();
        hd.post_process(results);
        osd_frame = cv::Mat(osd_height, osd_width, CV_8UC3, cv::Scalar(0, 0, 0));
        osd_frame_tmp = cv::Mat(osd_height, osd_width, CV_8UC3, cv::Scalar(0, 0, 0));

        float max_area_hand = 0;
        int max_id_hand = -1;
        for (int i = 0; i < results.size(); ++i)
        {
            float area_i = (results[i].x2 - results[i].x1) * (results[i].y2 - results[i].y1);
            if (area_i > max_area_hand)
            {
                max_area_hand = area_i;
                max_id_hand = i;
            }
        }

        std::string gesture = "";
        if (max_id_hand != -1)
        {
            // std::string text = hd.labels_[results[max_id_hand].label] + ":" + std::to_string(round(results[max_id_hand].score * 100) / 100.0);

            int w = results[max_id_hand].x2 - results[max_id_hand].x1 + 1;
            int h = results[max_id_hand].y2 - results[max_id_hand].y1 + 1;
            
            int rect_x = results[max_id_hand].x1/ SENSOR_WIDTH * osd_width;
            int rect_y = results[max_id_hand].y1/ SENSOR_HEIGHT * osd_height;
            int rect_w = (float)w / SENSOR_WIDTH * osd_width;
            int rect_h = (float)h / SENSOR_HEIGHT  * osd_height;
            
            int length = std::max(w,h)/2;
            int cx = (results[max_id_hand].x1+results[max_id_hand].x2)/2;
            int cy = (results[max_id_hand].y1+results[max_id_hand].y2)/2;
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
            gesture = hk.h_gesture(angle_list);

            // std::string text1 = "Gesture: " + gesture;
        }

        if(MODE == 0)
        {
            {
                ScopedTiming st("osd draw", atoi(argv[6]));

                if(gesture == "fist")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                    image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb); 
                }
                else if(gesture == "five")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                    image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb); 
                }
                else if(gesture == "yeah")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                    image_bu_argb.copyTo(copy_ori_image,image_bu_argb);  
                }
            }
        }
        else if (MODE == 1)
        {
            {
                ScopedTiming st("osd draw", atoi(argv[6]));

                if(gesture == "fist")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                    image_bu_argb.copyTo(copy_ori_image,image_bu_argb); 
                }
                else if(gesture == "five")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                    image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb);  
                }
                else if(gesture == "yeah")
                {
                    cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                    image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb); 
                }
            }
        }
        else
        {
            if(sleep_end)
            {
                usleep(2000000);
                sleep_end = false;
            }

            if(max_id_hand == -1)
            {
                set_stop_id = true;
            }

            if(counts_guess == -1 && gesture != "fist" && gesture != "yeah" && gesture != "five")
            {
                std::string start_txt = " G A M E   S T A R T ";
                std::string oneset_txt = std::to_string(1) + "  S E T";
                cv::putText(osd_frame, start_txt, cv::Point(200,500),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 0), 4);
                cv::putText(osd_frame, oneset_txt, cv::Point(400,600),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 150, 255), 4);
            }
            else if(counts_guess == MODE)
            {
                // osd_frame = cv::Mat(osd_height, osd_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
                if(k230_win > player_win)
                {
                    cv::putText(osd_frame, "Y O U   L O S E", cv::Point(340,500),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 0), 4);
                }
                else if(k230_win < player_win)
                {
                    cv::putText(osd_frame, "Y O U   W I N", cv::Point(340,500),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 0), 4);
                }
                else
                {
                    cv::putText(osd_frame, "T I E   G A M E", cv::Point(340,500),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 0, 0), 4);
                }
                counts_guess = -1;
                player_win = 0;
                k230_win = 0;

                sleep_end = true;
            }
            else
            {
                if(set_stop_id)
                {
                    if(counts_guess == -1 && (gesture == "fist" || gesture == "yeah" || gesture == "five"))
                    {
                        counts_guess = 0;
                    }

                    if(counts_guess != -1 && (gesture == "fist" || gesture == "yeah" || gesture == "five"))
                    {
                        int k230_guess=rand()%3;
                        if(gesture == "fist" && LIBRARY[k230_guess] == "yeah")
                        {
                            player_win += 1;
                        }
                        else if(gesture == "fist" && LIBRARY[k230_guess] == "five")
                        {
                            k230_win += 1;
                        }
                        if(gesture == "yeah" && LIBRARY[k230_guess] == "fist")
                        {
                            k230_win += 1;
                        }
                        else if(gesture == "yeah" && LIBRARY[k230_guess] == "five")
                        {
                            player_win += 1;
                        }
                        if(gesture == "five" && LIBRARY[k230_guess] == "fist")
                        {
                            player_win += 1;
                        }
                        else if(gesture == "five" && LIBRARY[k230_guess] == "yeah")
                        {
                            k230_win += 1;
                        }

                        if(LIBRARY[k230_guess] == "fist")
                        {
                            cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,shitou_width,shitou_height));
                            image_shitou_argb.copyTo(copy_ori_image,image_shitou_argb);  
                        }
                        else if(LIBRARY[k230_guess] == "five")
                        {
                            cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,bu_width,bu_height));
                            image_bu_argb.copyTo(copy_ori_image,image_bu_argb);  
                        }
                        else if(LIBRARY[k230_guess] == "yeah")
                        {
                            cv::Mat copy_ori_image = osd_frame(cv::Rect(20,20,jiandao_width,jiandao_height));
                            image_jiandao_argb.copyTo(copy_ori_image,image_jiandao_argb);  
                        }
                        counts_guess += 1;

                        std::string set_txt = std::to_string(counts_guess) + "  S E T";
                        cv::putText(osd_frame, set_txt, cv::Point(400,600),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 150, 255), 4);
                        osd_frame_tmp = osd_frame;
                        set_stop_id = false;
                        sleep_end = true;
                    }
                    else
                    {
                        std::string set_txt = std::to_string(counts_guess + 1) + "  S E T";
                        cv::putText(osd_frame, set_txt, cv::Point(400,600),cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 150, 255), 4);
                    }
                }
                else
                {
                    osd_frame = osd_frame_tmp;
                }
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
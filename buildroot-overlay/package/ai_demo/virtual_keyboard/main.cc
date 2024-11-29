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
#include "key.h"

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
	cout << "Usage: " << name << "<kmodel_det> <obj_thresh> <nms_thresh> <kmodel_kp> <debug_mode>" << endl
		 << "Options:" << endl
		 << "  kmodel_det      手掌检测kmodel路径\n"
         << "  obj_thresh      手掌检测阈值\n"
         << "  nms_thresh      手掌检测非极大值抑制阈值\n"
		 << "  kmodel_kp       手势关键点检测kmodel路径\n"
		 << "  debug_mode      是否需要调试, 0、1、2分别表示不调试、简单调试、详细调试\n"
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

    
    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[5]));
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[5]));

    // create tensors
    std::vector<std::tuple<int, void*>> tensors;
    for (unsigned i = 0; i < BUFFER_NUM; i++) {
        tensors.push_back({context.buffers[i].fd, context.buffers[i].mmap});
    }
    SensorBufManager sensor_buf = SensorBufManager({SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},tensors);

    std::vector<BoxInfo> results;

    std::vector<Key> keys;
    std::string letters = "QWERTYUIOPASDFGHJKLZXCVBNM";

    int character_len = 39;
    int box_w = 140;
    int box_h = 100;
    int startX = 215;
    int startY = 360;
    int margin = 10;
    for (int i = 0; i < letters.size(); i++) {
        if (i < 10) 
        {
            keys.push_back(Key(startX + i * box_w + i * margin, startY, box_w, box_h, std::string(1, letters[i])));
        }
        else if (i<19)
        {
            keys.push_back(Key(startX + (i-10) * box_w + (i-10) * margin + 75, startY+box_h+margin, box_w, box_h, std::string(1, letters[i])));
        }
        else
        {
            keys.push_back(Key(startX + (i-19) * box_w + (i-19) * margin + 150, startY+2*box_h+2*margin, box_w, box_h, std::string(1, letters[i])));
        }
    }

    keys.push_back(Key(startX + (26-19) * box_w + (26-19) * margin + 150, startY+2*box_h+2*margin, box_w, box_h, "clr"));
    keys.push_back(Key(startX + 90, startY+3*box_h+3*margin, 650, box_h, "Space"));
    keys.push_back(Key(startX + 100 + 650, startY+3*box_h+3*margin, 650, box_h, "<--"));
    
    Key textBox(startX, startY-box_h-margin, 10*box_w+9*margin, box_h, " ");

    float previousClick = 0.0;

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

        int signTipX = 0, signTipY = 0;
        int thumbTipX = 0, thumbTipY = 0;

        cv::Point2f index_top;
        cv::Point2f thumb_top;
        float ratio;

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

            {
                ScopedTiming st("osd draw keypoints", atoi(argv[5]));
                hk.draw_keypoints(osd_frame, bbox, false);
            }

            float *pred = hk.get_out()[0];
            int draw_x,draw_y;

            index_top.x = pred[8*2] * w_1 + x1_1;
            index_top.y = pred[8*2+1] * h_1 + y1_1;
            signTipX = static_cast<int>(index_top.x / SENSOR_WIDTH * osd_width);
            signTipY = static_cast<int>(index_top.y / SENSOR_HEIGHT * osd_height);

            thumb_top.x = pred[4*2] * w_1 + x1_1;
            thumb_top.y = pred[4*2+1] * h_1 + y1_1;
            thumbTipX = static_cast<int>(thumb_top.x / SENSOR_WIDTH * osd_width);
            thumbTipY = static_cast<int>(thumb_top.y / SENSOR_HEIGHT * osd_height);

            float dis = calculateIntDidtance(cv::Point(signTipX, signTipY), cv::Point(thumbTipX, thumbTipY));
            float dis_hand = calculateIntDidtance(cv::Point(hk.minX, hk.minY), cv::Point(hk.maxX, hk.maxY));
            ratio = dis/dis_hand;
            if (ratio < 0.25) 
            {
                int centerX = (signTipX + thumbTipX) / 2;
                int centerY = (signTipY + thumbTipY) / 2;
                cv::circle(osd_frame, cv::Point(centerX, centerY), 5, cv::Scalar(0, 255, 0), cv::FILLED);
            }
        }
        {
            ScopedTiming st("osd draw keyboard", atoi(argv[5]));
            float alpha = 0.5;
            textBox.drawKey(osd_frame, (float)0.3, 2);
            for (auto& k : keys) 
            {
                if (k.isOver(thumbTipX, thumbTipY)) 
                {
                    alpha = 0.2;
                    if ((k.isOver(signTipX, signTipY)) && (ratio < 0.25))
                    {
                        float clickTime = cv::getTickCount();
                        if ((clickTime - previousClick) / cv::getTickFrequency() > 0.6) 
                        {
                            if (k.text_ == "<--") 
                            {
                                textBox.text_ = textBox.text_.substr(0, textBox.text_.size() - 1);
                            }
                            else if (k.text_ == "clr") 
                            {
                                textBox.text_ = "";
                            }
                            else if (textBox.text_.size() < character_len) 
                            {
                                if (k.text_ == "Space") {
                                    textBox.text_ += " ";
                                }
                                else {
                                    textBox.text_ += k.text_;
                                }
                            }
                            previousClick = clickTime;
                        }
                    }
                }
                k.drawKey(osd_frame, alpha, 2);
                alpha = 0.5;
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
    if (argc != 6)
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
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

    std::vector<BoxInfo> results;
    std::vector<int> two_point;
    bool first_start = true;
    int two_point_left_x = 0;
    int two_point_top_y = 0;
    int two_point_mean_w = 0;
    int two_point_mean_h = 0;
    int two_point_crop_w = 0;
    int two_point_crop_h = 0;
    int osd_plot_x = 0;
    int osd_plot_y = 0;
    float ori_new_ratio = 0;
    int new_resize_w = 0;
    int new_resize_h = 0;
    int crop_area = 0;
    int rect_frame_x = 0;
    int rect_frame_y = 0;

    int max_new_resize_w = 450;
    int max_new_resize_h = 450;

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
        two_point.clear();
        hd.post_process(results);
        osd_frame = cv::Mat(osd_height, osd_width, CV_8UC3, cv::Scalar(0, 0, 0));

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

        if (max_id_hand != -1)
        {
            std::string text = hd.labels_[results[max_id_hand].label] + ":" + std::to_string(round(results[max_id_hand].score * 100) / 100.0);

            int w = results[max_id_hand].x2 - results[max_id_hand].x1 + 1;
            int h = results[max_id_hand].y2 - results[max_id_hand].y1 + 1;
            
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

            {
                ScopedTiming st("osd draw", atoi(argv[5]));
                hk.draw_keypoints(osd_frame, text, bbox, false, two_point);
            }
        }

        if(max_id_hand != -1)
        {
            if(first_start)
            {
                if(two_point[0] > 0 && two_point[0] < SENSOR_WIDTH && two_point[2] > 0 && two_point[2] < SENSOR_WIDTH && two_point[1] > 0 && two_point[1] < SENSOR_HEIGHT && two_point[3] > 0 && two_point[3] < SENSOR_HEIGHT)
                {
                    two_point_mean_w = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8;
                    two_point_mean_h = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8;
                    first_start = false;
                }
            }
            else
            {
                if(two_point[0] > 0 && two_point[0] < SENSOR_WIDTH && two_point[2] > 0 && two_point[2] < SENSOR_WIDTH && two_point[1] > 0 && two_point[1] < SENSOR_HEIGHT && two_point[3] > 0 && two_point[3] < SENSOR_HEIGHT)
                {
                    two_point_left_x = std::max((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2, 0);
                    two_point_top_y = std::max((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2, 0);
                    two_point_crop_w = std::min(std::min((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2 + two_point_mean_w , two_point_mean_w), SENSOR_WIDTH - ((two_point[0] + two_point[2]) / 2 - two_point_mean_w / 2));
                    two_point_crop_h = std::min(std::min((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2 + two_point_mean_h , two_point_mean_h), SENSOR_HEIGHT - ((two_point[1] + two_point[3]) / 2 - two_point_mean_h / 2));

                    ori_new_ratio = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))*0.8 / two_point_mean_w;

                    new_resize_w = two_point_crop_w * ori_new_ratio / SENSOR_WIDTH * osd_width;
                    new_resize_h = two_point_crop_h * ori_new_ratio / SENSOR_HEIGHT * osd_height;

                    new_resize_w = new_resize_w < max_new_resize_w  ? new_resize_w : max_new_resize_w;
                    new_resize_h = new_resize_h < max_new_resize_h ? new_resize_h : max_new_resize_h;

                    Bbox bbox_crop = {two_point_left_x,two_point_top_y,two_point_crop_w,two_point_crop_h};

                    std::unique_ptr<ai2d_builder> ai2d_builder_crop;
                    dims_t in_shape_crop{1, SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH};
                    dims_t out_shape_crop{1, 3, new_resize_h, new_resize_w};

                    runtime_tensor ai2d_out_tensor_crop = hrt::create(typecode_t::dt_uint8, out_shape_crop, hrt::pool_shared).expect("create ai2d input tensor failed");
                    Utils::crop_resize(bbox_crop, ai2d_builder_crop, img_data, ai2d_out_tensor_crop);

                    auto vaddr_out_buf = ai2d_out_tensor_crop.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
                    unsigned char *output = reinterpret_cast<unsigned char *>(vaddr_out_buf.data());

                    crop_area = new_resize_h*new_resize_w;
                    for(uint32_t hh = 0; hh < new_resize_h; hh++)
                    {
                        for(uint32_t ww = 0; ww < new_resize_w; ww++)
                        {
                            int new_hh = (hh + two_point_top_y * 1.0 / SENSOR_HEIGHT * osd_height);
                            int new_ww = (ww + two_point_left_x * 1.0 / SENSOR_WIDTH * osd_width);
                            int osd_channel_index = (new_hh * osd_width + new_ww) * 3;
                            if(osd_frame.data[osd_channel_index + 0] == 0)                        
                            {
                                int ori_pix_index = hh * new_resize_w + ww;
                                osd_frame.data[osd_channel_index] =  output[ori_pix_index];
                                osd_frame.data[osd_channel_index + 1] =  output[ori_pix_index + crop_area];
                                osd_frame.data[osd_channel_index + 2] =  output[ori_pix_index + crop_area * 2]; 
                            }                        
                        }
                    }

                    rect_frame_x = two_point_left_x * 1.0 / SENSOR_WIDTH * osd_width;
                    rect_frame_y = two_point_top_y * 1.0 / SENSOR_HEIGHT * osd_height;
                    cv::rectangle(osd_frame, cv::Rect(rect_frame_x, rect_frame_y , new_resize_w, new_resize_h), cv::Scalar(0, 255, 255), 2, 2, 0);
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
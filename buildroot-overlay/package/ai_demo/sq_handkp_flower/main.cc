/* Copyright (c) 2025, Canaan Bright Sight Co., Ltd
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
#include "setting.h"
#include "sensor_buf_manager.h"
#include "hand_detection.h"
#include "hand_keypoint.h"
#include "flower_recognition.h"
#include "sort.h"

using std::cerr;
using std::cout;
using std::endl;
using std::thread;

static std::mutex result_mutex;
static vector<BoxInfo> hand_results;
static vector<Sort::TrackingBox> frameTrackingResult;
static vector<HandKeyPointInfo> hand_keypoint_results;
static vector<BoxInfo> flower_recognition_results;
std::atomic<bool> ai_stop(false);
// 显示线程退出标志
std::atomic<bool> display_stop(false);
static volatile unsigned kpu_frame_count = 0;
static struct timeval tv, tv2;
// 显示实例和OSD缓冲区
static struct display* display;
struct display_buffer* draw_buffer;

void print_usage(const char *name)
{
	cout << "Usage: " << name << "<kmodel_det> <input_mode> <obj_thresh> <nms_thresh> <kmodel_kp> <flower_rec> <debug_mode>" << endl
		 << "Options:" << endl
		 << "  kmodel_det      手掌检测kmodel路径\n"
		 << "  input_mode      本地图片(图片路径)/ 摄像头(None) \n"
         << "  obj_thresh      手掌检测kmodel obj阈值\n"
         << "  nms_thresh      手掌检测kmodel nms阈值\n"
		 << "  kmodel_kp       手势关键点检测kmodel路径\n"
         << "  flower_rec      花卉识别kmodel路径 \n"
		 << "  debug_mode      是否需要调试, 0、1、2分别表示不调试、简单调试、详细调试\n"
		 << "\n"
		 << endl;
}

// zero copy, use less memory
static void ai_proc(char *argv[], int video_device) {
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

    HandDetection hd(argv[1], atof(argv[3]), atof(argv[4]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[7]));
    HandKeypoint hk(argv[5], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[7]));
    FlowerRecognition fr(argv[6], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[7]));

    Sort sort;
    int fi = 0;

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
        hand_results.clear();
        hd.post_process(hand_results);

        frameTrackingResult = sort.Sortx(hand_results, fi);
        fi ++;

        hand_keypoint_results = std::vector<HandKeyPointInfo>{};
        flower_recognition_results = std::vector<BoxInfo>{};

        if (frameTrackingResult.size()>=2 && fi>=5)
        {
            cv::Point2f left_top, right_bottom;
            int left_pred_x, left_pred_y;
            int right_pred_x, right_pred_y;
            int index1 = 1;

            for(int i=0;i< frameTrackingResult.size();i++)
            {
                auto tb = frameTrackingResult[i];
                std::string num = std::to_string(tb.id);

                int length = std::max(tb.box.width,tb.box.height)/2;
                int cx = tb.box.x+tb.box.width/2;
                int cy = tb.box.y+tb.box.height/2;
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

                HandKeyPointInfo post_process_result;
                hk.post_process(bbox, post_process_result);
                hand_keypoint_results.push_back(post_process_result);
                float* pred = post_process_result.pred.data();

                if (i==0)
                {
                    left_pred_x = std::max(std::min(pred[(index1+1)*4*2], float(SENSOR_WIDTH)), 0.0f);
                    left_pred_y = std::max(std::min(pred[(index1+1)*4*2+1], float(SENSOR_HEIGHT)), 0.0f);
                }
                else if (i==1)
                {
                    right_pred_x = std::max(std::min(pred[(index1+1)*4*2], float(SENSOR_WIDTH)), 0.0f);
                    right_pred_y = std::max(std::min(pred[(index1+1)*4*2+1], float(SENSOR_HEIGHT)), 0.0f);
                }

                
            }
            {
                int x_min = std::min(left_pred_x, right_pred_x);
                int x_max = std::max(left_pred_x, right_pred_x);
                int y_min = std::min(left_pred_y, right_pred_y);
                int y_max = std::max(left_pred_y, right_pred_y);
                Bbox box_info = {x_min,y_min, (x_max-x_min+1),(y_max-y_min+1)};

                fr.pre_process(img_data, box_info);
                fr.inference();
                std::string text = fr.post_process(); 

                BoxInfo flower_result;
                flower_result.x1 = left_pred_x;
                flower_result.y1 = left_pred_y;
                flower_result.x2 = right_pred_x;
                flower_result.y2 = right_pred_y;
                flower_result.label = text;
                
                flower_recognition_results.push_back(flower_result);
            }
        }

        result_mutex.unlock();
        kpu_frame_count += 1;
        v4l2_drm_dump_release(&context);
    }
    v4l2_drm_stop(&context);
}

/**
 * @brief V4L2-DRM 显示帧处理函数（每帧触发一次）
 *
 * 该函数由 v4l2_drm_run 驱动循环回调，在每一帧显示数据时被调用。主要功能用于显示AI推理的结果。
 * @param context V4L2-DRM 上下文结构体指针
 * @param displayed 表示该帧是否已经被实际显示
 * @return 返回 0 表示正常，返回 'q' 表示请求退出主循环（受控于 display_stop 标志）
 */
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
                if (draw_buffer->width > draw_buffer->height)
                {
                    // 创建临时 BGRA 显示缓冲Mat（用于画图）
                    cv::Mat temp_img(draw_buffer->height, draw_buffer->width, CV_8UC4);
                    // 横屏
                    temp_img.setTo(cv::Scalar(0, 0, 0, 0));
                    result_mutex.lock();
                    if (frameTrackingResult.size()==2 && hand_keypoint_results.size()==2 && flower_recognition_results.size()==1)
                    {
                        FlowerRecognition::draw_flower_results(temp_img, frameTrackingResult, hand_keypoint_results, flower_recognition_results);
                    }

                    result_mutex.unlock();
                    //---------------------- 显示缓冲同步 ----------------------
                    // 将绘图图像复制到实际显示缓冲区
                    memcpy(draw_buffer->map, temp_img.data, draw_buffer->size);
                }
                else
                {
                    // 创建临时 BGRA 显示缓冲Mat（用于画图）
                    cv::Mat temp_img(draw_buffer->width, draw_buffer->height, CV_8UC4);
                    // 横屏
                    temp_img.setTo(cv::Scalar(0, 0, 0, 0));
                    result_mutex.lock();
                    if (frameTrackingResult.size()==2 && hand_keypoint_results.size()==2 && flower_recognition_results.size()==1)
                    {
                        FlowerRecognition::draw_flower_results(temp_img, frameTrackingResult, hand_keypoint_results, flower_recognition_results);
                    }
                    result_mutex.unlock();
                    // 旋转回屏幕方向
                    cv::rotate(temp_img, temp_img, cv::ROTATE_90_CLOCKWISE);
                    //---------------------- 显示缓冲同步 ----------------------
                    // 将绘图图像复制到实际显示缓冲区
                    memcpy(draw_buffer->map, temp_img.data, draw_buffer->size);
                }
                last_drawed_buffer = buffer;
                // flush cache
                thead_csi_dcache_clean_invalid_range(buffer->map, buffer->size);
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

    // 若收到退出信号，返回 'q' 表示主循环退出
    if (display_stop) {
        return 'q';
    }
    return 0;
}

/**
 * @brief 显示线程主函数，初始化 V4L2-DRM 并绑定绘制回调
 *
 * 根据屏幕方向（横屏 / 竖屏）配置对应的宽高、格式和旋转角度，
 * 然后调用 `v4l2_drm_run()` 启动帧处理主循环，由 `frame_handler()` 每帧触发绘制。
 *
 * @param video_device 视频设备编号（如 /dev/video0 中的 1）
 */
void display_proc(int video_device) 
{
    struct v4l2_drm_context context;
    v4l2_drm_default_context(&context);
    context.device = video_device;
    // 根据屏幕方向设置 width/height/rotation
    if (display->width > display->height)
    {
        // 横屏
        context.width = display->width;
        context.height = (display->width * SENSOR_HEIGHT / SENSOR_WIDTH) & 0xfff8;
        context.video_format = V4L2_PIX_FMT_NV12;
        context.display_format = 0;
        context.drm_rotation = rotation_0;
    }
    else 
    {
        // 竖屏
        context.width = display->height;
        context.height = display->width;
        context.video_format = V4L2_PIX_FMT_NV12;
        context.display_format = 0;
        context.drm_rotation = rotation_90;
    }

    // 初始化 V4L2 + DRM 流
    if (v4l2_drm_setup(&context, 1, &display)) {
        std::cerr << "v4l2_drm_setup error" << std::endl;
        return;
    }

    // 分配OSD显示 plane 和 buffer
    struct display_plane* plane = display_get_plane(display, DRM_FORMAT_ARGB8888);
    draw_buffer = display_allocate_buffer(plane, display->width, display->height);
    display_commit_buffer(draw_buffer, 0, 0);

    gettimeofday(&tv, NULL);
    v4l2_drm_run(&context, 1, frame_handler);
    // 清理资源
    if (display) {
        display_free_plane(plane);
        display_exit(display);
    }
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

    if (strcmp(argv[2], "None") == 0)
    {
        display = display_init(0);
        if (!display) {
            cerr << "display_init error, exit" << endl;
            return -1;
        }

        // 锁住结果互斥量，等待首次帧到来后解锁
        result_mutex.lock();

        // 启动分类任务推理线程
        std::thread ai_thread(ai_proc, argv, 2);
        // 启动显示线程（处理显示内容绘制）
        std::thread display_thread(display_proc, 1);

        // 输入提示信息
        std::cout << "输入 'q'回车退出" << std::endl;

        // 命令行输入处理主循环
        std::string last_input = "";
        while (true) {
            std::string input;
            std::getline(std::cin, input);  // 获取用户输入
            if (input == "q") {
                // 退出程序
                display_stop.store(true); // 通知显示线程退出
                usleep(100000);           // 稍作延迟，确保帧处理完成
                ai_stop.store(true);      // 通知人脸线程退出
                break;
            }
            else{
                usleep(100000);
            }
        }

        // 等待两个线程完成后退出程序
        display_thread.join();
        ai_thread.join();
    }
    else
    {
        cv::Mat ori_img = cv::imread(argv[2]);
        int origin_w = ori_img.cols;
        int origin_h = ori_img.rows;
        FrameSize handimg_size = {origin_w, origin_h};

        HandDetection hd(argv[1], atof(argv[3]),atof(argv[4]), handimg_size, atoi(argv[7]));
        HandKeypoint hk(argv[5], atoi(argv[7]));
        FlowerRecognition fr(argv[6], atoi(argv[7]));

        hd.pre_process(ori_img);
        hd.inference();
        hd.post_process(hand_results);

        int index1 = 1;
        if (hand_results.size()>=2)
        {
            cv::Point2f left_top, right_bottom;
            for(int i=0;i< hand_results.size();i++)
            {
                BoxInfo r = hand_results[i];

                int w = r.x2 - r.x1 + 1;
                int h = r.y2 - r.y1 + 1;
                cv::rectangle(ori_img, cv::Rect(static_cast<int>(r.x1), static_cast<int>(r.y1) , w, h), cv::Scalar(255, 255, 255), 2, 2, 0);
                
                int length = std::max(w,h)/2;
                int cx = (r.x1+r.x2)/2;
                int cy = (r.y1+r.y2)/2;
                int ratio_num = 1.26*length;

                int x1_1 = std::max(0,cx-ratio_num);
                int y1_1 = std::max(0,cy-ratio_num);
                int x2_1 = std::min(origin_w-1, cx+ratio_num);
                int y2_1 = std::min(origin_h-1, cy+ratio_num);
                int w_1 = x2_1 - x1_1 + 1;
                int h_1 = y2_1 - y1_1 + 1;
                
                struct Bbox bbox = {x:x1_1,y:y1_1,w:w_1,h:h_1};
                hk.pre_process(ori_img, bbox);
                hk.inference();

                HandKeyPointInfo post_process_result;
                hk.post_process(bbox, post_process_result);
                hk.draw_keypoints(ori_img, post_process_result, true);

                if (i==0)
                {
                    left_top.x = post_process_result.pred[(index1+1)*4*2];
                    left_top.y = post_process_result.pred[(index1+1)*4*2+1];

                    cv::circle(ori_img, left_top, 6, cv::Scalar(0,0,0), 3);
                    cv::circle(ori_img, left_top, 5, cv::Scalar(0,0,0), 3);
                }
                if (i==1)
                {
                    right_bottom.x = post_process_result.pred[(index1+1)*4*2];
                    right_bottom.y = post_process_result.pred[(index1+1)*4*2+1];

                    cv::circle(ori_img, right_bottom, 6, cv::Scalar(0,0,0), 3);
                    cv::circle(ori_img, right_bottom, 5, cv::Scalar(0,0,0), 3);
                }

            }
            {
                int x_min = std::min(left_top.x, right_bottom.x);
                int x_max = std::max(left_top.x, right_bottom.x);
                int y_min = std::min(left_top.y, right_bottom.y);
                int y_max = std::max(left_top.y, right_bottom.y);
                Bbox box_info = {x_min,y_min, (x_max-x_min+1),(y_max-y_min+1)};

                fr.pre_process(ori_img, box_info);

                fr.inference();

                std::string text = fr.post_process();
                cv::putText(ori_img, text, cv::Point(x_min,y_min-20),cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 195, 0), 2);

                cv::rectangle(ori_img, cv::Rect(x_min, y_min , (x_max-x_min), (y_max-y_min)), cv::Scalar(255, 255, 0), 2, 2, 0);
            }
        }
        cv::imwrite("hand_flower_result.jpg", ori_img);
    }
    return 0;
}
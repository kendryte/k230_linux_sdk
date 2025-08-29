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
#include "setting.h"
#include "licence_det.h"
#include "licence_reco.h"
#include "sensor_buf_manager.h"
#include "text_paint.h"

using std::cerr;
using std::cout;
using std::endl;
using std::thread;

static std::mutex result_mutex;
static vector<BoxPoint> licence_det_results;
static vector<string> licence_rec_results;
static vector<vector<cv::Point2f>> licence_rec_sort_vtd;
std::atomic<bool> ai_stop(false);
// 显示线程退出标志
std::atomic<bool> display_stop(false);
static volatile unsigned kpu_frame_count = 0;
static struct timeval tv, tv2;
// 显示实例和OSD缓冲区
static struct display* display;
struct display_buffer* draw_buffer;

const int g_dict_size = 74;

TextRenderer writepen("SourceHanSansSC-Normal-Min.ttf",25);

void print_usage(const char *name)
{
    cout << "Usage: " << name << "<kmodel_det> <obj_thresh> <nms_thresh> <input_mode> <kmodel_reco> <debug_mode>" << endl
         << "Options:" << endl
         << "  kmodel_det      车牌检测 kmodel路径\n"
         << "  obj_thresh      车牌检测 分数阈值\n"
         << "  nms_thresh      车牌检测 非极大值抑制阈值\n"
         << "  input_mode      本地图片(图片路径)/ 摄像头(None) \n"
         << "  kmodel_reco     车牌识别kmodel路径 \n"
         << "  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
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

    LicenceDetect licenceDet(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[6]));
    LicenceReco licenceReco(argv[5],g_dict_size,atoi(argv[6]));

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
        licenceDet.pre_process(img_data);
        licenceDet.inference();
        result_mutex.lock();
        licence_det_results.clear();
        licenceDet.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, licence_det_results);

        int matsize = SENSOR_WIDTH * SENSOR_HEIGHT;
        cv::Mat ori_img;
        auto img_data_buf = img_data.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
        cv::Mat ori_img_R = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, img_data_buf.data());
        cv::Mat ori_img_G = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, img_data_buf.data() + 1 * matsize);
        cv::Mat ori_img_B = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, img_data_buf.data() + 2 * matsize);
        std::vector<cv::Mat> sensor_rgb;
        sensor_rgb.push_back(ori_img_B);
        sensor_rgb.push_back(ori_img_G);
        sensor_rgb.push_back(ori_img_R);
        cv::merge(sensor_rgb, ori_img);

        licence_rec_results.clear();
        licence_rec_sort_vtd.clear();
        for(int i = 0; i < licence_det_results.size(); i++)
        {
            vector<Point> ori_vec;
            vector<cv::Point2f> sort_vtd(4);
            ori_vec.clear();
            for(int j = 0; j < 4; j++)
            {
                ori_vec.push_back(licence_det_results[i].vertices[j]);
                // std::cout << licence_det_results[i].vertices[j].x << " " << licence_det_results[i].vertices[j].y << std::endl;
            }
            // std::cout << "=========" << std::endl;
            cv::RotatedRect rect = cv::minAreaRect(ori_vec);
            cv::Point2f ver[4];
            rect.points(ver);

            cv::Mat crop;
            licenceReco.warppersp(ori_img, crop, licence_det_results[i], sort_vtd);
            cv::Mat crop_gray;
            cv::cvtColor(crop, crop_gray, COLOR_BGR2GRAY);


            licenceReco.pre_process(crop_gray);
            licenceReco.inference();

            string rec_result="";
            licenceReco.post_process(rec_result);
            licence_rec_results.push_back(rec_result);
            licence_rec_sort_vtd.push_back(sort_vtd);
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
                    for(int i = 0; i < licence_rec_results.size(); i++)
                    {
                        for(int j=0;j<4;j++){

                            int x1=int(licence_rec_sort_vtd[i][j%4].x*temp_img.cols/SENSOR_WIDTH);
                            int y1=int(licence_rec_sort_vtd[i][j%4].y*temp_img.rows/SENSOR_HEIGHT);
                            int x2=int(licence_rec_sort_vtd[i][(j+1)%4].x*temp_img.cols/SENSOR_WIDTH);
                            int y2=int(licence_rec_sort_vtd[i][(j+1)%4].y*temp_img.rows/SENSOR_HEIGHT);
                            cv::line(temp_img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0,255), 2);
                        }
                        LicenceReco::draw_text(float(licence_rec_sort_vtd[i][0].x),float(licence_rec_sort_vtd[i][0].y),temp_img,licence_rec_results[i],writepen);
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
                    for(int i = 0; i < licence_rec_results.size(); i++)
                    {
                        for(int j=0;j<4;j++){
                            int x1=int(licence_rec_sort_vtd[i][j%4].x*temp_img.cols/SENSOR_WIDTH);
                            int y1=int(licence_rec_sort_vtd[i][j%4].y*temp_img.rows/SENSOR_HEIGHT);
                            int x2=int(licence_rec_sort_vtd[i][(j+1)%4].x*temp_img.cols/SENSOR_WIDTH);
                            int y2=int(licence_rec_sort_vtd[i][(j+1)%4].y*temp_img.rows/SENSOR_HEIGHT);
                            cv::line(temp_img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0,255), 2);
                        }
                        LicenceReco::draw_text(float(licence_rec_sort_vtd[i][0].x),float(licence_rec_sort_vtd[i][0].y),temp_img,licence_rec_results[i],writepen);
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
    if (argc != 7)
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
        cv::Mat ori_img = cv::imread(argv[4]);
        cv::Mat draw_img = ori_img.clone();
        int ori_w = ori_img.cols;
        int ori_h = ori_img.rows;

        LicenceDetect licenceDet(argv[1], atof(argv[2]), atof(argv[3]), atoi(argv[6]));
        LicenceReco licenceReco(argv[5],g_dict_size,atoi(argv[6]));

        licenceDet.pre_process(ori_img);

        licenceDet.inference();

        vector<BoxPoint> results_det;
        licenceDet.post_process({ori_w, ori_h}, results_det);

        for(int i = 0; i < results_det.size(); i++)
        {
            vector<Point> ori_vec;
            vector<cv::Point2f> sort_vtd(4);
            ori_vec.clear();
            for(int j = 0; j < 4; j++)
            {
                ori_vec.push_back(results_det[i].vertices[j]);
            }
            cv::RotatedRect rect = cv::minAreaRect(ori_vec);
            cv::Point2f ver[4];
            rect.points(ver);
            cv::Mat crop;
            licenceReco.warppersp(ori_img, crop, results_det[i], sort_vtd);
            cv::Mat crop_gray;
            cv::cvtColor(crop, crop_gray, cv::COLOR_BGR2GRAY);

            licenceReco.pre_process(crop_gray);
            licenceReco.inference();

            string rec_result="";
            licenceReco.post_process(rec_result);
            {
                ScopedTiming st("draw_text", atoi(argv[6]));
                for(int j=0;j<4;j++){
                    int x1=int(sort_vtd[j%4].x*draw_img.cols/ori_w);
                    int y1=int(sort_vtd[j%4].y*draw_img.rows/ori_h);
                    int x2=int(sort_vtd[(j+1)%4].x*draw_img.cols/ori_w);
                    int y2=int(sort_vtd[(j+1)%4].y*draw_img.rows/ori_h);
                    cv::line(draw_img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0), 2);
                    if(j==0){
                        licenceReco.draw_text(x1, y1-10,draw_img,rec_result,writepen);
                    }
                }
                
            } 
        }

        {
            ScopedTiming st("draw_detections", atoi(argv[6]));
            licenceDet.draw_detections(draw_img, results_det);
        }
        cv::imwrite("licence_det_rec.jpg", draw_img);
    }
    return 0;
}

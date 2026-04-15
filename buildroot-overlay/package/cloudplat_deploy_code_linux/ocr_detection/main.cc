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
#include "ai_utils.h"
#include "sensor_set.h"
#include "parse_args.h"
#include "sensor_buf_manager.h"
#include "ocr_box.h"
#include "mmz.h"

using std::cerr;
using std::cout;
using std::endl;
using std::thread;

// 帧率统计相关
static bool fps_count=false;
static volatile unsigned kpu_frame_count = 0;
static struct timeval tv, tv2;
// 显示实例和OSD缓冲区
static struct display* display;
struct display_buffer* draw_buffer;

// 线程互斥锁，防止后处理和结果绘制访问冲突
static std::mutex result_mutex;
// OCR检测结果
static vector<ocr_det_res> results;
// AI线程退出标志
std::atomic<bool> ai_stop(false);
// 显示线程退出标志
std::atomic<bool> display_stop(false);

void print_usage(const char *name)
{
    cout << "Usage: " << name << "<config_path> <image_path/None> <debug_mode>" << endl
         << "Options:" << endl
         << "  config_path     部署配置文件deploy_config.json路径\n"
         << "  image_path/None 推理图片路径，当使用视频推理时设置为None"
         << "  debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试\n"
         << "\n"
         << endl;
}

void ocrdet_proc(char *argv[], int video_device) {
    struct v4l2_drm_context context;
    struct v4l2_drm_video_buffer buffer;

    // wait display_proc running
    result_mutex.lock();
    result_mutex.unlock();

    v4l2_drm_default_context(&context);
    context.device = video_device;
    context.display = false;
    context.width = SENSOR_WIDTH;
    context.height = SENSOR_HEIGHT;
    context.video_format = v4l2_fourcc('B', 'G', '3', 'P');
    context.buffer_num = 3;

    if (v4l2_drm_setup(&context, 1, NULL)) {
        cerr << "v4l2_drm_setup error" << endl;
        return;
    }
    if (v4l2_drm_start(&context)) {
        cerr << "v4l2_drm_start error" << endl;
        return;
    }
    // 参数解析
    std::string config_path(argv[1]);
    int debug_mode = atoi(argv[3]);
    config_args args;
    parse_args(config_path,args,debug_mode);
    OCRBox ocr_det(args, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},debug_mode);

    // create tensors
    std::vector<std::tuple<int, void*>> tensors;
    for (unsigned i = 0; i < 3; i++) {
        tensors.push_back({context.buffers[i].fd, context.buffers[i].mmap});
    }
    SensorBufManager sensor_buf = SensorBufManager({SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},tensors);
    
    while (!ai_stop) {
        int ret = v4l2_drm_dump(&context, 1000);
        if (ret) {
            perror("v4l2_drm_dump error");
            continue;
        }
        //---------------------------------- OCR检测推理 ----------------------------------
        {
            ScopedTiming st("Classification total time", debug_mode);
            runtime_tensor input_tensor = sensor_buf.get_buf_for_index(context.vbuffer.index);
            ocr_det.pre_process(sensor_buf.get_buf_for_index(context.vbuffer.index));
            ocr_det.inference();
            results.clear();
            std::lock_guard<std::mutex> res_lock(result_mutex);
            ocr_det.post_process(results);
        }        
        //---------------------------------- OCR检测推理 ----------------------------------
        kpu_frame_count += 1;
        v4l2_drm_dump_release(&context);
    }
    v4l2_drm_stop(&context);
}

/**
 * @brief V4L2-DRM 显示帧处理函数（每帧触发一次）
 *
 * 该函数由 v4l2_drm_run 驱动循环回调，在每一帧显示数据时被调用。主要功能如下：
 * - 初始阶段：通知人脸检测线程开始运行（解锁互斥锁）。
 * - 检查当前帧是否来自新缓冲区，如是，则进行显示绘制；
 * - 根据不同状态（如识别、注册、调试），在显示缓冲区上绘制对应图像或文字；
 * - 将绘制图像复制到实际 DRM 显示缓冲区；
 * - 支持人脸框绘制；
 * - 可选择启用 FPS 打印功能（已注释）；
 *
 * @param context V4L2-DRM 上下文结构体指针
 * @param displayed 表示该帧是否已经被实际显示
 * @return 返回 0 表示正常，返回 'q' 表示请求退出主循环（受控于 display_stop 标志）
 */
int frame_handler(struct v4l2_drm_context *context, bool displayed)
{
    static bool first_frame = true;
    if (first_frame) {
        // 第一帧解锁互斥量，允许 AI 处理线程继续执行
        result_mutex.unlock();
        first_frame = false;
    }

    static unsigned response = 0, display_frame_count = 0;
    response += 1;

    if (displayed) 
    {
        // 判断该帧的 buffer 是否被标记为持有可绘制数据
        if (context[0].buffer_hold[context[0].wp] >= 0) 
        {
            static struct display_buffer* last_drawed_buffer = nullptr;
            auto buffer = context[0].display_buffers[context[0].buffer_hold[context[0].wp]];

            // 如果是新的一帧（不是重复帧），才进行绘制
            if (buffer != last_drawed_buffer) {
                //---------------------- 绘制显示结果 ----------------------
                if (draw_buffer->width > draw_buffer->height)
                {
                    // 创建临时 ARGB 显示缓冲（用于画图）
                    cv::Mat temp_img(draw_buffer->height, draw_buffer->width, CV_8UC4);
                    // 横屏：显示人脸检测框
                    temp_img.setTo(cv::Scalar(0, 0, 0, 0));
                    std::lock_guard<std::mutex> lock(result_mutex);
                    OCRBox::draw_result(temp_img,results,{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH});
                    //---------------------- 显示缓冲同步 ----------------------
                    // 将绘图图像复制到实际显示缓冲区
                    memcpy(draw_buffer->map, temp_img.data, draw_buffer->size);
                }
                else
                {
                    // 创建临时 ARGB 显示缓冲（用于画图）
                    cv::Mat temp_img(draw_buffer->width, draw_buffer->height, CV_8UC4);
                    // 竖屏st7701：横图绘制，然后转回竖图给display显示
                
                    temp_img.setTo(cv::Scalar(0, 0, 0, 0));
                    std::lock_guard<std::mutex> res_lock(result_mutex);
                    OCRBox::draw_result(temp_img,results,{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH});

                    // 旋转回屏幕方向
                    cv::rotate(temp_img, temp_img, cv::ROTATE_90_CLOCKWISE);
                    //---------------------- 显示缓冲同步 ----------------------
                    // 将绘图图像复制到实际显示缓冲区
                    memcpy(draw_buffer->map, temp_img.data, draw_buffer->size);
                }
                last_drawed_buffer = buffer;

                // 刷新缓存，通知显示设备更新
                // thead_csi_dcache_clean_invalid_range(buffer->map, buffer->size);
                thead_csi_dcache_clean_invalid_range(draw_buffer->map, draw_buffer->size);

                display_update_buffer(draw_buffer, 0, 0);
            }
        }
        display_frame_count += 1;
    }

    // FPS counter
    if(fps_count){
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

    if(fps_count){
        // 记录起始时间（用于 FPS 测试）
        gettimeofday(&tv, NULL);
    }

    // // 启动显示主循环，绑定回调 frame_handler
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


/**
 * @brief 主程序入口
 * 
 * 初始化显示、启动 AI 和显示线程，实现OCR检测。
 */
int main(int argc, char *argv[])
{
    // 输出编译信息
    std::cout << "程序名：" << argv[0] << " | 构建时间：" << __DATE__ << " " << __TIME__ << std::endl;

    // 检查参数数量是否合法（此处要求 9 个参数）
    if (argc != 4)
    {
        print_usage(argv[0]); // 打印用法说明
        return -1;
    }
    if (strcmp(argv[2], "None") == 0)
    {
        // 初始化显示模块
        display = display_init(0);
        if (!display) {
            std::cerr << "显示初始化失败，程序退出！" << std::endl;
            return -1;
        }

        // 锁住结果互斥量，等待首次帧到来后解锁
        result_mutex.lock();

        // 启动分类任务推理线程
        std::thread ai_thread(ocrdet_proc, argv, 2);
        // 启动显示线程（处理显示内容绘制）
        std::thread display_thread(display_proc, kd_mpi_get_vvcam_video00());

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
    else{
        // 参数解析
        std::string config_path(argv[1]);
        std::string image_path(argv[2]);
        int debug_mode = atoi(argv[3]);
        // deploy_config.json参数解析
        config_args args;
        parse_args(config_path,args,debug_mode);
        // 读取图片
        cv::Mat ori_img = cv::imread(image_path);
        FrameCHWSize image_size={ori_img.channels(),ori_img.rows,ori_img.cols};
        // 创建一个空的向量，用于存储chw图像数据,将读入的hwc数据转换成chw数据
        std::vector<uint8_t> chw_vec;
        std::vector<cv::Mat> bgrChannels(3);
        cv::split(ori_img, bgrChannels);
        for (auto i = 2; i > -1; i--)
        {
            std::vector<uint8_t> data = std::vector<uint8_t>(bgrChannels[i].reshape(1, 1));
            chw_vec.insert(chw_vec.end(), data.begin(), data.end());
        }
        // 创建tensor
        dims_t in_shape { 1, 3, ori_img.rows, ori_img.cols };
        runtime_tensor input_tensor = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("cannot create input tensor");
        auto input_buf = input_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
        memcpy(reinterpret_cast<char *>(input_buf.data()), chw_vec.data(), chw_vec.size());
        hrt::sync(input_tensor, sync_op_t::sync_write_back, true).expect("write back input failed");

        OCRBox ocr_det(args, image_size,debug_mode);
        ocr_det.pre_process(input_tensor);
        ocr_det.inference();
        ocr_det.post_process(results);
        ocr_det.draw_result(ori_img,results,image_size);
        cv::imwrite("ocrdet_result.jpg",ori_img);
    }
    return 0;
}

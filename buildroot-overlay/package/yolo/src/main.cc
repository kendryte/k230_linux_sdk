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
#include "sensor_set.h"
#include "sensor_buf_manager.h"
#include "yolo11.h"
#include "yolov8.h"
#include "yolov5.h"
#include "yolo26.h"

using std::cerr;
using std::cout;
using std::endl;
using std::thread;

static std::mutex result_mutex;
// 检测结果
static std::vector<YOLOBbox> yolo_results;
std::atomic<bool> ai_stop(false);
// 显示线程退出标志
std::atomic<bool> display_stop(false);
static volatile unsigned kpu_frame_count = 0;
static struct timeval tv, tv2;
// 显示实例和OSD缓冲区
static struct display* display;
struct display_buffer* draw_buffer;
cv::Mat draw_frame;

// zero copy, use less memory
static void ai_proc(YoloConfig &yolo_config,int video_device) {
    struct v4l2_drm_context context;
    struct v4l2_drm_video_buffer buffer;
    // wait display_proc running
    result_mutex.lock();
    result_mutex.unlock();
    int buffer_num=5;

    v4l2_drm_default_context(&context);
    context.device = video_device;
    context.display = false;
    context.width = SENSOR_WIDTH;
    context.height = SENSOR_HEIGHT;
    context.video_format = v4l2_fourcc('B', 'G', '3', 'P');
    context.buffer_num = buffer_num;
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
    for (unsigned i = 0; i < buffer_num; i++) {
        tensors.push_back({context.buffers[i].fd, context.buffers[i].mmap});
    }
    SensorBufManager sensor_buf = SensorBufManager({SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},tensors);

    FrameSize image_wh={SENSOR_WIDTH,SENSOR_HEIGHT};
    // 从标签文件中读取标签
    std::vector<std::string> labels=readLabelsFromTxt(yolo_config.labels_txt_filepath);

    if(strcmp(yolo_config.model_type, "yolo26") == 0){
        // 创建一个Yolo26对象，用于执行yolo26的推理流程
        Yolo26 yolo26(yolo_config.task_type,yolo_config.task_mode,yolo_config.kmodel_path,yolo_config.conf_thres,yolo_config.mask_thres,labels,image_wh,yolo_config.kp_num,yolo_config.kp_dim,yolo_config.debug_mode);
        // 循环执行推理流程，直到ai_stop为true
        while(!ai_stop){
            int ret = v4l2_drm_dump(&context, 1000);
            if (ret) {
                perror("v4l2_drm_dump error");
                continue;
            }
            runtime_tensor input_tensor= sensor_buf.get_buf_for_index(context.vbuffer.index);   
            // 执行预处理
            yolo26.pre_process(input_tensor);
            // 执行推理
            yolo26.inference();
            // 执行后处理
            result_mutex.lock();
            yolo26.post_process(yolo_results);
            // 将绘制的帧设置为黑色
            draw_frame.setTo(cv::Scalar(0, 0, 0, 0));
            // 在绘制的帧上绘制检测结果
            yolo26.draw_results(draw_frame,yolo_results);
            result_mutex.unlock();
            kpu_frame_count += 1;
            v4l2_drm_dump_release(&context);
        }
    }
    // 如果模型类型为yolo11，则执行yolo11的推理流程
    else if(strcmp(yolo_config.model_type, "yolo11") == 0){
        // 创建一个Yolo11对象，用于执行yolo11的推理流程
        Yolo11 yolo11(yolo_config.task_type,yolo_config.task_mode,yolo_config.kmodel_path,yolo_config.conf_thres,yolo_config.nms_thres,yolo_config.mask_thres,labels,image_wh,yolo_config.kp_num,yolo_config.kp_dim,yolo_config.debug_mode);
        while (!ai_stop) {
            int ret = v4l2_drm_dump(&context, 1000);
            if (ret) {
                perror("v4l2_drm_dump error");
                continue;
            }
            runtime_tensor input_tensor= sensor_buf.get_buf_for_index(context.vbuffer.index);        
            // 执行预处理
            yolo11.pre_process(input_tensor);
            // 执行推理
            yolo11.inference();
            result_mutex.lock();
            // 执行后处理
            yolo11.post_process(yolo_results);
            // 将绘制的帧设置为黑色
            draw_frame.setTo(cv::Scalar(0, 0, 0, 0));
            // 在绘制的帧上绘制检测结果
            yolo11.draw_results(draw_frame,yolo_results);
            result_mutex.unlock();
            kpu_frame_count += 1;
            v4l2_drm_dump_release(&context);
        }
    }
    // 如果模型类型为yolov8，则执行yolov8的推理流程
    else if(strcmp(yolo_config.model_type, "yolov8") == 0){
        // 创建一个Yolov8对象，用于执行yolov8的推理流程
        Yolov8 yolov8(yolo_config.task_type,yolo_config.task_mode,yolo_config.kmodel_path,yolo_config.conf_thres,yolo_config.nms_thres,yolo_config.mask_thres,labels,image_wh,yolo_config.kp_num,yolo_config.kp_dim,yolo_config.debug_mode);
        
        while (!ai_stop) {
            int ret = v4l2_drm_dump(&context, 1000);
            if (ret) {
                perror("v4l2_drm_dump error");
                continue;
            }
            runtime_tensor input_tensor= sensor_buf.get_buf_for_index(context.vbuffer.index);        
            // 执行预处理
            yolov8.pre_process(input_tensor);
            // 执行推理
            yolov8.inference();
            result_mutex.lock();
            // 执行后处理
            yolov8.post_process(yolo_results);
            // 将绘制的帧设置为黑色
            draw_frame.setTo(cv::Scalar(0, 0, 0, 0));
            // 在绘制的帧上绘制检测结果
            yolov8.draw_results(draw_frame,yolo_results);
            result_mutex.unlock();
            kpu_frame_count += 1;
            v4l2_drm_dump_release(&context);
        }
    }
    // 如果模型类型为yolov5，则执行yolov5的推理流程
    else if(strcmp(yolo_config.model_type, "yolov5") == 0){
        // 创建一个Yolov5对象，用于执行yolov5的推理流程
        Yolov5 yolov5(yolo_config.task_type,yolo_config.task_mode,yolo_config.kmodel_path,yolo_config.conf_thres,yolo_config.nms_thres,yolo_config.mask_thres,labels,image_wh,yolo_config.debug_mode);
        
        while (!ai_stop) {
            int ret = v4l2_drm_dump(&context, 1000);
            if (ret) {
                perror("v4l2_drm_dump error");
                continue;
            }
            runtime_tensor input_tensor= sensor_buf.get_buf_for_index(context.vbuffer.index);        
            // 执行预处理
            yolov5.pre_process(input_tensor);
            // 执行推理
            yolov5.inference();
            result_mutex.lock();
            // 执行后处理
            yolov5.post_process(yolo_results);
            // 将绘制的帧设置为黑色
            draw_frame.setTo(cv::Scalar(0, 0, 0, 0));
            // 在绘制的帧上绘制检测结果
            yolov5.draw_results(draw_frame,yolo_results);
            result_mutex.unlock();
            kpu_frame_count += 1;
            v4l2_drm_dump_release(&context);
        }
    }
    // 如果模型类型不是yolov5、yolov8或yolo11，则打印错误信息并返回-1
    else{
        std::cout << "仅支持模型: yolov5/yolov8/yolo11 " << std::endl;
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
                //---------------------- 绘制显示结果 ----------------------
                if (draw_buffer->width > draw_buffer->height)
                {
                    // 创建临时 BGRA 显示缓冲Mat（用于画图）
                    cv::Mat temp_img(draw_buffer->height, draw_buffer->width, CV_8UC4);
                    // 横屏
                    temp_img.setTo(cv::Scalar(0, 0, 0, 0));
                    result_mutex.lock();
                    draw_frame.copyTo(temp_img);
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
                    draw_frame.copyTo(temp_img);
                    result_mutex.unlock();
                    // 旋转回屏幕方向
                    cv::rotate(temp_img, temp_img, cv::ROTATE_90_CLOCKWISE);
                    //---------------------- 显示缓冲同步 ----------------------
                    // 将绘图图像复制到实际显示缓冲区
                    memcpy(draw_buffer->map, temp_img.data, draw_buffer->size);
                }
                last_drawed_buffer = buffer;
                // flush cache
                thead_csi_dcache_clean_invalid_range(draw_buffer->map, draw_buffer->size);
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
    if (v4l2_drm_setup(&context, 1, &display)) {
        std::cerr << "v4l2_drm_setup error" << std::endl;
        return;
    }
    // 分配OSD显示 plane 和 buffer
    struct display_plane* plane = display_get_plane(display, DRM_FORMAT_ARGB8888);
    draw_buffer = display_allocate_buffer(plane, display->width, display->height);
    display_commit_buffer(draw_buffer, 0, 0);

    if (draw_buffer->width > draw_buffer->height)
    {
        draw_frame = cv::Mat(draw_buffer->height, draw_buffer->width, CV_8UC4, cv::Scalar(0,0, 0, 0));
    }
    else{
        draw_frame = cv::Mat(draw_buffer->width, draw_buffer->height, CV_8UC4, cv::Scalar(0,0, 0, 0));
    }

    std::cout << "press 'q' to exit" << std::endl;
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

void _help(){
    printf("Please input:\n");
    printf("-model_type: default yolov8, yolov5/yolov8/yolo11\n");
    printf("-task_type: default detect, yolov5 support classify/detect/segment, yolov8 and yolo11 support classify/detect/segment/obb task\n");
    printf("-task_mode: default video, image/video\n");
    printf("-image_path: default test.jpg, image path\n");
    printf("-kmodel_path: default yolov8n.kmodel, kmodel path\n");
    printf("-labels_txt_filepath: default coco_labels.txt, labels txt filepath\n");
    printf("-conf_thres: default 0.35\n");
    printf("-nms_thres: default 0.65\n");
    printf("-mask_thres: default 0.5\n");
    printf("-kp_num: default 17\n");
    printf("-kp_dim: default 3\n");
    printf("-debug_mode: default 0, 0/1\n");
}


int main(int argc, char *argv[])
{
    std::cout << "case " << argv[0] << " built at " << __DATE__ << " " << __TIME__ << std::endl;

    YoloConfig yolo_config;

    // 遍历命令行参数，解析并设置配置项
    for (int i = 1; i < argc; i += 2)
    {
        if (strcmp(argv[i], "-help") == 0)
        {
            // 打印帮助信息
            _help();
            return 0;
        }
        else if (strcmp(argv[i], "-model_type") == 0)
        {
            // 设置模型类型
            yolo_config.model_type = argv[i + 1];
        }
        else if (strcmp(argv[i], "-task_type") == 0)
        {
            // 设置任务类型
            yolo_config.task_type = argv[i + 1];
        }
        else if (strcmp(argv[i], "-task_mode") == 0)
        {
            // 设置任务模式
            yolo_config.task_mode = argv[i + 1];
        }
        else if (strcmp(argv[i], "-image_path") == 0)
        {
            // 设置图像路径
            yolo_config.image_path = argv[i + 1];
        }
        else if (strcmp(argv[i], "-kmodel_path") == 0)
        {
            // 设置模型路径
            yolo_config.kmodel_path = argv[i + 1];
        }
        else if (strcmp(argv[i], "-labels_txt_filepath") == 0)
        {
            // 设置标签文件路径
            yolo_config.labels_txt_filepath = argv[i + 1];
        }
        else if (strcmp(argv[i], "-conf_thres") == 0)
        {
            // 设置置信度阈值
            yolo_config.conf_thres = atof(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-nms_thres") == 0)
        {
            // 设置非极大值抑制阈值
            yolo_config.nms_thres = atof(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-mask_thres") == 0)
        {
            // 设置掩码阈值
            yolo_config.mask_thres = atof(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-kp_num") == 0)
        {
            // 设置关键点数量
            yolo_config.kp_num = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-kp_dim") == 0)
        {
            // 设置关键点维度
            yolo_config.kp_dim = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-debug_mode") == 0)
        {
            // 设置调试模式
            yolo_config.debug_mode = atoi(argv[i + 1]);
        }
        else
        {
            // 打印错误信息并退出
            printf("Error :Invalid arguments %s\n", argv[i]);
            _help();
            return -1;
        }
    }

    // 如果任务模式为视频，则执行视频推理
    if (strcmp(yolo_config.task_mode, "video") == 0)
    {

        display = display_init(0);
        if (!display) {
            cerr << "display_init error, exit" << endl;
            return -1;
        }

        // 锁住结果互斥量，等待首次帧到来后解锁
        result_mutex.lock();

        // 启动分类任务推理线程
        std::thread ai_thread(ai_proc, std::ref(yolo_config),2);
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
    else
    {
        // 读取图像文件
        cv::Mat ori_img = cv::imread(yolo_config.image_path);
        // 获取图像的宽度和高度
        FrameSize image_wh={ori_img.cols,ori_img.rows};
        // 创建一个空的向量，用于存储图像数据
        std::vector<uint8_t> chw_vec;
        // 创建一个包含3个元素的向量，用于存储图像的BGR通道
        std::vector<cv::Mat> bgrChannels(3);
        // 将图像分割成BGR通道
        cv::split(ori_img, bgrChannels);
        // 遍历BGR通道，将每个通道的数据转换为一维向量，并将其添加到chw_vec中
        for (auto i = 2; i > -1; i--)
        {
            std::vector<uint8_t> data = std::vector<uint8_t>(bgrChannels[i].reshape(1, 1));
            chw_vec.insert(chw_vec.end(), data.begin(), data.end());
        }
        // 从标签文件中读取标签
        std::vector<std::string> labels=readLabelsFromTxt(yolo_config.labels_txt_filepath);
        // 创建一个空的向量，用于存储YOLO检测结果
        std::vector<YOLOBbox> yolo_results;
        // 创建一个维度向量，用于定义输入张量的形状
        dims_t in_shape { 1, 3, ori_img.rows, ori_img.cols };
        // 创建一个运行时张量，用于存储输入数据
        runtime_tensor input_tensor = host_runtime_tensor::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("cannot create input tensor");
        // 获取输入张量的主机缓冲区
        auto input_buf = input_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
        // 将图像数据复制到输入张量的主机缓冲区中
        memcpy(reinterpret_cast<char *>(input_buf.data()), chw_vec.data(), chw_vec.size());
        // 将输入张量的数据同步到设备上
        hrt::sync(input_tensor, sync_op_t::sync_write_back, true).expect("write back input failed");

        if(strcmp(yolo_config.model_type, "yolo26") == 0){
        Yolo26 yolo26(yolo_config.task_type,yolo_config.task_mode,yolo_config.kmodel_path,yolo_config.conf_thres,yolo_config.mask_thres,labels,image_wh,yolo_config.kp_num,yolo_config.kp_dim,yolo_config.debug_mode);
        yolo26.pre_process(input_tensor);
        yolo26.inference();
        yolo26.post_process(yolo_results);
        yolo26.draw_results(ori_img,yolo_results);
        }
        // 如果模型类型为yolo11，则执行yolo11的推理流程
        else if(strcmp(yolo_config.model_type, "yolo11") == 0){
            Yolo11 yolo11(yolo_config.task_type,yolo_config.task_mode,yolo_config.kmodel_path,yolo_config.conf_thres,yolo_config.nms_thres,yolo_config.mask_thres,labels,image_wh,yolo_config.kp_num,yolo_config.kp_dim,yolo_config.debug_mode);
            yolo11.pre_process(input_tensor);
            yolo11.inference();
            yolo11.post_process(yolo_results);
            yolo11.draw_results(ori_img,yolo_results);
        }
        // 如果模型类型为yolov8，则执行yolov8的推理流程
        else if(strcmp(yolo_config.model_type, "yolov8") == 0){
            Yolov8 yolov8(yolo_config.task_type,yolo_config.task_mode,yolo_config.kmodel_path,yolo_config.conf_thres,yolo_config.nms_thres,yolo_config.mask_thres,labels,image_wh,yolo_config.kp_num,yolo_config.kp_dim,yolo_config.debug_mode);
            yolov8.pre_process(input_tensor);
            yolov8.inference();
            yolov8.post_process(yolo_results);
            yolov8.draw_results(ori_img,yolo_results);
        }
        // 如果模型类型为yolov5，则执行yolov5的推理流程
        else if(strcmp(yolo_config.model_type, "yolov5") == 0){
            Yolov5 yolov5(yolo_config.task_type,yolo_config.task_mode,yolo_config.kmodel_path,yolo_config.conf_thres,yolo_config.nms_thres,yolo_config.mask_thres,labels,image_wh,yolo_config.debug_mode);
            yolov5.pre_process(input_tensor);
            yolov5.inference();
            yolov5.post_process(yolo_results);
            yolov5.draw_results(ori_img,yolo_results);
        }
        // 如果模型类型不是yolov5、yolov8或yolo11，则打印错误信息并返回-1
        else{
            std::cout << "仅支持模型: yolov5/yolov8/yolo11 " << std::endl;
            return -1;
        }

        // 将任务类型和模型类型转换为字符串
        std::string task_(yolo_config.task_type);
        std::string model_(yolo_config.model_type);
        // 将推理结果保存为图像文件
        cv::imwrite("result_"+model_+"_"+task_+".jpg",ori_img);

    }
    return 0;
}
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
#include "face_detection.h"
#include "face_pose.h"
#include "crop.h"
#include "src.h"
#include "nanotracker.h"
#include "cv2_utils.h"

using std::cerr;
using std::cout;
using std::endl;
using std::thread;

//手势识别
static std::mutex result_mutex;
static vector<BoxInfo> hand_results;
static vector<HandKeyPointInfo> hand_keypoint_results;
//动态手势识别
int osd_width = -1,osd_height =-1;
cv::Mat dyn_osd_frame;
cv::Mat tracker_osd_frame;

static vector<FaceDetectionInfo> face_det_results;
static vector<FacePoseInfo> face_pose_results;

std::atomic<bool> ai_stop(false);

//手势识别模型及参数，包括手掌检测和手势关键点分类模型，关键点为21个
string g_hand_detection_path="hand_det.kmodel";
string g_hand_keypoint_path="handkp_det.kmodel";
float g_obj_thresh=0.15;
float g_nms_thresh=0.4;
//动态手势识别模型及参数，动态手势识别同时需要用到手势识别的两个模型
string g_dynamic_gesture_path="gesture.kmodel";
float g_gesture_obj_thresh=0.4;
float g_gesture_nms_thresh=0.4;
//人脸姿态估计模型及参数，包括人脸检测和脸部姿态估计模型
string g_face_detection_path="face_detection_320.kmodel";
float g_facedet_obj_thresh=0.6;
float g_facedet_nms_thresh=0.2;
string g_face_pose_path="face_pose.kmodel";
//单目标跟踪模型及参数
string g_nanotracker_backbone_crop_path="cropped_test127.kmodel";
string g_nanotracker_backbone_src_path="nanotrack_backbone_sim.kmodel";
string g_nanotracker_head_path="nanotracker_head_calib_k230.kmodel";
int g_template_inputsize=127;
int g_obj_inputsize=255;
int g_head_thresh=0.05;
//任务相关参数
int cur_task_state=0;
string cur_gesture="";
string dg_state="middle";
int debug_mode=0;

static volatile unsigned kpu_frame_count = 0;
static struct display* display;
static struct timeval tv, tv2;

void print_usage(const char *name)
{
	cout << "Usage: " << name << "<kmodel_det> <input_mode> <obj_thresh> <nms_thresh> <kmodel_kp> <debug_mode>" << endl
		 << "Options:" << endl
		 << "  kmodel_det      手掌检测kmodel路径\n"
		 << "  input_mode      本地图片(图片路径)/ 摄像头(None) \n"
         << "  obj_thresh      手掌检测阈值\n"
         << "  nms_thresh      手掌检测非极大值抑制阈值\n"
		 << "  kmodel_kp       手势关键点检测kmodel路径\n"
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

    // create tensors
    std::vector<std::tuple<int, void*>> tensors;
    for (unsigned i = 0; i < BUFFER_NUM; i++) {
        tensors.push_back({context.buffers[i].fd, context.buffers[i].mmap});
    }
    DMABufManager dma_buf = DMABufManager({SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},tensors);

    //手势识别和动态手势识别初始化
    HandDetection hd(g_hand_detection_path.c_str(), g_gesture_obj_thresh, g_gesture_nms_thresh, {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, debug_mode);
    HandKeypoint hk(g_hand_keypoint_path.c_str(), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, debug_mode);

    DynamicGesture Dag(g_dynamic_gesture_path.c_str(), debug_mode);

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
    cv::Mat trigger_argb;
    cv::Mat up_argb;
    cv::Mat down_argb;
    cv::Mat left_argb;
    cv::Mat right_argb;
    cv::Mat middle_argb;
    dyn_osd_frame = cv::Mat(osd_height, osd_width, CV_8UC3, cv::Scalar(0, 0, 0));

    //人脸姿态估计初始化
    FaceDetection fd(g_face_detection_path.c_str(), g_facedet_obj_thresh,g_facedet_nms_thresh, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},  debug_mode);
    FacePose fp(g_face_pose_path.c_str(), {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, debug_mode);

    // //单目标跟踪初始化
    init();
    Crop crop(g_nanotracker_backbone_crop_path.c_str(),{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, debug_mode);
    Src src(g_nanotracker_backbone_src_path.c_str() ,{SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},  debug_mode);
    NanoTracker track(g_nanotracker_head_path.c_str(),g_head_thresh, debug_mode);
    tracker_osd_frame = cv::Mat(osd_height, osd_width, CV_8UC3, cv::Scalar(0, 0, 0));

    int draw_mean_x;
    int draw_mean_y;
    int draw_mean_w;
    int draw_mean_h;
    bool enter_init = true;
    int seconds = 1;  //倒计时时长，单位秒
    time_t endtime;  //倒计时结束时间
    int flag=0;

    while (!ai_stop) {
        int ret = v4l2_drm_dump(&context, 1000);
        if (ret) {
            perror("v4l2_drm_dump error");
            continue;
        }
        runtime_tensor& img_data = dma_buf.get_buf_for_index(context.vbuffer.index);
        result_mutex.lock();

        // cur_gesture="";

        if(cur_task_state==0){
            hd.pre_process(img_data);
            hd.inference();
            hand_results.clear();
            hd.post_process(hand_results);

            hand_keypoint_results.clear();
            for (auto r: hand_results)
            {
                if(hand_results.size()>1){
                    break;
                }
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
                HandKeyPointInfo post_process_result;
                hk.post_process(bbox, post_process_result);
                hand_keypoint_results.push_back(post_process_result);

                float *results = post_process_result.pred.data();

                //识别手势
                std::vector<double> angle_list = hand_angle(results);
                cur_gesture = h_gesture(angle_list);
                
                //如果是“one”手势进入动态手势识别，“yeah”手势进入人脸姿态估计，“three”手势进入人脸单目标跟踪
                if(cur_gesture=="one"){
                    cur_task_state=1;
                    dg_state="middle";
                }
                else if(cur_gesture=="yeah"){
                    cur_task_state=2;
                }
                else if(cur_gesture=="three"){
                    cur_task_state=3;
                    flag=1;
                    enter_init=true;
                }
            }
        }
        else if(cur_task_state==1){
            if (cur_state_== TRIGGER)
            {
                ScopedTiming st("trigger time", debug_mode);
                dyn_osd_frame.setTo(cv::Scalar(0, 0, 0));

                hd.pre_process(img_data);
                hd.inference();
                hand_results.clear();
                hd.post_process(hand_results);

                hand_keypoint_results.clear();
                for (auto r: hand_results)
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
                    HandKeyPointInfo post_process_result;
                    hk.pre_process(img_data, bbox);
                    hk.inference();
                    hk.post_process(bbox,post_process_result);
                    hand_keypoint_results.push_back(post_process_result);

                    float *results = post_process_result.pred.data();

                    std::vector<double> angle_list = hand_angle(results);
                    cur_gesture = h_gesture(angle_list);

                    if(cur_gesture=="love"){
                        cur_task_state=0;
                    }

                    if ((cur_gesture == "five") ||(cur_gesture == "yeah"))
                    {
                        double v1_x = results[24] - results[0];
                        double v1_y = results[25] - results[1];
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
                                cv::Mat copy_image = dyn_osd_frame(cv::Rect(0,0,bin_width,bin_height));
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
                                cv::Mat copy_image = dyn_osd_frame(cv::Rect(0,0,bin_height,bin_width));
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
                                cv::Mat copy_image = dyn_osd_frame(cv::Rect(0,0,bin_width,bin_height));
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
                                cv::Mat copy_image = dyn_osd_frame(cv::Rect(0,0,bin_height,bin_width));
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
                dyn_osd_frame.setTo(cv::Scalar(0, 0, 0));
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
                        cv::Mat copy_image = dyn_osd_frame(cv::Rect(0,0,bin_width,bin_height));
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
                        cv::Mat copy_image = dyn_osd_frame(cv::Rect(0,0,bin_height,bin_width));
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
                        cv::Mat copy_image = dyn_osd_frame(cv::Rect(0,0,bin_width,bin_height));
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
                        cv::Mat copy_image = dyn_osd_frame(cv::Rect(0,0,bin_height,bin_width));
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
                    cv::putText(dyn_osd_frame, "UP", cv::Point(osd_width*3/7,osd_height/2),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 195, 0), 2);
                } else if (draw_state_ == RIGHT)
                {
                    cv::putText(dyn_osd_frame, "LEFT", cv::Point(osd_width*3/7,osd_height/2),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 195, 0), 2);
                }else if (draw_state_ == DOWN)
                {
                    cv::putText(dyn_osd_frame, "DOWN", cv::Point(osd_width*3/7,osd_height/2),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 195, 0), 2);
                }else if (draw_state_ == LEFT)
                {
                    cv::putText(dyn_osd_frame, "RIGHT", cv::Point(osd_width*3/7,osd_height/2),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 195, 0), 2);
                }else if (draw_state_ == MIDDLE)
                {
                    cv::putText(dyn_osd_frame, "MIDDLE", cv::Point(osd_width*3/7,osd_height/2),cv::FONT_HERSHEY_COMPLEX, 5, cv::Scalar(255, 195, 0), 2);
                }

            }else
            {
                draw_state_ = TRIGGER;
            }
        }
        else if(cur_task_state==2){

            hd.pre_process(img_data);
            hd.inference();
            hand_results.clear();
            hd.post_process(hand_results);

            hand_keypoint_results.clear();
            for (auto r: hand_results)
            {
                if(hand_results.size()>1){
                    break;
                }
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
                HandKeyPointInfo post_process_result;
                hk.post_process(bbox, post_process_result);
                hand_keypoint_results.push_back(post_process_result);

                float *results = post_process_result.pred.data();

                //识别手势
                std::vector<double> angle_list = hand_angle(results);
                cur_gesture = h_gesture(angle_list);

                if(cur_gesture=="love"){
                    cur_task_state=0;
                    break;
                }
                
            }

            fd.pre_process(img_data);
            fd.inference();
            face_det_results.clear();
            fd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, face_det_results);

            face_pose_results.clear();
            for (int i = 0; i < face_det_results.size(); ++i)
            {
                fp.pre_process(img_data,face_det_results[i].bbox);
                fp.inference();
                FacePoseInfo pose_result;
                fp.post_process(pose_result);
                face_pose_results.push_back(pose_result);
            }

        }
        else if(cur_task_state==3){
            tracker_osd_frame.setTo(cv::Scalar(0, 0, 0));

            hd.pre_process(img_data);
            hd.inference();
            hand_results.clear();
            hd.post_process(hand_results);

            hand_keypoint_results.clear();
            for (auto r: hand_results)
            {
                if(hand_results.size()>1){
                    break;
                }
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
                HandKeyPointInfo post_process_result;
                hk.post_process(bbox, post_process_result);
                hand_keypoint_results.push_back(post_process_result);

                float *results = post_process_result.pred.data();

                //识别手势
                std::vector<double> angle_list = hand_angle(results);
                cur_gesture = h_gesture(angle_list);

                if(cur_gesture=="love"){
                    cur_task_state=0;
                    break;
                }
            }

            if(flag==1){
                fd.pre_process(img_data);
                fd.inference();
                face_det_results.clear();
                fd.post_process({SENSOR_WIDTH, SENSOR_HEIGHT}, face_det_results);

                if(face_det_results.size()>=1){
                    int center_x=osd_width/2;
                    int center_y=osd_height/2;
                    int distance=center_x*center_x+center_y*center_y;
                    int idx=-1;
                    for(int i=0;i<face_det_results.size();i++){
                        int x = face_det_results[i].bbox.x / SENSOR_WIDTH * osd_width;
                        int y = face_det_results[i].bbox.y / SENSOR_HEIGHT * osd_height;
                        int w = face_det_results[i].bbox.w / SENSOR_WIDTH * osd_width;
                        int h = face_det_results[i].bbox.h / SENSOR_HEIGHT * osd_height;
                        int c_x=x+w/2;
                        int c_y=y+h/2;
                        int c_w=std::abs(center_x-c_x);
                        int c_h=std::abs(center_y-c_y);
                        int dis_idx=c_w*c_w+c_h*c_h;
                        if(dis_idx<distance){
                            distance=dis_idx;
                            idx=i;
                        }

                    }
                    int track_x1=face_det_results[idx].bbox.x;
                    int track_y1=face_det_results[idx].bbox.y;
                    int track_x2=face_det_results[idx].bbox.x+face_det_results[0].bbox.w;
                    int track_y2=face_det_results[idx].bbox.y+face_det_results[0].bbox.h;
                    float track_mean_x = (track_x2 + track_x1) / 2.0;
                    float track_mean_y = (track_y1 + track_y2) / 2.0;
                    float track_w = float(track_x2 - track_x1);
                    float track_h = float(track_y2 - track_y1);
                    draw_mean_w = int(track_w / SENSOR_WIDTH * osd_width);
                    draw_mean_h = int(track_h / SENSOR_HEIGHT * osd_height);
                    draw_mean_x = int(track_mean_x / SENSOR_WIDTH * osd_width - draw_mean_w / 2.0);
                    draw_mean_y = int(track_mean_y / SENSOR_HEIGHT * osd_height - draw_mean_h / 2.0);
                    set_center(track_mean_x, track_mean_y);
                    set_rect_size(track_w, track_h);
                    endtime = time(NULL) + seconds;  
                    flag=0;
                }
                else{
                    continue;
                }
            }
            
             //首先对整帧图像做预处理
            float w_z, h_z, s_z;
            float *pos = get_updated_position();
            w_z = pos[0] + CONTEXT_AMOUNT * (pos[0] + pos[1]);
            h_z = pos[1] + CONTEXT_AMOUNT * (pos[0] + pos[1]);
            s_z = round(sqrt(w_z * h_z));

            cv::Mat src_img;
            auto img_data_buf = img_data.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_read).unwrap().buffer();
            cv::Mat ori_img_R = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, img_data_buf.data());
            cv::Mat ori_img_G = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, img_data_buf.data() + 1 * SENSOR_HEIGHT*SENSOR_WIDTH);
            cv::Mat ori_img_B = cv::Mat(SENSOR_HEIGHT, SENSOR_WIDTH, CV_8UC1, img_data_buf.data() + 2 * SENSOR_HEIGHT*SENSOR_WIDTH);
            std::vector<cv::Mat> sensor_rgb;
            sensor_rgb.push_back(ori_img_R);
            sensor_rgb.push_back(ori_img_G);
            sensor_rgb.push_back(ori_img_B);
            cv::merge(sensor_rgb, src_img);

            cv::Mat src_input = sub_window(src_img, INSTANCE_SIZE, round(s_z * INSTANCE_SIZE / EXEMPLAR_SIZE));
            time_t nowtime = time(NULL);
            //在跟踪框初始化阶段，从图像中crop出待跟踪目标，经backbone处理成featuremap，等待后续跟踪使用
            if (enter_init && nowtime <= endtime)
            {
                cv::rectangle(tracker_osd_frame, cv::Rect( draw_mean_x,draw_mean_y,draw_mean_w,draw_mean_h ), cv::Scalar(255, 0,255, 0), 8, 2, 0); // ARGB
                cv::Mat crop_input = sub_window(src_img, EXEMPLAR_SIZE, round(s_z));
                crop.pre_process(crop_input);
                crop.inference();
                crop.post_process();
                if (nowtime > endtime)
                {
                    enter_init = false;
                }
            }
            // 跟踪阶段
            else
            {
                
               //对每一帧图像经过backbone处理成featuremap
                src.pre_process(src_input);
                src.inference();
                src.post_process();
                //将两个输出的featuremap输入到对比模型中，从回归结果中得到跟踪框
                std::vector<float*> inputs;
                inputs.clear();
                inputs.push_back(crop.output);
                inputs.push_back(src.output);
                track.pre_process(inputs);
                track.inference();
                std::vector<Tracker_box> track_boxes;
                track.post_process(SENSOR_WIDTH, SENSOR_HEIGHT,track_boxes);
                track.draw_track(track_boxes,{SENSOR_WIDTH, SENSOR_HEIGHT}, {osd_width,osd_height}, tracker_osd_frame);        
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

                if(cur_task_state==0){
                    for(int i=0;i<hand_results.size();++i)
                    {
                        HandKeypoint::draw_keypoints_video(img, hand_results[i], hand_keypoint_results[i], false);
                    }
                }
                else if(cur_task_state==1){
                    dyn_osd_frame.copyTo(img,dyn_osd_frame);
                }
                else if(cur_task_state==2){
                    for(int i=0;i<face_det_results.size();++i)
                    {
                        FacePose::draw_result_video(img,face_det_results[i].bbox,face_pose_results[i]);
                    }
                }
                else if(cur_task_state==3){
                    tracker_osd_frame.copyTo(img, tracker_osd_frame);
                }

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
    return 0;
}
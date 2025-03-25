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
#include "ocr_box.h"
#include "ocr_reco.h"
#include "sort.h"

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
	cout << "Usage: " << name << "<kmodel_det> <input_mode> <obj_thresh> <nms_thresh> <kmodel_kp> <kmodel_ocrdet> <threshold> <box_thresh> <kmodel_reco> <debug_mode>" << endl
		 << "Options:" << endl
		 << "  kmodel_det      手掌检测kmodel路径\n"
		 << "  input_mode      本地图片(图片路径)/ 摄像头(None) \n"
         << "  obj_thresh      手掌检测kmodel obj阈值\n"
         << "  nms_thresh      手掌检测kmodel nms阈值\n"
		 << "  kmodel_kp       手势关键点检测kmodel路径\n"
         << "  kmodel_ocrdet   ocr检测kmodel路径\n"
         << "  threshold       ocr检测 threshold\n"
         << "  box_thresh      ocr检测 box_thresh\n"
         << "  kmodel_reco     ocr识别kmodel路径 \n"
		 << "  debug_mode      是否需要调试, 0、1、2分别表示不调试、简单调试、详细调试\n"
		 << "\n"
		 << endl;
}

bool sortBoxesByY(const Boxb& box1, const Boxb& box2) 
{
  return box1.meany < box2.meany;
}

int ocr_process(OCRBox &ocrbox, OCRReco &ocrreco,cv::Mat ori_img, cv::Mat& osd_frame_img, int xmin, int ymin)
{
    int ori_w = ori_img.cols;
    int ori_h = ori_img.rows;

    ocrbox.pre_process(ori_img);
    ocrbox.inference();

    vector<Boxb> ocrbox_results;
    ocrbox.post_process({ori_w, ori_h}, ocrbox_results);

    std::sort(ocrbox_results.begin(), ocrbox_results.end(), sortBoxesByY);

    std::cout<<"ocr识别结果: "<<endl;
    for(int i = 0; i < ocrbox_results.size(); i++)
    {
        vector<Point> vec;
        vec.clear();
        for(int j = 0; j < 4; j++)
        {
            cv::Point tmp = ocrbox_results[i].vertices[j];

            tmp.x = (1.0*(tmp.x+xmin)/SENSOR_WIDTH)*osd_width;
            tmp.y = (1.0*(tmp.y+ymin)/SENSOR_HEIGHT)*osd_height;

            vec.push_back(tmp);
        }
        cv::RotatedRect rect = cv::minAreaRect(vec);
        cv::Point2f ver[4];
        rect.points(ver);

        for(int i = 0; i < 4; i++)
            line(osd_frame_img, ver[i], ver[(i + 1) % 4], Scalar(0, 0, 255), 3);

        cv::Mat crop;
        OCRReco::warppersp(ori_img, crop, ocrbox_results[i]);

        ocrreco.pre_process(crop);
        ocrreco.inference();

        vector<string> ocrreco_results;
        
        ocrreco.post_process(ocrreco_results);
        for (int i=0;i<ocrreco_results.size();i++)
        {
            std::cout<<ocrreco_results[i];
        }
        std::cout<<endl;
    }
    std::cout<<" "<<endl;
    return ocrbox_results.size();
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

    HandDetection hd(argv[1], atof(argv[3]), atof(argv[4]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[7]));
    HandKeypoint hk(argv[5], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[7]));
    OCRBox ocrbox(argv[6], atof(argv[7]), atof(argv[8]),  atoi(argv[10]));
    OCRReco ocrreco(argv[9],6625,atoi(argv[10]));

    std::vector<BoxInfo> results;
    
    // create tensors
    std::vector<std::tuple<int, void*>> tensors;
    for (unsigned i = 0; i < BUFFER_NUM; i++) {
        tensors.push_back({context.buffers[i].fd, context.buffers[i].mmap});
    }
    SensorBufManager sensor_buf = SensorBufManager({SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},tensors);

    Sort sort;
    int fi = 0;

    cv::Mat cropped_img;
    int ocr_det_size = -1;

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

        std::vector<Sort::TrackingBox> frameTrackingResult = sort.Sortx(results, fi);
        fi ++;

        for (auto r: results)
        {
            std::string text = hd.labels_[r.label] + ":" + std::to_string(round(r.score * 100) / 100.0);

            int w = r.x2 - r.x1 + 1;
            int h = r.y2 - r.y1 + 1;
            
            int rect_x = r.x1/ SENSOR_WIDTH * osd_width;
            int rect_y = r.y1 / SENSOR_HEIGHT  * osd_height;
            int rect_w = (float)w / SENSOR_WIDTH * osd_width;
            int rect_h = (float)h / SENSOR_HEIGHT  * osd_height;
            cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w, rect_h), cv::Scalar(255, 255, 255), 2, 2, 0);
        }

        if (frameTrackingResult.size()>0 && fi>=5)
        {
            cv::Point2f left_top;
            int index1 = 1;

            for(int i=0;i< frameTrackingResult.size();i++)
            {
                auto tb = frameTrackingResult[i];
                int rect_x = tb.box.x / SENSOR_WIDTH * osd_width;
                int rect_y = tb.box.y / SENSOR_HEIGHT  * osd_height;
                int rect_w = (float)tb.box.width / SENSOR_WIDTH * osd_width;
                int rect_h = (float)tb.box.height / SENSOR_HEIGHT  * osd_height;
                cv::rectangle(osd_frame, cv::Rect(rect_x, rect_y , rect_w + 20, rect_h + 20), cv::Scalar(255, 0, 255), 2, 2, 0);
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

                float *pred = hk.get_out()[0];
                int draw_x,draw_y;

                float pred_x = std::max(std::min(pred[(index1+1)*4*2], 1.0f), 0.0f);
                float pred_y = std::max(std::min(pred[(index1+1)*4*2+1], 1.0f), 0.0f);

                left_top.x = pred_x * w_1 + x1_1;
                left_top.y = pred_y * h_1 + y1_1;
                draw_x = left_top.x / SENSOR_WIDTH * osd_width;
                draw_y = left_top.y / SENSOR_HEIGHT * osd_height;

                cv::circle(osd_frame, cv::Point(draw_x, draw_y), 6, cv::Scalar(0,0,0), 3);
                cv::circle(osd_frame, cv::Point(draw_x, draw_y), 5, cv::Scalar(0,0,0), 3);

                ScopedTiming st("osd draw", atoi(argv[10]));
                hk.draw_keypoints(osd_frame, num, bbox, false);
            }

            int x_min = std::max(static_cast<int>(left_top.x-500), 0);
            int x_max = std::min(static_cast<int>(left_top.x), SENSOR_WIDTH);
            int y_min = std::max(static_cast<int>(left_top.y-300), 0);
            int y_max = std::min(static_cast<int>(left_top.y), SENSOR_HEIGHT);
            Bbox box_info = {x_min,y_min, (x_max-x_min),(y_max-y_min)};

            if ((x_max-x_min>32) && (y_max-y_min>32))
            {
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
                
                cropped_img = Utils::crop(ori_img, box_info);
                {
                    ScopedTiming st("ocr time", atoi(argv[10]));
                    ocr_det_size = ocr_process(ocrbox, ocrreco, cropped_img, osd_frame, x_min,y_min);
                }

                int x = 1.0 * x_min/ SENSOR_WIDTH * osd_width;
                int y = 1.0 * y_min / SENSOR_HEIGHT  * osd_height;
                int w = 1.0 * (x_max-x_min) / SENSOR_WIDTH * osd_width;
                int h = 1.0 * (y_max-y_min) / SENSOR_HEIGHT  * osd_height;

                cv::rectangle(osd_frame, cv::Rect(x, y, w, h), cv::Scalar(0, 255, 255), 2, 2, 0);
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
    if (argc != 11)
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
        cv::Mat img = cv::imread(argv[2]);
        cv::Mat img_draw = img.clone();

        int origin_w = img.cols;
        int origin_h = img.rows;
        FrameSize handimg_size = {origin_w, origin_h};

        HandDetection hd(argv[1], atof(argv[3]), atof(argv[4]), handimg_size, atoi(argv[10]));
        HandKeypoint hk(argv[5], atoi(argv[10]));
        OCRBox ocrbox(argv[6], atof(argv[7]), atof(argv[8]), atoi(argv[10]));
        OCRReco ocrreco(argv[9],6625,atoi(argv[10]));

        hd.pre_process(img);
        hd.inference();

        std::vector<BoxInfo> result_hd;
        hd.post_process(result_hd);

        int index1 = 1;
        for (auto r : result_hd)
        {
            std::string text = hd.labels_[r.label] + ":" + std::to_string(round(r.score * 100) / 100.0);
            std::cout << "text = " << text << std::endl;
            cv::Point2f left_top;
            
            int w = r.x2 - r.x1 + 1;
            int h = r.y2 - r.y1 + 1;
            cv::rectangle(img_draw, cv::Rect(static_cast<int>(r.x1), static_cast<int>(r.y1) , w, h), cv::Scalar(255, 255, 255), 2, 2, 0);
            
            int length = std::max(w,h)/2;
            int cx = (r.x1+r.x2)/2;
            int cy = (r.y1+r.y2)/2;
            int ratio_num = 1.26*length;

            int x1_1 = std::max(0,cx-ratio_num);
            int y1_1 = std::max(0,cy-ratio_num);
            int x2_1 = std::min(origin_w-1, cx+ratio_num);
            int y2_1 = std::min(origin_h-1, cy+ratio_num);
            int w_1 = x2_1 - x1_1+1;
            int h_1 = y2_1 - y1_1+1;

            struct Bbox bbox = {x:x1_1,y:y1_1,w:w_1,h:h_1};
            hk.pre_process(img, bbox);
            hk.inference();

            float *pred = hk.get_out()[0];

            float pred_x = std::max(std::min(pred[(index1+1)*4*2], 1.0f), 0.0f);
            float pred_y = std::max(std::min(pred[(index1+1)*4*2+1], 1.0f), 0.0f);

            left_top.x = pred_x * w_1 + x1_1;
            left_top.y = pred_y * h_1 + y1_1;

            cv::circle(img_draw, left_top, 6, cv::Scalar(0,0,0), 3);
            cv::circle(img_draw, left_top, 5, cv::Scalar(0,0,0), 3);

            hk.draw_keypoints(img_draw,text, bbox, true);

            int x_min = std::max(static_cast<int>(left_top.x-500), 0);
            int x_max = std::min(static_cast<int>(left_top.x), origin_w);
            int y_min = std::max(static_cast<int>(left_top.y-300), 0);
            int y_max = std::min(static_cast<int>(left_top.y), origin_h);
            Bbox box_info = {x_min,y_min, (x_max-x_min),(y_max-y_min)};

            if ((x_max!=x_min) && (y_max!=y_min))
            {
                cv::Mat cropped_img = Utils::crop(img, box_info);

                ocrbox.pre_process(cropped_img);
                ocrbox.inference();

                vector<Boxb> ocrbox_results;
                ocrbox.post_process({500, 300}, ocrbox_results);

                std::sort(ocrbox_results.begin(), ocrbox_results.end(), sortBoxesByY);

                std::cout<<"ocr识别结果: "<<endl;
                for(int i = 0; i < ocrbox_results.size(); i++)
                {
                    vector<Point> vec;
                    vec.clear();
                    for(int j = 0; j < 4; j++)
                    {
                        vec.push_back(ocrbox_results[i].vertices[j]);
                    }
                    cv::RotatedRect rect = cv::minAreaRect(vec);
                    cv::Point2f ver[4];
                    rect.points(ver);
                    cv::Mat crop;
                    OCRReco::warppersp(cropped_img, crop, ocrbox_results[i]);

                    ocrreco.pre_process(crop);
                    ocrreco.inference();

                    vector<string> ocrreco_results;
                    
                    ocrreco.post_process(ocrreco_results);
                    for (int i=0;i<ocrreco_results.size();i++)
                    {
                        std::cout<<ocrreco_results[i];
                    }
                    std::cout<<endl;

                    for(int i = 0; i < 4; i++)
                        line(img_draw, cv::Point2f(ver[i].x+x_min,ver[i].y+y_min), cv::Point2f(ver[(i + 1) % 4].x+x_min,ver[(i + 1) % 4].y+y_min), Scalar(255, 0, 0), 3);
                }

                cv::rectangle(img_draw, cv::Rect(x_min, y_min , (x_max-x_min), (y_max-y_min)), cv::Scalar(255, 255, 0), 2, 2, 0);
            }
        }
        cv::imwrite("hand_ocr_result.jpg", img_draw);
    }
    return 0;
}
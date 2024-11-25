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

using std::cerr;
using std::cout;
using std::endl;
using std::thread;

static std::mutex result_mutex;
std::atomic<bool> ai_stop(false);
static volatile unsigned kpu_frame_count = 0;
static struct display* display;
static struct timeval tv, tv2;

int osd_width = -1,osd_height =-1;
cv::Mat osd_frame;

void print_usage(const char *name)
{
	cout << "Usage: " << name << "<kmodel_det> <obj_thresh> <nms_thresh> <kmodel_kp> <bin_file> <level> <debug_mode>" << endl
		 << "Options:" << endl
		 << "  kmodel_det      手掌检测kmodel路径\n"
         << "  obj_thresh      手掌检测阈值\n"
         << "  nms_thresh      手掌检测非极大值抑制阈值\n"
		 << "  kmodel_kp       手势关键点检测kmodel路径\n"
         << "  bin_file        拼图文件 (文件名 或者 None(表示排序数字拼图模式))\n"
         << "  level           拼图游戏难度\n"
		 << "  debug_mode      是否需要调试,0、1、2分别表示不调试、简单调试、详细调试\n"
		 << "\n"
		 << endl;
}

// 读取bin文件的函数
void read_binary_file_bin(std::string file_name,unsigned char *outi)
{
    std::ifstream ifs(file_name, std::ios::binary);
    ifs.seekg(0, ifs.end);
    size_t len = ifs.tellg();
    ifs.seekg(0, ifs.beg);
    ifs.read((char *)(outi), len);
    ifs.close();
}

void bin_2_mat(std::string bin_data_path, int mat_width, int mat_height, cv::Mat &image_argb)
{
    unsigned char *bin_data = new unsigned char[mat_width*mat_height*4];
    read_binary_file_bin(bin_data_path,bin_data);
    std::vector<Mat> image_argb_vec;
    image_argb_vec.push_back(cv::Mat(mat_height, mat_width, CV_8UC1, bin_data));
    image_argb_vec.push_back(cv::Mat(mat_height, mat_width, CV_8UC1, bin_data + 1 * mat_width * mat_height));
    image_argb_vec.push_back(cv::Mat(mat_height, mat_width, CV_8UC1, bin_data + 2 * mat_width * mat_height));
    image_argb_vec.push_back(cv::Mat(mat_height, mat_width, CV_8UC1, bin_data + 3 * mat_width * mat_height));
    cv::merge(image_argb_vec, image_argb);
    delete[] bin_data;
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

    
    int puzzle_width;
    int puzzle_height;
    int puzzle_ori_width;
    int puzzle_ori_height;
    puzzle_width = osd_height;
    puzzle_height = osd_height;
    puzzle_ori_width = osd_width-puzzle_height-5;
    puzzle_ori_height = osd_width-puzzle_height-5;


    int level = atoi(argv[6]);
    int every_block_width = puzzle_width/level;
    int every_block_height = puzzle_height/level;
    float ratio_num = every_block_width/360.0;
    int blank_x = 0;
    int blank_y = 0;
    std::vector<int> direction_vec = {-1,1,-1,1};

    cv::Mat image_puzzle_argb;
    cv::Mat image_puzzle_ori;
    cv::Mat osd_frame_tmp(osd_height, osd_width, CV_8UC3, cv::Scalar(0, 0, 0));

    int exact_division_x = 0;
    int exact_division_y = 0;
    int distance_tow_points = osd_width;
    int distance_thred = every_block_width*0.3;
    cv::Rect move_rect;
    cv::Mat move_mat;
    cv::Mat copy_blank;
    cv::Mat copy_move;

    if (strcmp(argv[5], "None") == 0)
    {
        image_puzzle_argb = cv::Mat(puzzle_height, puzzle_width, CV_8UC3, cv::Scalar(130, 150, 100));
        for(int i = 0; i < level*level; i++)
        {
            cv::rectangle(image_puzzle_argb, cv::Rect((i%level)*every_block_width, (i/level)*every_block_height, every_block_width, every_block_height), cv::Scalar(255, 255, 255), 5);
            std::string classString = to_string(i);
            cv::Size textSize = cv::getTextSize(classString, cv::FONT_HERSHEY_DUPLEX, 7*ratio_num, 8*ratio_num, 0);
            cv::putText(image_puzzle_argb, classString, cv::Point((i%level)*every_block_width + (every_block_width-textSize.width)/2,(i/level)*every_block_height + (every_block_height+textSize.height)/2),cv::FONT_HERSHEY_COMPLEX, 7*ratio_num, cv::Scalar(0, 0, 255), 8*ratio_num, 0);
        }
    }
    else
    {
        bin_2_mat(argv[5], puzzle_width, puzzle_height, image_puzzle_argb);
        for(int i = 0; i < level*level; i++)
        {
            cv::rectangle(image_puzzle_argb, cv::Rect((i%level)*every_block_width, (i/level)*every_block_height, every_block_width, every_block_height), cv::Scalar(255, 255, 255), 5);
        }
    }
    std::cout << "1========================== " << std::endl;

    cv::Mat blank_block_puzzle(every_block_height, every_block_width, CV_8UC3, cv::Scalar(114, 114, 114));
    cv::Mat image_puzzle_argb_blank = image_puzzle_argb(cv::Rect(0,0,every_block_width,every_block_height));
    blank_block_puzzle.copyTo(image_puzzle_argb_blank);

    std::cout << puzzle_ori_width << " ====  " << puzzle_ori_height << std::endl;
    cv::resize(image_puzzle_argb, image_puzzle_ori, cv::Size(puzzle_ori_width, puzzle_ori_height), cv::INTER_AREA);
    std::cout << "44-------------------------------- " << std::endl;
    cv::Mat copy_ori_image_0 = osd_frame_tmp(cv::Rect(0,0,puzzle_width,puzzle_height));
    image_puzzle_argb.copyTo(copy_ori_image_0);
    cv::Mat copy_ori_image_1;
    std::cout << "-------------------------------- " << std::endl;

    copy_ori_image_1 = osd_frame_tmp(cv::Rect(puzzle_width+2,(1080-puzzle_ori_height)/2,puzzle_ori_width,puzzle_ori_height));
    image_puzzle_ori.copyTo(copy_ori_image_1);

    cv::Mat blank_block(every_block_height, every_block_width, CV_8UC3, cv::Scalar(114, 114, 114));

    std::cout << "2========================== " << std::endl;

    srand((unsigned)time(NULL));
    for(int i = 0; i < level*10; i++)
    {
        int k230_random = rand() % 4;
        int blank_x_tmp = blank_x;
        int blank_y_tmp = blank_y;
        if (k230_random < 2)
        {
            blank_x_tmp = blank_x + direction_vec[k230_random];
        }
        else
        {
            blank_y_tmp = blank_y + direction_vec[k230_random];
        }

        if((blank_x_tmp >= 0 && blank_x_tmp < level) && (blank_y_tmp >= 0 && blank_y_tmp < level) && (std::abs(blank_x - blank_x_tmp) <= 1 && std::abs(blank_y - blank_y_tmp) <= 1))
        {
            move_rect = cv::Rect(blank_x_tmp*every_block_width,blank_y_tmp*every_block_height,every_block_width,every_block_height);
            move_mat = osd_frame_tmp(move_rect);

            copy_blank = osd_frame_tmp(cv::Rect(blank_x_tmp*every_block_width,blank_y_tmp*every_block_height,every_block_width,every_block_height));
            copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
            move_mat.copyTo(copy_move);
            blank_block.copyTo(copy_blank);

            blank_x = blank_x_tmp;
            blank_y = blank_y_tmp;
        }
    }

    std::cout << "3========================== " << std::endl;

    HandDetection hd(argv[1], atof(argv[2]), atof(argv[3]), {SENSOR_WIDTH, SENSOR_HEIGHT}, {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[7]));
    HandKeypoint hk(argv[4], {SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH}, atoi(argv[7]));

    std::vector<BoxInfo> results;
    std::vector<int> two_point;

    // create tensors
    std::vector<std::tuple<int, void*>> tensors;
    for (unsigned i = 0; i < BUFFER_NUM; i++) {
        tensors.push_back({context.buffers[i].fd, context.buffers[i].mmap});
    }
    DMABufManager dma_buf = DMABufManager({SENSOR_CHANNEL, SENSOR_HEIGHT, SENSOR_WIDTH},tensors);

    while (!ai_stop) {
        int ret = v4l2_drm_dump(&context, 1000);
        if (ret) {
            perror("v4l2_drm_dump error");
            continue;
        }
        auto img_data = dma_buf.get_buf_for_index(context.vbuffer.index);
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
                ScopedTiming st("osd draw", atoi(argv[7]));
                hk.draw_keypoints(osd_frame, text, bbox, false, two_point);
            }
        }

        if(max_id_hand != -1 && two_point[1] <= SENSOR_WIDTH)
        {
            distance_tow_points = std::sqrt(std::pow((two_point[0] - two_point[2]),2) + std::pow((two_point[1] - two_point[3]),2))* 1.0 / SENSOR_WIDTH * osd_width;
            exact_division_x = (two_point[0] * 1.0 / SENSOR_WIDTH * osd_width)/every_block_width;
            exact_division_y = (two_point[1] * 1.0 / SENSOR_HEIGHT * osd_height)/every_block_height;
            if(distance_tow_points < distance_thred && exact_division_x >= 0 && exact_division_x < level && exact_division_y >= 0 && exact_division_y < level)
            {   
                if(std::abs(blank_x - exact_division_x) == 1 && std::abs(blank_y - exact_division_y) == 0)
                {
                    move_rect = cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height);
                    move_mat = osd_frame_tmp(move_rect);

                    copy_blank = osd_frame_tmp(cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height));
                    copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
                    move_mat.copyTo(copy_move);
                    blank_block.copyTo(copy_blank);

                    blank_x = exact_division_x;
                }
                else if (std::abs(blank_y - exact_division_y) == 1 && std::abs(blank_x - exact_division_x) == 0)
                {
                    move_rect = cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height);
                    move_mat = osd_frame_tmp(move_rect);

                    copy_blank = osd_frame_tmp(cv::Rect(exact_division_x*every_block_width,exact_division_y*every_block_height,every_block_width,every_block_height));
                    copy_move = osd_frame_tmp(cv::Rect(blank_x*every_block_width,blank_y*every_block_height,every_block_width,every_block_height));
                    move_mat.copyTo(copy_move);
                    blank_block.copyTo(copy_blank);

                    blank_y = exact_division_y;
                }

                osd_frame = osd_frame_tmp.clone();
                if(two_point.size() > 0)
                {
                    int x1 = two_point[0] * 1.0 / SENSOR_WIDTH * osd_width;
                    int y1 = two_point[1] * 1.0 / SENSOR_HEIGHT * osd_height;
                    cv::circle(osd_frame, cv::Point(x1, y1), 10, cv::Scalar(0, 255, 255), 10);
                }
            }
            else
            {
                osd_frame = osd_frame_tmp.clone();
                if(two_point.size() > 0)
                {
                    int x1 = two_point[0] * 1.0 / SENSOR_WIDTH * osd_width;
                    int y1 = two_point[1] * 1.0 / SENSOR_HEIGHT * osd_height;
                    cv::circle(osd_frame, cv::Point(x1, y1), 10, cv::Scalar(255, 255, 0), 10);
                }
            }
        }
        else
        {
            osd_frame = osd_frame_tmp.clone();
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
    if (argc != 8)
    {
        print_usage(argv[0]);
        return -1;
    }

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
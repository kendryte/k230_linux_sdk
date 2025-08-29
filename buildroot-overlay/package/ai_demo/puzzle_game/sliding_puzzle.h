#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <algorithm>

class SlidingPuzzle {
public:
    SlidingPuzzle(int level,int osd_width,int osd_height);

    void initialize(cv::Mat &draw_frame);
    void process_hand(const std::vector<int> &two_point, cv::Mat &draw_frame);

private:
    int osd_width_;
    int osd_height_;
    int level_;
    int block_w_, block_h_;
    int puzzle_width_, puzzle_height_;
    int puzzle_ori_width_, puzzle_ori_height_;
    int blank_x_, blank_y_;

    cv::Mat blank_block_;

    void draw_numbered_blocks(cv::Mat &img, float ratio);
    void shuffle_puzzle(cv::Mat &draw_frame);
};

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "key.h"
#include "utils.h"

class Keyboard {
public:
    Keyboard(FrameCHWSize image_size,int osd_width,int osd_height);

    ~Keyboard();

    void draw_result(cv::Mat &draw_frame,std::vector<int> &two_point);

private:
    int character_len = 39;
    std::vector<Key> keys;
    Key textBox = Key(0, 0, 0, 0, " ");
    float previousClick = 0.0f;
    FrameCHWSize ori_size_={3,1080,1920};
    FrameCHWSize image_size_;
    FrameCHWSize osd_size_;
};
#endif

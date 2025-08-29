#include <opencv2/opencv.hpp>
#include "keyboard.h"

Keyboard::Keyboard(FrameCHWSize image_size,int osd_width,int osd_height){
    image_size_=image_size;
    osd_size_={4,osd_height,osd_width};
    std::string letters = "QWERTYUIOPASDFGHJKLZXCVBNM";
    int box_w = 140.0 / ori_size_.width * osd_width;
    int box_h = 100.0 / ori_size_.height * osd_height;
    int startX = 215.0 / ori_size_.width * osd_width;
    int startY = 360.0 / ori_size_.height * osd_height;
    int margin = 10.0 / ori_size_.width * osd_width;
    int startX2 = 290.0 / ori_size_.width * osd_width;
    int startX3 = 365.0 / ori_size_.height * osd_height;
    int box_w_space = 650.0 / ori_size_.width * osd_width;

    // Letter keys
    for (int i = 0; i < letters.size(); i++) {
        int x = 0, y = 0;
        if (i < 10) {
            x = startX + i * (box_w + margin);
            y = startY;
        } else if (i < 19) {
            x = startX2 + (i - 10) * (box_w + margin);
            y = startY + box_h + margin;
        } else {
            x = startX3 + (i - 19) * (box_w + margin);
            y = startY + 2 * (box_h + margin);
        }
        keys.push_back(Key(x, y, box_w, box_h, std::string(1, letters[i])));
    }

    // "clr" key
    keys.push_back(Key(startX3 + (26 - 19) * (box_w + margin),startY + 2 * (box_h + margin),box_w, box_h, "clr"));
    // "Space"
    keys.push_back(Key(startX2, startY + 3 * (box_h + margin), box_w_space, box_h, "Space"));
    // "<--" keys
    keys.push_back(Key(startX2 + margin + box_w_space, startY + 3 * (box_h + margin), box_w_space, box_h, "<--"));
    // Text box
    textBox = Key(startX, startY - box_h - margin, 10 * box_w + 9 * margin, box_h, " ");
}

Keyboard::~Keyboard(){}

void Keyboard::draw_result(cv::Mat &draw_frame,std::vector<int> &two_point){
    int x1=two_point[0];
    int y1=two_point[1];
    int x2=two_point[2];
    int y2=two_point[3];
    float dis=cv::sqrt((x1-x2) * (x1-x2) + (y1-y2) * (y1-y2));
    if(dis<200){
        int cx=(x1+x2)/2;
        int cy=(y1+y2)/2;
        cv::circle(draw_frame,cv::Point(cx,cy),5,cv::Scalar(0,255,0,255),cv::FILLED);
    }

    float alpha = 0.5;
    textBox.drawKey(draw_frame, 0.3, 0.5);
    for (auto& k : keys) 
    {
        if (k.isOver(x1, y1)) 
        {
            alpha = 0.2;
            if ((k.isOver(x2, y2)) && (dis<200))
            {
                float clickTime = cv::getTickCount();
                if ((clickTime - previousClick) / cv::getTickFrequency() > 0.6) 
                {
                    if (k.text_ == "<--") 
                    {
                        textBox.text_ = textBox.text_.substr(0, textBox.text_.size() - 1);
                    }
                    else if (k.text_ == "clr") 
                    {
                        textBox.text_ = "";
                    }
                    else if (textBox.text_.size() < character_len) 
                    {
                        if (k.text_ == "Space") {
                            textBox.text_ += " ";
                        }
                        else {
                            textBox.text_ += k.text_;
                        }
                    }
                    previousClick = clickTime;
                }
            }
        }
        k.drawKey(draw_frame, alpha, 0.5);
        alpha = 0.5;
    }
}

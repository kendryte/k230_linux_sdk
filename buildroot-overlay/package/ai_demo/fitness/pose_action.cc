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

#include "pose_action.h"
#include "utils.h"


std::vector<action_helper> action_recs(3);

float get_leg_ratio(std::vector<KKeyPoint> &kpts_sframe,std::vector<int> kpts ) {
  
  int aver_hip = ( kpts_sframe[kpts[2*0]].p.y + kpts_sframe[kpts[2*0+1]].p.y ) / 2.0;
  int aver_knee = ( kpts_sframe[kpts[2*1]].p.y + kpts_sframe[kpts[2*1+1]].p.y ) / 2.0;
  int aver_ankle = ( kpts_sframe[kpts[2*2]].p.y + kpts_sframe[kpts[2*2+1]].p.y ) / 2.0;

  float ratio = ( aver_hip - aver_knee ) * 1.0 / (aver_knee - aver_ankle);

  return ratio;

}

int PoseAction::check_deep_down(std::vector<KKeyPoint> &kpts_sframe, int recid,float thres_conf) 
{

    float down_thres=0.5;
    float up_thres=0.9;

    if(kpts_sframe.empty()) {
        return action_recs[recid].action_count;
    }

    float ratio;

    std::vector<int> kpts = {11,12,13,14,15,16};

    if (kpts_sframe[kpts[0]].prob > thres_conf && kpts_sframe[kpts[1]].prob > thres_conf &&
     kpts_sframe[kpts[2]].prob > thres_conf &&  kpts_sframe[kpts[3]].prob > thres_conf && 
      kpts_sframe[kpts[4]].prob > thres_conf &&  kpts_sframe[kpts[5]].prob > thres_conf) {
        ratio = get_leg_ratio(kpts_sframe,kpts);
    }
    else {
        return action_recs[recid].action_count;
    }

    if (ratio > up_thres && action_recs[recid].mark){
        action_recs[recid].latency += 1;
        if (action_recs[recid].latency == 2) {
            action_recs[recid].mark = false;
            action_recs[recid].latency = 0;
        }
    }
    else if( ratio < down_thres && !(action_recs[recid].mark)) {
        action_recs[recid].latency += 1;
        if (action_recs[recid].latency == 2) {
            action_recs[recid].action_count += 1;
            action_recs[recid].mark = true;
            action_recs[recid].latency = 0;
        }
    }
    return action_recs[recid].action_count;
}


int PoseAction::single_action_check(std::vector<KKeyPoint> &results_kpts, float thres_conf, int actionid, int recid) {


  if (results_kpts.size()==0 || actionid>3) {
    return action_recs[recid].action_count;
  }
  else {
    return check_deep_down(results_kpts,recid,thres_conf);
  }
}

void PoseAction::DrawPred_video(cv::Mat& img, FrameSize frame_size, std::vector<OutputPose>& results,
              const std::vector<std::vector<unsigned int>> &SKELLTON,
              const std::vector<std::vector<unsigned int>> &KPS_COLORS,
              const std::vector<std::vector<unsigned int>> &LIMB_COLORS,float thres_conf)
{
    int osd_width= img.cols;
    int osd_height = img.rows;

    int SENSOR_HEIGHT = frame_size.height;
    int SENSOR_WIDTH = frame_size.width;

    cv::Point origin;

    const int num_point =17;
    std::vector<KKeyPoint> keypoints;

    for (auto &result:results){
        int  left,top,width, height;
        left = result.box.x;
        top = result.box.y;
        width = result.box.width;
        height = result.box.height;

        std::string label = "person:  " + std::to_string(result.confidence).substr(0, 4);
        int baseLine;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        top = std::max(top, labelSize.height) -10 ;
 
        // 连线
        keypoints.clear();
        auto &kps = result.kps;

        for( int k=0;k<17;k++ )
        {
            int kps_x = std::round(kps[k*3]);
            int kps_y = std::round(kps[k*3 + 1]);
            float kps_s = kps[k*3 + 2];

            int kps_x1 = (float)kps_x / SENSOR_WIDTH * osd_width;
            int kps_y1 =  (float)kps_y / SENSOR_HEIGHT  * osd_height;


            cv::Point2f p(kps_x1,kps_y1);
            float prob = kps_s;

            keypoints.push_back( { p,prob } );
        }

        for (int k=0; k<num_point+2; k++){// 不要设置为>0.5f ,>0.0f显示效果比较好
            // 关键点绘制
            if (k<num_point){
                int kps_x = std::round(kps[k*3]);
                int kps_y = std::round(kps[k*3 + 1]);
                float kps_s = kps[k*3 + 2];
 
                int kps_x1 =  (float)kps_x / SENSOR_WIDTH * osd_width;
                int kps_y1 =   (float)kps_y / SENSOR_HEIGHT  * osd_height;

                if (kps_s > 0.0f){
                    cv::Scalar kps_color = cv::Scalar(KPS_COLORS[k][0],KPS_COLORS[k][1],KPS_COLORS[k][2]);
                    cv::circle(img, {kps_x1, kps_y1}, 5, kps_color, -3);
                }
            }
 
            auto &ske = SKELLTON[k];
            int pos1_x = std::round(kps[(ske[0] -1) * 3]);
            int pos1_y = std::round(kps[(ske[0] -1) * 3 + 1]);

            int pos1_x_ =  (float)pos1_x / SENSOR_WIDTH * osd_width;
            int pos1_y_ =  (float)pos1_y / SENSOR_HEIGHT  * osd_height;

            int pos2_x = std::round(kps[(ske[1] -1) * 3]);
            int pos2_y = std::round(kps[(ske[1] -1) * 3 + 1]);

            int pos2_x_ =  (float)pos2_x / SENSOR_WIDTH * osd_width;
            int pos2_y_ =  (float)pos2_y / SENSOR_HEIGHT  * osd_height;
 
            float pos1_s = kps[(ske[0] -1) * 3 + 2];
            float pos2_s = kps[(ske[1] -1) * 3 + 2];
 
            if (pos1_s > 0.0f && pos2_s >0.0f){// 不要设置为>0.5f ,>0.0f显示效果比较好
                cv::Scalar limb_color = cv::Scalar(LIMB_COLORS[k][0], LIMB_COLORS[k][1], LIMB_COLORS[k][3]);
                cv::line(img, {pos1_x_, pos1_y_}, {pos2_x_, pos2_y_}, limb_color,3);
            }
        }
    }
    action_count( img,keypoints,thres_conf);
}

void PoseAction::action_count( cv::Mat& image, std::vector<KKeyPoint> keypoints, float thresh)
{
    int deepdown = PoseAction::single_action_check(keypoints, thresh, 3, 2);
    std::string deep = "deep-down:" + std::to_string(deepdown);
    cv::putText(image, deep, cv::Point(10, 80), cv::FONT_HERSHEY_COMPLEX, 3, cv::Scalar(0, 255, 255), 2, 5, 0);
}















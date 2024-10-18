/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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

#ifndef _PULC_H_
#define _PULC_H_

#include <vector>
#include "utils.h"
#include "ai_base.h"
#include "person_detect.h"

using std::vector;


/**
 * @brief  Pulc 人体属性识别模型
 * 主要封装了对于每一帧图片，从预处理、运行到后处理给出结果的过程
 */
class Pulc: public AIBase
{
    public:

        /** 
        * @brief Pulc 构造函数，加载kmodel,并初始化kmodel输入、输出、类阈值和NMS阈值
        * @param kmodel_file kmodel文件路径
        * @param pulc_thresh 人体属性识别阈值
        * @param glasses_thresh 眼镜检测阈值
        * @param hold_thresh  手持物体阈值
        * @param debug_mode 0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */
        Pulc(const char *kmodel_file,float pulc_thresh, float glasses_thresh, float hold_thresh, const int debug_mode);

        /**
        * @brief Pulc 构造函数，加载kmodel,并初始化kmodel输入、输出、类阈值和NMS阈值
        * @param kmodel_file kmodel文件路径
        * @param isp_shape   isp输入大小（chw）
        * @param pulc_thresh 人体属性识别阈值
        * @param glasses_thresh 眼镜检测阈值
        * @param hold_thresh  手持物体阈值
        * @param debug_mode 0（不调试）、 1（只显示时间）、2（显示所有打印信息）
        * @return None
        */
        Pulc(const char *kmodel_file, FrameCHWSize isp_shape,float pulc_thresh, float glasses_thresh, float hold_thresh, const int debug_mode);


        /**
        * @brief 图片预处理（ai2d for image）
        * @param ori_img 原始图片
        * @param bbox    原始人脸检测框位置
        * @return None
        */
        void pre_process(cv::Mat ori_img, BoxInfo &bbox);

        /**
        * @brief 视频流预处理（ai2d for isp）
        * @param img_data 当前视频帧数据
        * @return None
        */
        void pre_process(runtime_tensor& img_data, Bbox &bbox);

        /**
        * @brief kmodel推理
        * @return None
        */
        void inference();

        /** 
        * @brief postprocess 函数，输出解码
        * @return string (gender + age + direction + glasses + hat + hold_obj + bag + upper + lower + shoe)
        */
        string post_process();

        /** 
        * @brief GetGender 函数，获取性别
        * @return string ("Female" : "Male")
        */
        string GetGender();

        /** 
        * @brief GetAge 函数，获取年龄
        * @return string ( "AgeLess18" , "Age18-60" or "AgeOver60")
        */
        string GetAge();

        /** 
        * @brief GetDirection 函数，获取朝向
        * @return string ("Front" ,  "Side" or "Back")
        */
        string GetDirection();

        /** 
        * @brief GetGlasses 函数，获取配戴眼镜情况
        * @return string ("Glasses: " + "True" or "False")
        */
        string GetGlasses();

        /** 
        * @brief GetHat 函数，获取戴帽子情况
        * @return string ( "Hat: "  "True" or "False")
        */
        string GetHat();

        /** 
        * @brief GetHoldObj 函数，获取手持物体情况
        * @return string ( "HoldObjectsInFront: " + "True" or "False")
        */
        string GetHoldObj();

        /** 
        * @brief GetBag 函数，获取带包情况
        * @return string ( "HandBag" , "ShoulderBag" , "Backpack" or "No bag")
        */
        string GetBag();
    
        /** 
        * @brief GetUpper 函数，获取上身衣着情况
        * @return string ( "Upper: " + "LongSleeve " , "ShortSleeve " , "UpperStride" , "UpperLogo" , "UpperPlaid" or "UpperSplice")
        */
        string GetUpper();

        /** 
        * @brief GetLower 函数，获取下身衣着情况
        * @return string ( "Lower: " +  "LowerStripe" , "LowerPattern" , "LongCoat"  , "Trousers" , "Shorts" or "Skirt&Dress")
        */
        string GetLower();

        /** 
        * @brief GetShoe 函数，获取穿鞋情况
        * @return string ( "Boots" or "No boots");)
        */
        string GetShoe();

        /** 
        * @brief  personDetect 析构函数
        * @return None
        */
        ~Pulc();

    private:

        void get_affine_matrix(Bbox &bbox);

        float pulc_thresh_;  // 人体属性识别阈值
        float glasses_thresh_;  // 眼镜识别阈值
        float hold_thresh_ ;   // 手持物体阈值

        float *output_0;  // 输出

        std::unique_ptr<ai2d_builder> ai2d_builder_; // ai2d构建器
        runtime_tensor ai2d_in_tensor_;              // ai2d输入tensor
        runtime_tensor ai2d_out_tensor_;             // ai2d输出tensor
        
        FrameCHWSize isp_shape_;                     // isp对应的地址大小
        cv::Mat matrix_dst_;                         // affine的变换矩阵
};
#endif

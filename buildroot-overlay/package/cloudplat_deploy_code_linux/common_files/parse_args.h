#ifndef PARSE_ARGS_
#define PARSE_ARGS_

#include <vector>
#include <iostream>
#include <fstream>
#include "json.h"

using namespace std;
using std::cout;
using std::endl;
using std::ifstream;
using std::vector;

/**
 * @brief config配置参数
 */
typedef struct config_args
{
    int input_height;//kmodel输入的高度 
    int input_width;//kmodel输入的宽度
    float obj_thresh;//目标分数阈值
    float nms_thresh;//非极大值抑制阈值
    float mask_thresh;//掩码阈值
    float box_thresh;//ocr检测框阈值
    int num_class;//标签类别数
    int reg_max;//gfldet的区域数
    int dict_num;//ocr识别的字典大小
    int strides[3];//每个输出分辨率的缩减倍数
    bool fixed_length;//ocr识别是否是直接resize或者padding resize
    bool nms_option;
    std::string model_type;//任务模型类别名称
    std::string kmodel_path;//kmodel的路径
    float anchors[3][3][2];//预设锚框
    std::vector<std::string> labels;//标签类别名称
    std::vector<int> memory_bank_shape;//异常检测memory_bank形状信息
    float image_threshold;//异常检测分数阈值

}config_args;

void parse_args(std::string config,config_args& args,int debug_mode);

#endif
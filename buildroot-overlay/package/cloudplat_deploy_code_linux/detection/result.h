#ifndef _RESULT_H
#define _RESULT_H


/**
 * @brief 多目标检测的结果结构
 */
typedef struct ob_det_res
{
    float x1;//检测框左上角点横坐标
    float y1;//检测框左上角点纵坐标
    float x2;//检测框右下角点横坐标
    float y2;//检测框右下角点纵坐标
    float score;//检测框的分数
    int label_index;//检测分类类别索引
    string label;//健侧类别名称
}ob_det_res;

/**
 * @brief 中心点对
 */
typedef struct CenterPrior
{
    int x;
    int y;
    int stride;
}CenterPrior;


#endif
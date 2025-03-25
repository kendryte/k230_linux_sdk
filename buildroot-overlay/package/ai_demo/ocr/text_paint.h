#ifndef TEXT_PAINT_H
#define TEXT_PAINT_H

#include <opencv2/opencv.hpp>  // 包含OpenCV库
#include <ft2build.h>          // 包含FreeType构建配置
#include FT_FREETYPE_H         // 包含FreeType核心库
#include <string>              // 包含std::string
#include <locale>              // 包含std::locale和std::codecvt
#include <codecvt>             // 包含std::wstring_convert (C++17之前)

// 定义ChineseTextRenderer类
class ChineseTextRenderer {
private:
    FT_Library ft;  // FreeType库句柄
    FT_Face face;   // 字体面

public:
    // 构造函数，用于初始化FreeType库和字体
    ChineseTextRenderer(std::string fontFile, int fontSize);

    // 析构函数，用于释放资源
    ~ChineseTextRenderer();

    // 绘制中文文本的方法
    void putChineseText(cv::Mat& img, std::string text, cv::Point position, cv::Scalar color);
};

#endif // CHINESE_TEXT_RENDERER_H
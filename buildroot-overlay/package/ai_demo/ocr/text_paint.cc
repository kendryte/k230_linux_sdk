#include "text_paint.h"
#include <iostream>
#include <fstream>

// 构造函数的具体实现
ChineseTextRenderer::ChineseTextRenderer(std::string fontFile, int fontSize) {
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "无法初始化FreeType库\n");
        return;
    }

    if (FT_New_Face(ft, fontFile.c_str(), 0, &face)) {
        fprintf(stderr, "无法加载字体文件\n");
        return;
    }

    // 设置字体大小
    FT_Set_Pixel_Sizes(face, 0, fontSize);
}

// 析构函数的具体实现
ChineseTextRenderer::~ChineseTextRenderer() {
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void ChineseTextRenderer::putChineseText(cv::Mat& img, std::string text, cv::Point position, cv::Scalar color) {
    // 确保图像有3个通道来支持彩色
    if (img.channels() != 3) {
        cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
    }

    // 设置文本参数
    cv::Point pos(position.x, position.y); 
    
    // 创建一个转换器
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    // 将std::string转换为std::wstring
    std::wstring textw = converter.from_bytes(text);

    for (wchar_t c : textw) {
        FT_Load_Char(face, c, FT_LOAD_RENDER);

        FT_GlyphSlot glyph = face->glyph;

        // 将字形位图绘制到OpenCV图像上
        for (int row = 0; row < glyph->bitmap.rows; ++row) {
            for (int col = 0; col < glyph->bitmap.width; ++col) {
                int x = pos.x + glyph->bitmap_left + col;
                int y = pos.y - glyph->bitmap_top + row;

                if (x >= 0 && x < img.cols && y >= 0 && y < img.rows) {
                    // 只有当位图缓冲区中的值不为0时才绘制
                    if (glyph->bitmap.buffer[row * glyph->bitmap.pitch + col] > 0) {
                        // 使用灰度值作为alpha混合因子
                        uchar gray = glyph->bitmap.buffer[row * glyph->bitmap.pitch + col];
                        cv::Vec3b& pixel = img.at<cv::Vec3b>(y, x);
                        for (int i = 0; i < 3; ++i) {
                            // 混合当前颜色与前景色
                            pixel[i] = static_cast<uchar>((pixel[i] * (255 - gray) + color[i] * gray) / 255);
                        }
                    }
                }
            }
        }

        // 更新绘制位置
        pos.x += glyph->advance.x >> 6;
    }
}
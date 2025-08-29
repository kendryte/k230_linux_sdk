/* Copyright (c) 2025, Canaan Bright Sight Co., Ltd
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
#include "text_paint.h"
#include <iostream>
#include <fstream>

/**
 * @class TextRenderer
 * @brief 基于 FreeType 实现的文本绘制类，支持UTF-8字符串渲染到OpenCV图像
 */

/**
 * @brief 构造函数，初始化 FreeType 库并加载指定字体文件，设置字体大小
 * 
 * @param fontFile 字体文件路径（如 *.ttf）
 * @param fontSize 字体大小（像素单位）
 */
TextRenderer::TextRenderer(std::string fontFile, int fontSize) {
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "无法初始化FreeType库\n");
        return;
    }

    if (FT_New_Face(ft, fontFile.c_str(), 0, &face)) {
        fprintf(stderr, "无法加载字体文件\n");
        return;
    }

    // 设置字体大小，宽度为0表示自动按高度缩放
    FT_Set_Pixel_Sizes(face, 0, fontSize);
}

/**
 * @brief 析构函数，释放 FreeType 资源
 */
TextRenderer::~TextRenderer() {
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

/**
 * @brief 在图像上绘制文本（支持 UTF-8 编码）
 * 
 * @param img      待绘制文本的图像（BGR或BGRA）
 * @param text     UTF-8 编码的文本字符串
 * @param position 文本左下角起始绘制位置
 * @param color    文本颜色，cv::Scalar格式，BGR通道顺序
 * 
 * @note 若输入图像为灰度图，会自动转换为BGR格式
 * @note 该方法逐字绘制，使用 FreeType 渲染字体字形并进行alpha混合
 */
void TextRenderer::putText(cv::Mat& img, std::string text, cv::Point position, cv::Scalar color) {
    // 确保图像通道为3（BGR）或4（BGRA），否则转换为BGR
    if (img.channels() != 3 && img.channels() != 4) {
        cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
    }

    // 将 UTF-8 字符串转换为宽字符（wchar_t）字符串
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wtext = converter.from_bytes(text);

    cv::Point pos = position;

    // 遍历每个宽字符，渲染并绘制到图像
    for (wchar_t wc : wtext) {
        if (FT_Load_Char(face, wc, FT_LOAD_RENDER)) {
            continue; // 跳过加载失败的字形
        }

        FT_GlyphSlot glyph = face->glyph;
        FT_Bitmap& bitmap = glyph->bitmap;

        // 遍历字形位图像素，进行混合绘制
        for (int row = 0; row < bitmap.rows; ++row) {
            for (int col = 0; col < bitmap.width; ++col) {
                int x = pos.x + glyph->bitmap_left + col;   // 计算目标图像X坐标
                int y = pos.y - glyph->bitmap_top + row;    // 计算目标图像Y坐标

                // 边界检查，跳过图像外部像素
                if (x < 0 || x >= img.cols || y < 0 || y >= img.rows) continue;

                int buffer_index = row * bitmap.pitch + col;
                if (buffer_index >= bitmap.rows * bitmap.pitch) continue;

                uchar gray = bitmap.buffer[buffer_index];  // 字形像素灰度值
                if (gray == 0) continue;                    // 透明像素跳过

                if (img.channels() == 3) {
                    // BGR图像通道混合
                    cv::Vec3b& pixel = img.at<cv::Vec3b>(y, x);
                    for (int i = 0; i < 3; ++i) {
                        uchar fg = static_cast<uchar>(std::min(std::max(color[i], 0.0), 255.0));
                        pixel[i] = static_cast<uchar>((pixel[i] * (255 - gray) + fg * gray) / 255);
                    }
                }
                else if (img.channels() == 4) {
                    // BGRA图像通道混合，保留alpha通道最大值
                    cv::Vec4b& pixel = img.at<cv::Vec4b>(y, x);
                    for (int i = 0; i < 3; ++i) {
                        uchar fg = static_cast<uchar>(std::min(std::max(color[i], 0.0), 255.0));
                        pixel[i] = static_cast<uchar>((pixel[i] * (255 - gray) + fg * gray) / 255);
                    }
                    pixel[3] = std::max(pixel[3], gray); // 保留最大透明度
                }
            }
        }

        // 字符间距调整，向右移动绘制位置
        pos.x += glyph->advance.x >> 6;
    }
}

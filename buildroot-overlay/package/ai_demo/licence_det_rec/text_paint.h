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
#ifndef TEXT_PAINT_H
#define TEXT_PAINT_H

#include <opencv2/opencv.hpp>  ///< OpenCV库，提供图像处理相关功能
#include <ft2build.h>          ///< FreeType库构建配置头文件
#include FT_FREETYPE_H         ///< FreeType核心库头文件
#include <string>              ///< 标准字符串类
#include <locale>              ///< 语言环境相关类，支持字符转换
#include <codecvt>             ///< 字符编码转换类（C++17之前支持）

/**
 * @class TextRenderer
 * @brief 基于 FreeType 实现的文本渲染类，支持在 OpenCV 图像上绘制 UTF-8 编码的文本，包括中文
 *
 * 该类封装了 FreeType 库的初始化、字体加载和文本绘制功能，
 * 支持指定字体文件和字体大小，渲染效果自然，支持多通道图像（BGR/BGRA）。
 */
class TextRenderer {
private:
    FT_Library ft;  ///< FreeType 库句柄，用于管理字体资源
    FT_Face face;   ///< 字体面对象，表示加载的字体文件

public:
    /**
     * @brief 构造函数，初始化 FreeType 库并加载指定字体文件，设置字体大小
     * @param fontFile 字体文件路径（如 *.ttf）
     * @param fontSize 字体大小（以像素为单位）
     */
    TextRenderer(std::string fontFile, int fontSize);

    /**
     * @brief 析构函数，释放 FreeType 资源，防止内存泄漏
     */
    ~TextRenderer();

    /**
     * @brief 在图像上绘制文本，支持 UTF-8 编码字符串
     * @param img      待绘制文本的 OpenCV 图像（支持3通道BGR或4通道BGRA）
     * @param text     UTF-8 编码的文本字符串，支持中文
     * @param position 文本起始绘制位置（左下角坐标）
     * @param color    文本颜色，采用 cv::Scalar 表示，顺序为BGR通道
     */
    void putText(cv::Mat& img, std::string text, cv::Point position, cv::Scalar color);
};

#endif // TEXT_PAINT_H

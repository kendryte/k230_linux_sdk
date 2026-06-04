#!/usr/bin/env python3
"""
k230_display 测试程序
"""

import numpy as np
import cv2
import time
import sys
import select
import k230_display

#可以使用如下命令构建测试图片
# ffmpeg -f lavfi -i testsrc2=duration=1:size=1920x1080:rate=1 -vframes 1 color_bars.png
def show_image_file(filepath):
    """显示 PNG/JPG 图片文件（OpenCV BGRA 格式，零转换）"""
    print(f"显示图片: {filepath}")

    # 读取图片（OpenCV 默认 BGR 格式）
    img_bgr = cv2.imread(filepath)
    if img_bgr is None:
        print(f"错误: 无法读取图片 {filepath}")
        return

    # 转成 RGB 格式
    img_rgb = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2RGB)

    # 获取显示尺寸
    disp_width = k230_display.get_width()
    disp_height = k230_display.get_height()

    # 调整图像尺寸以适配显示
    img_h, img_w = img_rgb.shape[:2]
    print(f"图片尺寸: {img_w}x{img_h}, 显示尺寸: {disp_width}x{disp_height} {img_rgb.shape}")
    print(f"red{img_rgb[500,30]},green{img_rgb[500,508]},blue{img_rgb[500,1023]}")

    # 如果图片大于显示尺寸，提示并返回
    if img_w > disp_width or img_h > disp_height:
        print(f"错误: 图片尺寸大于显示尺寸，不显示")
        return

    # 显示 RGB 图片
    k230_display.show(img_rgb)
    print("显示完成，按任意键退出...")
    sys.stdin.readline()



def main():
    print("k230_display 测试程序")
    k230_display.init()
    show_image_file("color_bars.png")
    k230_display.deinit()


if __name__ == "__main__":
    main()

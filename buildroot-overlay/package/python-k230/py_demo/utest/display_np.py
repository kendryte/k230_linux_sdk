import time
import numpy as np
import k230_display


k230_display.init()

# 获取显示尺寸
disp_width = k230_display.get_width()
disp_height = k230_display.get_height()
print(f"Display size: {disp_width}x{disp_height}")

# 竖屏时需要横屏构建图片（因为内部会旋转）
# 横屏: width > height, 直接使用
# 竖屏: width < height, 需要交换宽高
if disp_width < disp_height:
    # 竖屏，图片按横屏构建
    img_height, img_width = disp_width, disp_height
    print(f"Portrait mode, using landscape image size: {img_width}x{img_height}")
else:
    # 横屏
    img_height, img_width = disp_height, disp_width

# BGR 格式图片列表（根据显示尺寸构建）
images = [
    np.full((img_height, img_width, 3), [255, 0, 0], dtype=np.uint8),   # 红色
    np.full((img_height, img_width, 3), [0, 255, 0], dtype=np.uint8),   # 绿色
    np.full((img_height, img_width, 3), [0, 0, 255], dtype=np.uint8),   # 蓝色
    np.full((img_height, img_width, 3), [0, 0, 0], dtype=np.uint8),     # 黑色
    np.full((img_height, img_width, 3), [255, 255, 255], dtype=np.uint8),# 白色
]

# 帧率统计变量
frame_count = 0
total_frame_count = 0
last_time = time.time()

while True:
    # 循环显示多张图片
    img = images[total_frame_count % len(images)]
    k230_display.show(img)

    frame_count += 1
    total_frame_count += 1
    time.sleep(0.015)

    # 每秒统计一次
    current_time = time.time()
    if current_time - last_time >= 1.0:
        fps = frame_count / (current_time - last_time)
        colors = ["红", "绿", "蓝", "黑", "白"]
        print(f"[{time.strftime('%H:%M:%S')}] FPS: {fps:.1f}, Total: {total_frame_count}, 显示: {colors[total_frame_count % len(images)]}")
        last_time = current_time
        frame_count = 0

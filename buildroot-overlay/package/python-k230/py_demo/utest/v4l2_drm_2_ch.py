import time
import numpy as np
import cv2
from k230_v4l2_drm import V4l2Drm, DRM_FORMAT_ARGB8888, ROTATION_0, ROTATION_90

# 创建双路实例, 启用OSD
v4l2 = V4l2Drm(context_num=2, osd=True)
display_w, display_h = v4l2.drm_init()
print(f"INFO: display {display_w}x{display_h}")

# 根据显示方向自动选择旋转角度
if display_w > display_h:
    rotation = ROTATION_0
else:
    rotation = ROTATION_90

# Context 0: 摄像头1 → 显示 (NV12)
v4l2.set_context(
    index=0, device=1,
    width=max(display_w, display_h), height=min(display_w, display_h),
    format="NV12", display=True
)

# Context 1: 摄像头2 → AI推理输入 (BG3P格式, 不显示)
v4l2.set_context(
    index=1, device=2,
    width=1280, height=720,
    format="BG3P", display=False
)

v4l2.set_rotation(0, rotation)

# 配置OSD格式
v4l2.set_osd_format(DRM_FORMAT_ARGB8888)

# 初始化流水线
if not v4l2.setup():
    print("Error: V4L2-DRM setup failed!")
    exit(-1)

v4l2.display_start()
v4l2.dump_start(index=1)  # 启动AI路采集

try:
    frame_count = 0
    fps = 0.0
    fps_start_time = time.time()

    while True:
        # 1. 采集AI路帧
        if not v4l2.dump_frame(index=1, timeout_ms=1000):
            continue

        # 2. 获取帧数据 (BG3P格式: shape=(3, H, W))
        frame = v4l2.get_buffer_array(index=1)

        # 3. 释放帧缓冲区
        v4l2.dump_release(index=1)

        # 4. AI推理 (使用nncaseruntime等)
        # results = ai_inference(frame)

        # 5. 帧率统计
        frame_count += 1
        elapsed = time.time() - fps_start_time
        if elapsed >= 1.0:
            fps = frame_count / elapsed
            frame_count = 0
            fps_start_time = time.time()
            print(f"[{time.strftime('%H:%M:%S')}] FPS: {fps:.1f}")

        # 6. 生成OSD图像 (ARGB8888)
        osd_img = np.zeros((min(display_w, display_h), max(display_w, display_h), 4), dtype=np.uint8)
        # 在osd_img上绘制检测框、文字等...
        # cv2.rectangle(osd_img, (x1, y1), (x2, y2), (B, G, R, A), 2)

        # 7. 显示帧率到OSD
        cv2.putText(osd_img, f"FPS: {fps:.1f}", (10, 40),
                    cv2.FONT_HERSHEY_SIMPLEX, 1.2, (0, 255, 0, 255), 2)

        # 8. 更新OSD显示
        v4l2.osd_update(osd_img)

except KeyboardInterrupt:
    print("\nStopping...")

v4l2.dump_stop(index=1)
v4l2.display_stop()
print("Done!")

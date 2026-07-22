import time
from k230_v4l2_drm import V4l2Drm, ROTATION_0, ROTATION_90

v4l2drm = V4l2Drm(context_num=1)
display_width, display_height = v4l2drm.drm_init()
print(f"INFO: display {display_width}x{display_height}")

# 根据显示方向自动选择旋转角度
if display_width > display_height:
    rotation = ROTATION_0
else:
    rotation = ROTATION_90

v4l2drm.set_context(
    index=0, device=1,
    width=max(display_width, display_height), height=min(display_width, display_height),
    format="NV12", display=True
)
v4l2drm.set_rotation(0, rotation)

if not v4l2drm.setup():
    print("Error: V4L2-DRM setup failed!")
    exit(-1)

v4l2drm.display_start()

try:
    while True:
        time.sleep(1)  # display_start 后台线程自动处理采集和显示，主线程休眠等待
except KeyboardInterrupt:
    print("\nStopping...")

v4l2drm.display_stop()
print("Done!")

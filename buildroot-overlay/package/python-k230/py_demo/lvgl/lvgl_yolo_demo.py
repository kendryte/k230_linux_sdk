#!/usr/bin/env python3
"""
LVGL + YOLOv8 Object Detection Demo for K230

结合 LVGL OSD 界面与 YOLOv8 目标检测:
- v4l2-drm 处理摄像头视频层
- YOLOv8 KPU 推理进行目标检测, 结果通过 osd_update 绘制
- LVGL 渲染 OSD 叠加层 (透明背景 + 标题/按钮等 UI 控件)

LVGL UI 初始化 + timer_handler 都在独立线程, AI 推理在主线程, 互不阻塞。
主线程只写 Python 变量, LVGL 线程读取并调用 lv_* API。

Usage:
    python3 lvgl_yolo_demo.py [kmodel_path] [obj_thresh] [nms_thresh]

Example:
    python3 lvgl_yolo_demo.py
    python3 lvgl_yolo_demo.py /root/app/object_detect_yolov8n/yolov8n_320.kmodel 0.3 0.45
"""

import sys
import os
import time
import random
import signal
import threading
import numpy as np
import cv2
import nncaseruntime as nn
import lvgl as lv
from k230_v4l2_drm import V4l2Drm, DRM_FORMAT_ARGB8888, ROTATION_90, ROTATION_0

# Ctrl+C 直接退出 (多线程下 KeyboardInterrupt 可能无法中断 C 扩展调用)
signal.signal(signal.SIGINT, lambda sig, frame: os._exit(0))

class ScopedTiming:
    """耗时统计工具类"""

    def __init__(self, info="", enable_profile=True):
        self.info = info
        self.enable_profile = enable_profile

    def __enter__(self):
        if self.enable_profile:
            self.start_time = time.time_ns()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if self.enable_profile:
            elapsed_time = time.time_ns() - self.start_time
            print(f"{self.info} took {elapsed_time / 1000000:.2f} ms")


# ============================================================================
# YOLO 配置常量
# ============================================================================
CONFIDENCE_THRES = 0.3
IOU_THRES = 0.45
KMODEL_PATH = "/root/app/object_detect_yolov8n/yolov8n_320.kmodel"
INPUT_SIZE = (320, 320)
SENSOR_2_AI_SIZE = (1280, 720)

# COCO 80 类
CLASS_NAMES = [
    'person', 'bicycle', 'car', 'motorcycle', 'airplane', 'bus',
    'train', 'truck', 'boat', 'traffic light', 'fire hydrant',
    'stop sign', 'parking meter', 'bench', 'bird', 'cat', 'dog',
    'horse', 'sheep', 'cow', 'elephant', 'bear', 'zebra', 'giraffe',
    'backpack', 'umbrella', 'handbag', 'tie', 'suitcase', 'frisbee',
    'skis', 'snowboard', 'sports ball', 'kite', 'baseball bat',
    'baseball glove', 'skateboard', 'surfboard', 'tennis racket',
    'bottle', 'wine glass', 'cup', 'fork', 'knife', 'spoon', 'bowl',
    'banana', 'apple', 'sandwich', 'orange', 'broccoli', 'carrot',
    'hot dog', 'pizza', 'donut', 'cake', 'chair', 'couch',
    'potted plant', 'bed', 'dining table', 'toilet', 'tv', 'laptop',
    'mouse', 'remote', 'keyboard', 'cell phone', 'microwave',
    'oven', 'toaster', 'sink', 'refrigerator', 'book', 'clock',
    'vase', 'scissors', 'teddy bear', 'hair drier', 'toothbrush'
]

random.seed(0)
COLORS = [[random.randint(0, 255) for _ in range(3)] + [255] for _ in range(80)]


# ============================================================================
# YOLO 后处理
# ============================================================================
def postprocess_yolo(predictions, confidence_thres, iou_thres,
                     input_size, img_size, ratio):
    """YOLOv8 后处理：解析输出、过滤、NMS"""
    boxes = predictions[:, :4]
    class_scores = predictions[:, 4:]

    scores = np.max(class_scores, axis=1)
    class_ids = class_scores.argmax(axis=1)

    mask = scores > confidence_thres
    boxes = boxes[mask]
    scores = scores[mask]
    class_ids = class_ids[mask]

    if len(boxes) == 0:
        return [], [], []

    boxes_xy = boxes[:, :2] - boxes[:, 2:4] / 2
    boxes_xy = boxes_xy / ratio
    boxes_wh = boxes[:, 2:4] / ratio
    boxes_xy2 = boxes_xy + boxes_wh
    boxes = np.concatenate([boxes_xy, boxes_xy2], axis=1).astype(np.float32)
    scores = scores.astype(np.float32)

    indices = cv2.dnn.NMSBoxes(boxes.tolist(), scores.tolist(),
                               confidence_thres, iou_thres)

    if len(indices) > 0:
        indices = indices.flatten()
        return boxes[indices], scores[indices], class_ids[indices]

    return [], [], []


def draw_detections(osd_img, boxes, scores, class_ids, colors,
                    class_names, fps, scale_factor=(1.0, 1.0)):
    """在 OSD 图像上绘制检测结果"""
    scale_x, scale_y = scale_factor

    cv2.putText(osd_img, f"FPS: {fps:.1f}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0, 255), 2)

    for box, score, class_id in zip(boxes, scores, class_ids):
        x1, y1 = int(box[0] * scale_x), int(box[1] * scale_y)
        x2, y2 = int(box[2] * scale_x), int(box[3] * scale_y)
        label = f"{class_names[class_id]}: {score:.2f}"
        color = colors[class_id]

        cv2.rectangle(osd_img, (x1, y1), (x2, y2), color, 2)
        cv2.putText(osd_img, label, (x1, y1 - 5),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1)


# ============================================================================
# LVGL 线程
# ============================================================================
def lvgl_thread(v4l2drm, ui_data, stop, ready):
    """LVGL 线程: 初始化驱动 + 创建 UI + 驱动 timer_handler。

    所有 lv_* 调用都在此线程内, 主线程只写 ui_data 字典。
    创建完 UI 后 set(ready), 主线程 wait(ready) 后继续。
    """
    # --- 初始化 LVGL 驱动 ---
    lv.init(v4l2drm, v4l2_drm_run_flag=1)
    print("[lvgl] initialized")

    # --- 创建 UI ---
    scr = lv.screen_active()
    scr.set_style_bg_opa(lv.OPA_TRANSP, lv.PART.MAIN)

    title = lv.Label(scr)
    title.set_text("K230 YOLOv8 + LVGL")
    title.set_style_bg_opa(lv.OPA_60, lv.PART.MAIN)
    title.set_style_bg_color(lv.color_black(), lv.PART.MAIN)
    title.set_style_text_color(lv.color_white(), lv.PART.MAIN)
    title.align(lv.ALIGN.TOP_MID, 0, 8)

    fps_label = lv.Label(scr)
    fps_label.set_text("FPS: --")
    fps_label.set_style_bg_opa(lv.OPA_60, lv.PART.MAIN)
    fps_label.set_style_bg_color(lv.color_black(), lv.PART.MAIN)
    fps_label.set_style_text_color(lv.color_make(0, 255, 0), lv.PART.MAIN)
    fps_label.align(lv.ALIGN.TOP_LEFT, 10, 40)

    count_label = lv.Label(scr)
    count_label.set_text("Objects: 0")
    count_label.set_style_bg_opa(lv.OPA_60, lv.PART.MAIN)
    count_label.set_style_bg_color(lv.color_black(), lv.PART.MAIN)
    count_label.set_style_text_color(lv.color_make(255, 200, 0), lv.PART.MAIN)
    count_label.align(lv.ALIGN.TOP_LEFT, 10, 65)

    btn = lv.Button(scr)
    btn.set_size(100, 36)
    btn.align(lv.ALIGN.BOTTOM_MID, 0, -15)
    btn_text = lv.Label(btn)
    btn_text.set_text("Hide OSD")
    btn_text.center()

    osd_visible = [True]
    def on_btn_click(event_code):
        if event_code == lv.EVENT.CLICKED:
            if osd_visible[0]:
                title.add_flag(lv.OBJ_FLAG.HIDDEN)
                fps_label.add_flag(lv.OBJ_FLAG.HIDDEN)
                count_label.add_flag(lv.OBJ_FLAG.HIDDEN)
                btn_text.set_text("Show OSD")
                osd_visible[0] = False
            else:
                title.remove_flag(lv.OBJ_FLAG.HIDDEN)
                fps_label.remove_flag(lv.OBJ_FLAG.HIDDEN)
                count_label.remove_flag(lv.OBJ_FLAG.HIDDEN)
                btn_text.set_text("Hide OSD")
                osd_visible[0] = True
    btn.add_event_cb(lv.EVENT.CLICKED, on_btn_click)

    ready.set()

    # --- timer_handler 循环 ---
    while not stop[0]:
        if ui_data['fps_new']:
            fps_label.set_text(f"FPS: {ui_data['fps']}")
            ui_data['fps_new'] = False
        if ui_data['count_new']:
            count_label.set_text(f"Objects: {ui_data['count']}")
            ui_data['count_new'] = False
        idle_ms = lv.timer_handler()
        if idle_ms > 0:
            time.sleep(idle_ms / 1000.0)
        #time.sleep(20)


# ============================================================================
# 初始化函数
# ============================================================================
def init_v4l2_drm():
    """初始化 V4L2-DRM, 返回 (v4l2drm, display_w, display_h) 或 (None, 0, 0)"""
    v4l2drm = V4l2Drm(context_num=2, osd=True)
    display_w, display_h = v4l2drm.drm_init(drm_id=0)
    if display_w < 0:
        print("Failed to initialize DRM display")
        return None, 0, 0
    print(f"[v4l2-drm] Display: {display_w}x{display_h}")

    rotation = ROTATION_0 if display_w > display_h else ROTATION_90

    v4l2drm.set_context(
        index=0, device=1,
        width=max(display_w, display_h), height=min(display_w, display_h),
        format="NV12", display=True
    )
    v4l2drm.set_context(
        index=1, device=2,
        width=SENSOR_2_AI_SIZE[0], height=SENSOR_2_AI_SIZE[1],
        format="BG3P", display=False
    )
    v4l2drm.set_rotation(0, rotation)
    v4l2drm.set_osd_format(DRM_FORMAT_ARGB8888)

    if not v4l2drm.setup():
        print("Error: V4L2-DRM setup failed!")
        return None, 0, 0

    return v4l2drm, display_w, display_h


def init_yolo(kmodel_path, display_w, display_h):
    """初始化 YOLO 模型 + AI2D, 返回 (kpu, ai2d, kpu_input_tensor, ratio, scale_factor, osd_img)"""
    kpu = nn.Interpreter()
    ai2d = nn.AI2D()
    kpu.load_model(kmodel_path)

    tmp_tensor = nn.RuntimeTensor.from_numpy(
        np.ones((1, 3, INPUT_SIZE[1], INPUT_SIZE[0]), dtype=np.uint8)
    )
    kpu.set_input_tensor(0, tmp_tensor)
    kpu_input_tensor = kpu.get_input_tensor(0)

    ratio = min(INPUT_SIZE[0] / SENSOR_2_AI_SIZE[0],
                INPUT_SIZE[1] / SENSOR_2_AI_SIZE[1])
    dw = INPUT_SIZE[0] - int(SENSOR_2_AI_SIZE[0] * ratio)
    dh = INPUT_SIZE[1] - int(SENSOR_2_AI_SIZE[1] * ratio)

    scale_factor = (
        max(display_w, display_h) / SENSOR_2_AI_SIZE[0],
        min(display_w, display_h) / SENSOR_2_AI_SIZE[1]
    )

    ai2d.set_datatype(nn.AI2D_FORMAT.NCHW_FMT, nn.AI2D_FORMAT.NCHW_FMT,
                      np.uint8, np.uint8)
    ai2d.set_resize_param(True, nn.AI2D_INTERP_METHOD.tf_bilinear,
                          nn.AI2D_INTERP_MODE.half_pixel)
    ai2d.set_pad_param(True,
                       [0, 0, 0, 0, 0, int(dh), 0, int(dw)],
                       0, [114, 114, 114])
    ai2d.build([1, 3, SENSOR_2_AI_SIZE[1], SENSOR_2_AI_SIZE[0]],
               [1, 3, INPUT_SIZE[1], INPUT_SIZE[0]])

    osd_h = min(display_w, display_h)
    osd_w = max(display_w, display_h)
    osd_img = np.zeros((osd_h, osd_w, 4), dtype=np.uint8)

    return kpu, ai2d, kpu_input_tensor, ratio, scale_factor, osd_img


# ============================================================================
# 主函数
# ============================================================================
def main():
    # --- 参数解析 ---
    import argparse
    parser = argparse.ArgumentParser(description='LVGL + YOLOv8 Demo for K230')
    parser.add_argument('--kmodel', default=KMODEL_PATH, help='kmodel path')
    parser.add_argument('--conf', type=float, default=CONFIDENCE_THRES, help='confidence threshold')
    parser.add_argument('--nms', type=float, default=IOU_THRES, help='NMS threshold')
    parser.add_argument('--debug', action='store_true', help='enable debug timing')
    args = parser.parse_args()

    debug_mode = args.debug
    kmodel_path = args.kmodel
    confidence_thres = args.conf
    iou_thres = args.nms

    if not os.path.exists(kmodel_path):
        print(f"Error: KModel file not found: {kmodel_path}")
        return -1
    print(f"KModel: {kmodel_path}")
    print(f"Confidence: {confidence_thres}, IOU: {iou_thres}")

    # --- 初始化 V4L2-DRM ---
    v4l2drm, display_w, display_h = init_v4l2_drm()
    if v4l2drm is None:
        return -1

    # --- 初始化 YOLO ---
    kpu, ai2d, kpu_input_tensor, ratio, scale_factor, osd_img = \
        init_yolo(kmodel_path, display_w, display_h)

    # --- 启动 LVGL 线程 ---
    ui_data = {'fps': '--', 'count': 0,
               'fps_new': False, 'count_new': False}
    lvgl_stop = [False]
    ui_ready = threading.Event()

    t = threading.Thread(target=lvgl_thread,
                         args=(v4l2drm, ui_data, lvgl_stop, ui_ready),
                         daemon=True, name="lvgl")
    t.start()
    ui_ready.wait()

    # --- 启动显示 ---
    v4l2drm.display_start()
    v4l2drm.dump_start(index=1)

    last_time = time.time()
    frame_count = 0
    ai_fps = 0.0

    print("Running... Ctrl+C to exit")

    while True:
        # --- AI 推理 ---
        with ScopedTiming("total", debug_mode):
            if not v4l2drm.dump_frame(index=1, timeout_ms=1000):
                continue
            frame_nchw = v4l2drm.get_buffer_array(index=1)

            with ScopedTiming("ai2d", debug_mode):
                ai2d_input_tensor = nn.RuntimeTensor.from_numpy(frame_nchw)
                ai2d.run(ai2d_input_tensor, kpu_input_tensor)
                v4l2drm.dump_release(index=1)

            with ScopedTiming("kpu", debug_mode):
                kpu.run()

            with ScopedTiming("output", debug_mode):
                model_output = kpu.get_output_tensor(0).to_numpy()
                predictions = model_output[0].transpose()

            with ScopedTiming("post", debug_mode):
                boxes, scores, class_ids = postprocess_yolo(
                    predictions, confidence_thres, iou_thres,
                    INPUT_SIZE, SENSOR_2_AI_SIZE, ratio
                )
            with ScopedTiming("post", debug_mode):
                # --- OSD 绘制 ---
                osd_img[:] = 0
                draw_detections(osd_img, boxes, scores, class_ids,
                                COLORS, CLASS_NAMES, ai_fps, scale_factor)
                v4l2drm.osd_update(osd_img)

            # --- 通知 LVGL 线程更新标签 ---
            ui_data['fps'] = f"{ai_fps:.1f}"
            ui_data['fps_new'] = True
            ui_data['count'] = len(boxes) if len(boxes) > 0 else 0
            ui_data['count_new'] = True

            # --- FPS 计算 ---
            frame_count += 1
            now_time = time.time()
            if now_time - last_time >= 1.0:
                ai_fps = frame_count / (now_time - last_time)
                last_time = now_time
                frame_count = 0
                print(f"[{time.strftime('%H:%M:%S')}] AI FPS: {ai_fps:.1f}")


if __name__ == "__main__":
    main()

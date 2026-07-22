#!/usr/bin/env python3
"""YOLOv8 目标检测 Demo (v4l2-drm + OSD)

使用 K230 的 v4l2-drm 捕获摄像头画面并显示，同时通过 KPU 运行 YOLOv8 推理，
将检测结果（边界框 + 类别名 + 置信度）绘制到 OSD 叠加层上。

流程:
    摄像头 → v4l2-drm 显示
           → AI2D 预处理 (resize + pad) → KPU 推理 → 后处理 → OSD 绘制

注意: 本示例不使用 LVGL，OSD 通过 numpy 数组直接绘制。
      如需 LVGL 交互 UI 版本，请参考 lvgl/lvgl_yolo_demo.py。

Usage:
    python3 yolo_v4l2_drm.py
"""

import numpy as np
import random
import cv2
import nncaseruntime as nn
import time
import sys
import select
import os
from k230_v4l2_drm import V4l2Drm, DRM_FORMAT_ARGB8888 ,ROTATION_90 , ROTATION_0

debug_mode=False
#debug_mode=True

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
# 配置常量
# ============================================================================
CONFIDENCE_THRES = 0.3      # 置信度阈值
IOU_THRES = 0.45            # NMS IoU 阈值
#KMODEL_PATH = "yolov8n_224.kmodel"  #https://g.a-bug.org/canmv/k230/nncase_kmodels_v2_11/-/blob/v2.11.0/ai_poc/kmodel/yolov8n_224.kmodel?ref_type=heads
#INPUT_SIZE = (224, 224)     # 模型输入尺寸 (width, height)
KMODEL_PATH = "/root/app/object_detect_yolov8n/yolov8n_320.kmodel"
INPUT_SIZE = (320, 320)     # 模型输入尺寸 (width, height)
#SENSOR_2_AI_SIZE = (1920,1080)
SENSOR_2_AI_SIZE = (1280, 720)

# COCO 数据集 80 类标签
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

# 为每个类别生成随机颜色 (RGBA 格式)
random.seed(0)
COLORS = [[random.randint(0, 255) for _ in range(3)] + [255] for _ in range(80)]


# ============================================================================
# 辅助函数
# ============================================================================
def postprocess_yolo(predictions, confidence_thres, iou_thres,
                     input_size, img_size, ratio):
    """YOLOv8 后处理：解析输出、过滤、NMS"""
    boxes = predictions[:, :4]          # [x_center, y_center, w, h]
    class_scores = predictions[:, 4:]   # 各类别置信度

    # 取每个候选框的最大类别得分
    scores = np.max(class_scores, axis=1)
    class_ids = class_scores.argmax(axis=1)

    # 过滤低置信度目标
    mask = scores > confidence_thres
    boxes = boxes[mask]
    scores = scores[mask]
    class_ids = class_ids[mask]

    if len(boxes) == 0:
        return [], [], []

    # 边界框格式转换 (xywh → xyxy) 并缩放回原图尺寸
    boxes_xy = boxes[:, :2] - boxes[:, 2:4] / 2
    boxes_xy = boxes_xy / ratio
    boxes_wh = boxes[:, 2:4] / ratio
    boxes_xy2 = boxes_xy + boxes_wh
    boxes = np.concatenate([boxes_xy, boxes_xy2], axis=1).astype(np.float32)
    scores = scores.astype(np.float32)

    # 执行 NMS
    indices = cv2.dnn.NMSBoxes(boxes.tolist(), scores.tolist(),
                               confidence_thres, iou_thres)

    if len(indices) > 0:
        indices = indices.flatten()
        return boxes[indices], scores[indices], class_ids[indices]

    return [], [], []


def draw_detections(osd_img, boxes, scores, class_ids, colors, class_names, fps, scale_factor=(1.0, 1.0)):
    """在 OSD 图像上绘制检测结果

    Args:
        scale_factor: AI 图像到显示图像的缩放因子 (scale_x, scale_y)
    """
    scale_x, scale_y = scale_factor

    # 绘制 FPS
    cv2.putText(osd_img, f"FPS: {fps:.1f}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0, 255), 2)

    # 绘制检测框
    for box, score, class_id in zip(boxes, scores, class_ids):
        # 将 AI 坐标缩放到显示坐标
        x1, y1 = int(box[0] * scale_x), int(box[1] * scale_y)
        x2, y2 = int(box[2] * scale_x), int(box[3] * scale_y)
        label = f"{class_names[class_id]}: {score:.2f}"
        color = colors[class_id]

        cv2.rectangle(osd_img, (x1, y1), (x2, y2), color, 2)
        cv2.putText(osd_img, label, (x1, y1 - 5),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1)


# ============================================================================
# 主检测函数
# ============================================================================
def detect():
     # 检查模型文件是否存在
    if not os.path.exists(KMODEL_PATH):
        print(f"Error: KMODEL file not found: {KMODEL_PATH}")
        print(f"Please download it with:")
        print(f"  wget --no-check-certificate https://raw.githubusercontent.com/kendryte/k230_linux_sdk/dev/buildroot-overlay/package/yolo/utils/yolov8n.kmodel")
        return -1

    """主检测函数：初始化摄像头、加载模型、执行推理、显示结果"""
    with ScopedTiming("init",True):
        # -------------------------------
        # 一、初始化摄像头与显示
        # -------------------------------
        v4l2drm = V4l2Drm(context_num=2)
        display_width, display_height = v4l2drm.drm_init()
        print(f"INFO: display {display_width}x{display_height}")
        if display_width > display_height:
            rotation = ROTATION_0
        else:
            rotation = ROTATION_90

        # Context 0: 摄像头直接显示 (NV12 格式)
        v4l2drm.set_context(
            index=0, device=1,
            width=max(display_width, display_height), height=min(display_width, display_height),
            format="NV12", display=True
        )

        # Context 1: AI 推理输入 (BG3P 格式,hwc)
        v4l2drm.set_context(
            index=1, device=2,
            width=SENSOR_2_AI_SIZE[0], height=SENSOR_2_AI_SIZE[1],
            format="BG3P", display=False
        )

        v4l2drm.set_rotation(0, rotation)

        v4l2drm.set_osd_format(DRM_FORMAT_ARGB8888)

        if not v4l2drm.setup():
            print("Error: V4L2-DRM setup failed!")
            return -1

        v4l2drm.display_start()
        v4l2drm.dump_start(index=1)

        # -------------------------------
        # 二、初始化模型与 AI2D 预处理
        # -------------------------------

        kpu = nn.Interpreter()
        ai2d = nn.AI2D()

        kpu.load_model(KMODEL_PATH)

        # 创建临时输入张量用于绑定
        tmp_tensor = nn.RuntimeTensor.from_numpy(
            np.ones((1, 3, INPUT_SIZE[1], INPUT_SIZE[0]), dtype=np.uint8)
        )
        kpu.set_input_tensor(0, tmp_tensor)
        kpu_input_tensor = kpu.get_input_tensor(0)

        # -------------------------------
        # 三、计算 letterbox 参数(计算图像缩放和填充参数)
        # -------------------------------
        ratio = min(INPUT_SIZE[0] / SENSOR_2_AI_SIZE[0], INPUT_SIZE[1] / SENSOR_2_AI_SIZE[1])
        new_w = int(SENSOR_2_AI_SIZE[0] * ratio)
        new_h = int(SENSOR_2_AI_SIZE[1] * ratio)
        dw = INPUT_SIZE[0] - new_w
        dh = INPUT_SIZE[1] - new_h
        pad_top, pad_bottom = 0, int(dh)
        pad_left, pad_right = 0, int(dw)

        # 计算 AI 图像到 OSD 图像的缩放因子
        scale_factor = ( max(display_width, display_height) / SENSOR_2_AI_SIZE[0], min(display_width, display_height) / SENSOR_2_AI_SIZE[1])

        # -------------------------------
        # 四、配置 AI2D 预处理流水线
        # -------------------------------
        ai2d.set_datatype(
            nn.AI2D_FORMAT.NCHW_FMT, # 输入格式
            nn.AI2D_FORMAT.NCHW_FMT,# 输出格式
            np.uint8, np.uint8 # 输入输出数据类型
        )
        # 设置 resize 参数（使用 tf_bilinear 双线性插值）
        ai2d.set_resize_param(True, nn.AI2D_INTERP_METHOD.tf_bilinear, nn.AI2D_INTERP_MODE.half_pixel)
        # 设置 padding 参数（补边）
        ai2d.set_pad_param(True, [0, 0, 0, 0, pad_top, pad_bottom, pad_left, pad_right],
                        0, [114, 114, 114])# 用灰色填充
        # 构建 AI2D pipeline（输入、输出 shape）
        ai2d.build([1, 3, SENSOR_2_AI_SIZE[1], SENSOR_2_AI_SIZE[0]],
                [1, 3, INPUT_SIZE[1], INPUT_SIZE[0]])  #NCHW;

        # -------------------------------
        # 五、主推理循环
        # -------------------------------
        last_time = time.time()
        frame_count = 0
        fps = 0.0

    try:
        while True:
            # time.sleep(1)
            # continue
            with ScopedTiming("totoal ", debug_mode):
                with ScopedTiming("dump_frame", debug_mode):
                    # 摄像头捕获
                    if not v4l2drm.dump_frame(index=1, timeout_ms=1000):
                        continue
                    # 获取图像数据,# 图像预处理：k230 BG3P出的就是,hwc
                    frame_nchw = v4l2drm.get_buffer_array(index=1)

                with ScopedTiming("ai2d", debug_mode):
                    # AI2D 预处理,执行 AI2D 预处理（resize + pad）
                    ai2d_input_tensor = nn.RuntimeTensor.from_numpy(frame_nchw)
                    ai2d.run(ai2d_input_tensor, kpu_input_tensor)
                    v4l2drm.dump_release(index=1)

                with ScopedTiming("kpu.run", debug_mode):
                    # 模型推理
                    kpu.run()

                with ScopedTiming("get out", debug_mode):
                    # 获取输出
                    model_output = kpu.get_output_tensor(0).to_numpy()
                    predictions = model_output[0].transpose()  # (8400, 84)

                with ScopedTiming("postprocess", debug_mode):
                    # 后处理
                    boxes, scores, class_ids = postprocess_yolo(
                        predictions, CONFIDENCE_THRES, IOU_THRES,
                        INPUT_SIZE, SENSOR_2_AI_SIZE, ratio
                    )

                with ScopedTiming("osd pic gen", debug_mode):
                    # 创建 OSD 图像并绘制
                    osd_img = np.zeros((min(display_width, display_height),max(display_width, display_height) , 4), dtype=np.uint8)
                    draw_detections(osd_img, boxes, scores, class_ids,
                                COLORS, CLASS_NAMES, fps, scale_factor)

                with ScopedTiming("osd draw", debug_mode):
                    v4l2drm.osd_update(osd_img)
                    pass

                # FPS 计算
                frame_count += 1
                now_time = time.time()
                if now_time - last_time >= 1.0:
                    fps = frame_count / (now_time - last_time)
                    last_time = now_time
                    frame_count = 0
                    print(f"[{time.strftime('%H:%M:%S')}] FPS: {fps:.1f}")
                    # 每秒保存一次 OSD 图像
                    #cv2.imwrite(f"osd_{int(now_time)}.png", osd_img)

    except KeyboardInterrupt:
        print("\nStopping...")

    # 清理资源
    v4l2drm.dump_stop(index=1)
    v4l2drm.display_stop()
    print("Done!")


# ============================================================================
# 主程序入口
# ============================================================================
if __name__ == '__main__':
    detect()

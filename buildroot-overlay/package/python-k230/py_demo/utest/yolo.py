import numpy as np
import random
import cv2
import nncaseruntime as nn
import time
import sys
import select
import k230_display

def detect():
    """主检测函数：初始化摄像头、加载模型、执行推理、显示结果"""
    print(time.time())

    # ===============================
    # 一、检测与模型相关参数配置
    # ===============================
    confidence_thres = 0.3      # 置信度阈值（过滤低置信度目标）
    iou_thres = 0.45            # NMS（非极大值抑制）的 IoU 阈值
    kmodel_path = "yolov8n_224.kmodel"  # 模型文件路径
    input_size = [224, 224]     # 模型输入分辨率（宽、高）

    # 为每个类别生成随机颜色（用于绘制框）
    random.seed(0)
    colors = [[random.randint(0, 255) for _ in range(3)] for _ in range(80)]

    # COCO 数据集的 80 类标签
    class_names = [
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

    # ===============================
    # 二、初始化摄像头与显示
    # ===============================

    # 打开摄像头设备（索引 1）
    cap = cv2.VideoCapture(1)

    # 初始化 K230 显示设备
    k230_display.init()
    display_width = k230_display.get_width()
    display_height = k230_display.get_height()
    print("INFO: display width:", display_width, "display height:", display_height)

    # 根据显示尺寸确定图像捕获尺寸
    img_size = [max(display_width, display_height), min(display_width, display_height)]

    # 检查摄像头是否成功打开
    if not cap.isOpened():
        raise IOError(f"无法打开摄像头索引 1")

    # 设置摄像头分辨率
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, img_size[0])
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, img_size[1])
    print("摄像头1初始化成功")

    # ===============================
    # 三、初始化模型与 AI2D 预处理
    # ===============================

    # 初始化 nncase 推理器和预处理引擎
    kpu = nn.Interpreter()
    ai2d = nn.AI2D()

    # 加载 YOLOv8 模型
    kpu.load_model(kmodel_path)

    # 创建一个临时输入张量用于绑定输入
    tmp_tensor = nn.RuntimeTensor.from_numpy(
        np.ones((1, 3, input_size[1], input_size[0]), dtype=np.uint8)
    )
    kpu.set_input_tensor(0, tmp_tensor)
    kpu_input_tensor = kpu.get_input_tensor(0)

    # ===============================
    # 四、计算图像缩放和填充参数（letterbox）
    # ===============================
    # 计算输入图像缩放比例，保持纵横比
    ratio = min(input_size[0] / img_size[0], input_size[1] / img_size[1])
    new_w, new_h = int(img_size[0] * ratio), int(img_size[1] * ratio)
    dw, dh = (input_size[0] - new_w), (input_size[1] - new_h)
    pad_left, pad_right = 0, int(dw)
    pad_top, pad_bottom = 0, int(dh)

    # ===============================
    # 五、配置 AI2D 预处理流水线
    # ===============================
    ai2d.set_datatype(
        nn.AI2D_FORMAT.NCHW_FMT,  # 输入格式
        nn.AI2D_FORMAT.NCHW_FMT,  # 输出格式
        np.uint8, np.uint8        # 输入输出数据类型
    )
    # 设置 resize 参数（使用 tf_bilinear 双线性插值）
    ai2d.set_resize_param(True, nn.AI2D_INTERP_METHOD.tf_bilinear, nn.AI2D_INTERP_MODE.half_pixel)
    # 设置 padding 参数（补边）
    ai2d.set_pad_param(True, [0, 0, 0, 0, pad_top, pad_bottom, pad_left, pad_right],
                       0, [114, 114, 114])  # 用灰色填充
    # 构建 AI2D pipeline（输入、输出 shape）
    ai2d.build([1, 3, img_size[1], img_size[0]], [1, 3, input_size[1], input_size[0]])

    # ===============================
    # 六、主推理循环
    # ===============================
    last_time = time.time()
    frame_count = 0
    fps = 0

    while True:
        # 获取当前时间
        time1 = time.time()

        # 从摄像头捕获一帧图像
        ret, frame = cap.read()
        if not ret:
            print("视频流结束或发生错误")
            break

        # -------------------------------
        # FPS 计算逻辑（每秒更新一次）
        # -------------------------------
        frame_count += 1
        now_time = time.time()
        if now_time - last_time >= 1.0:
            fps = frame_count / (now_time - last_time)
            last_time = now_time
            frame_count = 0
            print(f"[{time.strftime('%H:%M:%S')}] FPS: {fps:.1f}")
        # -------------------------------
        # 图像预处理：k230 出的就是RGB
        # -------------------------------
        img_rgb = frame  #cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        #img_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        img_nchw = np.array([img_rgb.transpose((2, 0, 1))])  # 转换为 NCHW

        # -------------------------------
        # 执行 AI2D 预处理（resize + pad）
        # -------------------------------
        ai2d_input_tensor = nn.RuntimeTensor.from_numpy(img_nchw)
        ai2d.run(ai2d_input_tensor, kpu_input_tensor)

        # -------------------------------
        # 模型推理
        # -------------------------------
        kpu.run()

        # 获取模型输出
        model_output = kpu.get_output_tensor(0).to_numpy()
        predictions = model_output[0].transpose()  # (8400, 84)

        # -------------------------------
        # 后处理：YOLOv8 输出解析
        # -------------------------------
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

        # -------------------------------
        # 边界框格式转换 (xywh → xyxy)
        # -------------------------------
        boxes_xy = boxes[:, :2] - boxes[:, 2:4] / 2
        boxes_xy = boxes_xy / ratio
        boxes_wh = boxes[:, 2:4] / ratio
        boxes_xy2 = boxes_xy + boxes_wh
        boxes = np.concatenate([boxes_xy, boxes_xy2], axis=1).astype(np.float32)
        scores = scores.astype(np.float32)

        # -------------------------------
        # 执行 NMS（非极大值抑制）
        # -------------------------------
        indices = cv2.dnn.NMSBoxes(boxes.tolist(), scores.tolist(), confidence_thres, iou_thres)

        # -------------------------------
        # 绘制检测结果
        # -------------------------------
        if len(indices) > 0:
            indices = indices.flatten()
            detections = []
            for i in indices:
                box = boxes[i]
                score = scores[i]
                class_id = class_ids[i]
                detections.append({
                    "box": box,
                    "score": score,
                    "class_id": class_id
                })

            # 绘制每个检测到的目标
            for det in detections:
                box = det["box"]
                score = det["score"]
                class_id = det["class_id"]
                x1, y1, x2, y2 = map(int, box)
                label = f"{class_names[class_id]}: {score:.2f}"

                # 绘制矩形框
                cv2.rectangle(img_rgb, (x1, y1), (x2, y2), colors[class_id], 2)

                # 绘制类别与置信度文字
                cv2.putText(img_rgb, label, (x1, y1 - 5),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, colors[class_id], 1)

        # -------------------------------
        # 在图像上绘制 FPS 信息
        # -------------------------------
        cv2.putText(img_rgb, f"FPS: {fps:.1f}", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        # -------------------------------
        # 缩放并显示到屏幕
        # -------------------------------
        resized_img = cv2.resize(img_rgb, (img_size[0], img_size[1]), interpolation=cv2.INTER_LINEAR)
        k230_display.show(resized_img)

        # -------------------------------
        # 键盘输入检测（非阻塞）
        # -------------------------------
        if sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
            line = sys.stdin.readline().strip()
            if line in ('q', 'Q', 'exit'):
                print("收到退出命令")
                break

    # ===============================
    # 七、资源释放与退出
    # ===============================
    cap.release()
    k230_display.deinit()
    print("程序已退出")

# ===============================
# 主程序入口
# ===============================
if __name__ == '__main__':
    detect()

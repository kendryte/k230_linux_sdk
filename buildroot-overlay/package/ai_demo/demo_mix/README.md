# 1.简介

本项目实现了多个demo串烧操作，是手势关键点识别、动态手势识别、人脸姿态角和单目标跟踪的集成。可以作为智能跟踪拍摄车的软件部分实现隔空调整底盘位置，隔空调整相机角度，追踪人脸目标。

# 2.应用使用说明

## 2.1使用帮助

### 2.1.1 手势说明

支持手势如下：

<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/demo_mix/gesture.jpg" alt="手势关键点类别" width="50%" height="50%"/>

one手势进入动态手势识别，love手势退出；yeah手势进入姿态角调整，会选择距离屏幕中心点最近的点调整，love手势退出；three手势进入单目标追踪，会选择距离屏幕中心点最近的人脸进行跟踪。

### 2.1.2 快速启动

执行命令：

```shell
# 准备kmodel列表：hand_det.kmodel、handkp_det.kmodel、gesture.kmodel、face_detection_320.kmodel、face_pose.kmodel、cropped_test127.kmodel、nanotrack_backbone_sim.kmodel、nanotracker_head_calib_k230.kmodel
#准备文件：shang.bin、xia.bin、zuo.bin、you.bin、demo_mix.elf
#视频流推理（demo_mix.sh）
./demo_mix.elf
```


# K230 Linux SDK

>This document uses the k230d_canmv_defconfig configuration as an example. If you use other configurations, replace k230d_canmv_defconfig with correct names. All the configurations supported by this sdk,  can be found in  buildroot-overlay/configs directory.

## Install toolchain and dependencies

download  Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.2.tar.gz toolchan from  `https://www.xrvm.cn/community/download?id=4433353576298909696`
uncompress the toolchain to the /opt/toolchain(Refer command):

```bash
mkdir -p /opt/toolchain;
tar -zxvf Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.2.tar.gz -C /opt/toolchain;
```

install dependencies(Refer command):

```bash
apt-get install -y   git sed make binutils build-essential diffutils gcc  g++ bash patch gzip \
        bzip2 perl  tar cpio unzip rsync file  bc findutils wget  libncurses-dev python3  \
        libssl-dev gawk cmake bison flex  bash-completion parted curl  xz-utils
```

>k230d_canmv_ilp32_defconfig required  ubuntu 22.04 or 24.04 and install rv64ilp32 toolchain(refer command):
>
>`wget -c https://github.com/ruyisdk/riscv-gnu-toolchain-rv64ilp32/releases/download/2024.06.25/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25-nightly.tar.gz;`
>
>`mkdir -p /opt/toolchain/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25/;`
》
>`tar -xvf riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25-nightly.tar.gz   -C /opt/toolchain/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25/;`

## build

```bash

make CONF=k230d_canmv_defconfig #build k230d canmv image (kernel and rootfs both 64bit);
# make CONF=k230d_canmv_defconfig  BR2_PRIMARY_SITE=https://kendryte-download.canaan-creative.com/k230/downloads/dl/ #download package from kendryte
# make CONF=k230_canmv_defconfig # build k230 canmv image
# make CONF=k230d_canmv_ilp32_defconfig  #build k230d canmv 32bit rootfs;
# make help # view help
```

>[BR2_PRIMARY_SIT configuration primary download site]("https://bootlin.com/pub/conferences/2011/elce/using-buildroot-real-project/using-buildroot-real-project.pdf") for example: `make CONF=k230d_canmv_defconfig  BR2_PRIMARY_SITE=https://kendryte-download.canaan-creative.com/k230/downloads/dl/`

## output

```bash
output/k230d_canmv_defconfig/images/sysimage-sdcard.img.gz
```

>Note that k230d_canmv_defconfig is an example and needs to be replaced with the correct name
>uncompress file , [burn to tf card]("https://gitee.com/kendryte/k230_docs/blob/main/zh/01_software/board/K230_SDK_%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E.md#51-sd%E5%8D%A1%E9%95%9C%E5%83%8F%E7%83%A7%E5%BD%95"),insert  tf to device, poweron device.

## uboot

```bash
make uboot-rebuild #rebuild uboot
make uboot-dirclean #uboot clean
#uboot directory description
buildroot-overlay/boot/uboot/u-boot-2022.10-overlay/  #uboot overlay code
output/k230d_canmv_defconfig/build/uboot-2022.10/ #uboot full code,uboot build dir
```

## linux

```bash
make linux-menuconfig #modify configuration
make linux-savedefconfig #save to defconfig

make linux-rebuild  #rebuild linux
make linux-dirclean #linux clean
```

## buildroot

```bash
make menuconfig #buildroot(sdk) modify configuration
make savedefconfig #buildroot(sdk) savedefconfig
```

## debian ubuntu

```bash
make debian #generate debian distribution image
make ubuntu #generate ubuntu distribution image
```

>For more information about Debian, please refer to [<<K230 debian 系统指南>>](https://kvftsfijpo.feishu.cn/drive/folder/ENyifLFdJl3cQ7du5RDcc6SDnFc?from=from_copylink)

## ai_demo

The AI-related demos currently integrated are as follows:

| name    | 说明                                                               |
|------------------------|--------------------------------------------------------------------------|
| anomaly_det            | 异常检测                                                                 |
| bytetrack              | ByteTrack（一种多目标跟踪算法）                                           |
| crosswalk_detect       | 斑马线检测                                                                 |
| demo_mix               | 演示混合                                                                 |
| dynamic_gesture        | 动态手势识别                                                              |
| eye_gaze               | 视线追踪/眼动检测                                                          |
| face_alignment         | 人脸对齐                                                                 |
| face_detection         | 人脸检测                                                                 |
| face_emotion           | 面部情绪识别                                                              |
| face_gender            | 人脸性别识别                                                              |
| face_glasses           | 人脸眼镜检测                        |
| face_landmark          | 人脸关键点检测                                                             |
| face_mask              | 口罩检测                                                                 |
| face_mesh              | 人脸网格                                                                 |
| face_parse             | 人脸解析                                                                 |
| face_pose              | 人脸姿态检测                                                              |
| face_verification      | 人脸验证                                                                 |
| falldown_detect        | 跌倒检测                                                                 |
| finger_guessing        | 猜拳/手势识别（可能特指手指游戏）                                            |
| fitness                | 健身/运动检测                                                              |
| head_detection         | 头部检测                                                                 |
| helmet_detect          | 头盔检测                                                                 |
| kws                    | 关键词唤醒                                      |
| licence_det            | 车牌检测                                                                 |
| licence_det_rec        | 车牌检测与识别                                                           |
| llamac                 | 大语言模型 |
| object_detect_yolov8   | YOLOv8目标检测                                                           |
| ocr                    | 光学字符识别                          |
| person_attr            | 人体属性识别                                                             |
| person_detect          | 人体检测                                                                 |
| person_distance        | 人体距离检测                                                             |
| pose_detect            | 姿态检测/人体姿势检测                                                     |
| pphumanseg             | 飞桨（PaddlePaddle）人体分割模型                                          |
| puzzle_game            | 拼图游戏                                                                 |
| segment_yolov8n        | YOLOv8n语义分割                                                          |
| smoke_detect           | 烟雾检测                                                                 |
| space_resize           | 空间尺寸调整                                                             |
| sq_hand_det            | 手掌检测                  |
| sq_handkp_class        | 手掌关键点分类                                     |
| sq_handkp_det          | 手掌关键点检测                                                           |
| sq_handkp_flower       | 手掌关键点（花朵相关场景）检测                                             |
| sq_handkp_ocr          | 手掌关键点OCR识别                                                         |
| sq_handreco            | 手掌识别                                     |
| traffic_light_detect   | 交通信号灯检测                                                           |
| tts_zh                 | 中文语音合成                                         |
| vehicle_attr           | 车辆属性识别                                                             |
| virtual_keyboard       | 虚拟键盘                                                                 |
| yolop_lane_seg         | YOLOP车道线分割                                 |

>
> - demo source code directory ：buildroot-overlay/package/ai_demo
>
>   ```bash
>   ls buildroot-overlay/package/ai_demo; #can see all ai demo
>   ls buildroot-overlay/package/ai_demo/face_detection/;#face_detection code
>   ```
>
> - The compilation method for a single demo (taking face_detection as an example):
>
>   ```bash
>   make face_detection-dirclean;make face_detection;#rebuild face_detection demo
>   ```
>
> - By default, only the (face_detection) demo is enabled. If other demos are needed, please enable the corresponding demo through the menu:
>
>   ```bash
>   make menuconfig;#Target packages  --->canaan package  ---> AI  --->
>   [*] face detection
>   [ ] demo mix
>   [ ] bytetrack
>    .............
>   [ ] face_mesh
>   [ ] face_parse
>   [ ] face_pose
>   [ ] face_verification
>   ```
>
> - If you need to experience the demo, please enter the corresponding directory under /root/app and run the relevant demo
>
>   ```shell
>   [root@canaan ~ ]#cd /root/app/face_detection/
>   [root@canaan ~/app/face_detection ]ls
>   face_detect_image.sh       face_detection_320.kmodel
>   face_detect_isp.sh         face_detection_640.kmodel
>   face_detection.elf
>   [root@canaan ~/app/face_detection ]#./face_detect_isp.sh
>   ```
>
> - For more information about ai demo, please refer to [K230 AI Demo使用指南](https://www.kendryte.com/k230_linux/dev/zh/01_software/K230_AI_Demo%E4%BD%BF%E7%94%A8%E6%8C%87%E5%8D%97.html)
>

for more ,please refer to [<<K230 Linux SDK 文档>>](https://www.kendryte.com/k230_linux/dev/index.html)

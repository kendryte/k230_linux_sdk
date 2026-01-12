# K230 Linux SDK

>This document uses the k230d_canmv_defconfig configuration as an example. If you use other configurations, replace k230d_canmv_defconfig with correct names. All the configurations supported by this sdk,  can be found in  buildroot-overlay/configs directory.

## Install toolchain and dependencies

```bash
sudo make toolchain_and_depend
#The above command will install GCC and the SDK's dependent packages. For details, refer to the tools/install_toolchain_and_depend.sh file
```

> - This SDK has been verified on Ubuntu 22.04 and 24.04, and other versions may have compilation issues.
> - Only the k230d_canmv_ilp32_defconfig requires additional manual installation(see command below) of the rv64ilp32 toolchain:
>
>   ```bash
>   #only k230d_canmv_ilp32_defconfig need
>   wget -c https://github.com/ruyisdk/riscv-gnu-toolchain-rv64ilp32/releases/download/2024.06.25/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25-nightly.tar.gz;
>   mkdir -p /opt/toolchain/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25/;
>   tar -xvf riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25-nightly.tar.gz   -C /opt/toolchain/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25/;
>   #only k230d_canmv_ilp32_defconfig need
>   ```

## build

```bash

make CONF=k230d_canmv_defconfig #build k230d canmv image (kernel and rootfs both 64bit);
# make CONF=k230_canmv_defconfig # build k230 canmv image
# make CONF=k230d_canmv_ilp32_defconfig  #build k230d canmv 32bit rootfs;
# make help # view help
```

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

| name                   | 说明                                                                 |
|------------------------|----------------------------------------------------------------------|
| anomaly_det            | 异常检测                                                             |
| bytetrack              | ByteTrack（一种多目标跟踪算法）                                      |
| crosswalk_detect       | 斑马线检测                                                           |
| demo_mix               | 演示混合                                                             |
| dynamic_gesture        | 动态手势识别                                                         |
| eye_gaze               | 视线追踪/眼动检测                                                    |
| face_alignment         | 人脸对齐                                                             |
| face_detection         | 人脸检测                                                             |
| face_emotion           | 面部情绪识别                                                         |
| face_gender            | 人脸性别识别                                                         |
| face_glasses           | 人脸眼镜检测                                                         |
| face_landmark          | 人脸关键点检测                                                       |
| face_mask              | 口罩检测                                                             |
| face_mesh              | 人脸网格                                                             |
| face_parse             | 人脸解析                                                             |
| face_pose              | 人脸姿态检测                                                         |
| face_verification      | 人脸验证                                                             |
| falldown_detect        | 跌倒检测                                                             |
| finger_guessing        | 猜拳/手势识别（可能特指手指游戏）                                    |
| fitness                | 健身/运动检测                                                        |
| head_detection         | 头部检测                                                             |
| helmet_detect          | 头盔检测                                                             |
| kws                    | 关键词唤醒                                                           |
| licence_det            | 车牌检测                                                             |
| licence_det_rec        | 车牌检测与识别                                                       |
| llamac                 | 大语言模型                                                           |
| object_detect_yolov8   | YOLOv8 目标检测                                                      |
| ocr                    | 光学字符识别                                                         |
| person_attr            | 人体属性识别                                                         |
| person_detect          | 人体检测                                                             |
| person_distance        | 人体距离检测                                                         |
| pose_detect            | 姿态检测/人体姿势检测                                                |
| pphumanseg             | 飞桨（PaddlePaddle）人体分割模型                                     |
| puzzle_game            | 拼图游戏                                                             |
| segment_yolov8n        | YOLOv8n 语义分割                                                     |
| smoke_detect           | 烟雾检测                                                             |
| space_resize           | 空间尺寸调整                                                         |
| sq_hand_det            | 手掌检测                                                             |
| sq_handkp_class        | 手掌关键点分类                                                       |
| sq_handkp_det          | 手掌关键点检测                                                       |
| sq_handkp_flower       | 手掌关键点（花朵相关场景）检测                                       |
| sq_handkp_ocr          | 手掌关键点 OCR 识别                                                  |
| sq_handreco            | 手掌识别                                                             |
| traffic_light_detect   | 交通信号灯检测                                                       |
| tts_zh                 | 中文语音合成                                                         |
| vehicle_attr           | 车辆属性识别                                                         |
| virtual_keyboard       | 虚拟键盘                                                             |
| yolop_lane_seg         | YOLOP 车道线分割                                                     |

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
## FQA

### how to enable gdb

enable BR2_PACKAGE_GDB and BR2_PACKAGE_GDB_DEBUGGER

```bash
make menuconfig
# Target packages → Debugging, profiling and benchmark->gdb->full debugger
make
```

### How to quickly update the kernel and device tree

Just replace the corresponding files in the boot directory.

```bash
[root@canaan ~ ]#ls -lh /boot/
total 17M
-rw-r--r--    1 root     root       17.9M Jun 20  2025 Image    #kernel
-rw-r--r--    1 root     root      264.4K Jun 20  2025 fw_jump_add_uboot_head.bin
lrwxrwxrwx    1 root     root          24 Jun 20  2025 k.dtb -> k230-canmv-lckfb-lcd.dtb
-rwxr-xr-x    1 root     root       53.8K Jun 20  2025 k230-canmv-lckfb-lcd.dtb  #lcd dtb
drwx------    2 root     root       12.0K Jun 20  2025 lost+found
-rwxr-xr-x    1 root     root      172.2K Jun 20  2025 nuttx-7000000-uart2.bin
[root@canaan ~ ]#
```

### How to configure WiFi

- use sta.sh script

```bash
sta.sh wlan0 H3C_wjx 12345678
```

- save wifissid and passwd to env

```bash
fw_setenv wlanssid  H3C_wjx; fw_setenv wlanpass 12345678;reboot;
```

### How to test DDR

Use the following command to generate a DDR test image. This image will automatically execute the DDR stability test program, which will run continuously. There will be constant output on the serial port, and if it fails, an error message will be printed on the serial port.

```bash
make ddr_test_img_128 #build ddr 128MB test img,128 can be 512/1024/2048/128"
```

for more ,please refer to:

[K230 Linux SDK 文档](https://www.kendryte.com/zh/sdkResource/230linux)

[k230问答社区](https://www.kendryte.com/answer/)

[k230产品中心](https://www.kendryte.com/zh/products)

[k230文档](https://www.kendryte.com/zh/document)

[K230 debian 系统指南](https://kvftsfijpo.feishu.cn/drive/folder/ENyifLFdJl3cQ7du5RDcc6SDnFc?from=from_copylink)

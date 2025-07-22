# Linux 部署勘智在线训练平台模型

## 1. 概述

`cloudplat_deploy_code_linux` 封装了勘智在线训练平台模型部署的代码，用户需要在`k230_linux_sdk`下编译可执行文件部署训练平台得到的模型。

## 2. 源码说明

`cloudplat_deploy_code_linux` 代码实现了训练平台支持的图像分类、目标检测、语义分割、OCR检测、OCR识别、双模型任务OCR检测+识别、度量学习（图像特征化）、多标签分类共计8个任务。代码封装了模型推理、预处理工具方法、配置文件解析、结果绘制的公共部分，放在`common_files`目录下，其他目录分别存放对应任务的推理代码。

### 3. 代码结构

下面是代码文件的说明：

```shell
cloudplat_deploy_code_linux
├── common_files
├── classification
├── detection
├── segmentation
├── ocr_detection
├── ocr_recognition
├── ocr
├── metric_learning
├── multilabel_classification
├── utils
│    │- SourceHanSansSC-Normal-Min.ttf # 字体文件
├── CMakeLists.txt
└── build.sh
```

## 4. 编译说明

### 4.1 环境搭建并编译固件

使用如下命令搭建linux_sdk环境，并完成对应开发板的固件编译：

```shell
#下载k230_linux_sdk
git clone https://github.com/kendryte/k230_linux_sdk.git

#下载编译工具链
wget https://kendryte-download.canaan-creative.com/k230/downloads/dl/gcc/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.2-20250410.tar.gz

# 解压工具链到/opt/toolchain
mkdir -p /opt/toolchain;
tar -zxvf Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.2-20250410.tar.gz -C /opt/toolchain;

#安装依赖
apt-get install -y   git sed make binutils build-essential diffutils gcc  g++ bash patch gzip \
        bzip2 perl  tar cpio unzip rsync file  bc findutils wget  libncurses-dev python3  \
        libssl-dev gawk cmake bison flex  bash-completion parted curl

# 选择对应的开发板配置文件，编译固件
make CONF=k230_canmv_lckfb_defconfig  BR2_PRIMARY_SITE=https://kendryte-download.canaan-creative.com/k230/downloads/dl/
```

固件编译成功后，将固件烧录到开发板上。

### 4.2 编译源码

进入`k230_linux_sdk/buildroot-overlay/package/`目录，**编译源码**：

```shell
# 进入目录
cd cloudplat_deploy_code_linux

# 编译文件,会在k230_bin目录下得到所有的任务编译elf文件
./build.sh

# 如果只想编译某一个任务的部署文件，可以使用./build.sh <任务名>
./build.sh classification
./build.sh detection
...

```

编译产物在 `k230_bin` 目录下。

### 4.3 上板部署

将得到的`elf文件`、字体文件和勘智训练平台得到的`kmodel`、`deploy_config.json`以及测试图片拷贝到开发板上的某一目录中，运行命令：

```shell
# 分类-视频推理，输入`q`回车退出视频推理
./classification.elf deploy_config.json None 0

# 分类-图片推理
./classification.elf deploy_config.json test.jpg 0

# 检测-视频推理，输入`q`回车退出视频推理
./detection.elf deploy_config.json None 0

# 检测-图片推理
./detection.elf deploy_config.json test.jpg 0

# 语义分割-视频推理，输入`q`回车退出视频推理
./segmentation.elf deploy_config.json None 0

# 语义分割-图片推理
./segmentation.elf deploy_config.json test.jpg 0

# OCR检测-视频推理，输入`q`回车退出视频推理
./ocr_detection.elf deploy_config.json None 0

# OCR检测-图片推理
./ocr_detection.elf deploy_config.json test.jpg 0

# OCR识别-图片推理，该任务只支持图片推理
./ocr_recognition.elf deploy_config.json test.jpg 0

# OCR-视频推理，输入`q`回车退出视频推理
./ocr.elf ocrdet_deploy_config.json ocrrec_deploy_config.json None 0

# OCR-图片推理
./ocr.elf ocrdet_deploy_config.json ocrrec_deploy_config.json test.jpg 0

# 度量学习-视频推理，输入`q`回车退出视频推理
./metric_learning.elf deploy_config.json None 0

# 度量学习-图片推理
./metric_learning.elf deploy_config.json test.jpg 0

# 多标签分类-视频推理，输入`q`回车退出视频推理
./multilabel_classification.elf deploy_config.json None 0

# 多标签分类-图片推理
./multilabel_classification.elf deploy_config.json test.jpg 0
```

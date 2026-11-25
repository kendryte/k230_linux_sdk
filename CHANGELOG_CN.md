# K230 Linux SDK 更新日志（Changelog）

[English](CHANGELOG.md) | 简体中文

本文档记录 K230 Linux SDK 自 v0.6.5 起各发布版本的主要变更。

* 代码仓库：<https://github.com/kendryte/k230_linux_sdk>
* 镜像下载：<https://download.kendryte.com/k230/release/linux_sdk_images/daily_build/>

## 版本目录

* [v1.5](#k230-linux-sdk-v15-发布说明)
* [v1.4](#k230-linux-sdk-v14-发布说明)
* [v1.3](#k230-linux-sdk-v13-发布说明)
* [v1.2](#k230-linux-sdk-v12-发布说明)
* [v1.1](#k230-linux-sdk-v11-发布说明)
* [v1.0](#k230-linux-sdk-v10-发布说明)
* [v0.6.9](#v069)
* [v0.6.5](#v065)

---

## K230 Linux SDK v1.5 发布说明

发布日期：待定

**K230 Linux SDK v1.5** 正式发布。本版本新增 DongshanPi CanMV V3 板卡，K230 EVB 支持 SPI-NAND 启动；Camera 方面新增 libcamera 支持和手动 AE 控制；重构了 PUFcc 加密驱动并加强 OTP 保护；新增 PDM 麦克风驱动；还修复了 DDR 容量识别、启动流程、MMC 别名等稳定性问题，并支持在 Ubuntu 26.04 主机上编译。

### 🚀 主要亮点

* **新增板卡：** 支持 DongshanPi CanMV V3（`dongshanpi_v3_defconfig`）。
* **SPI-NAND 启动：** K230 EVB 支持从 SPI-NAND（W25N01GW）启动，可以生成 NAND 镜像。
* **libcamera：** 新增 vvcam libcamera pipeline 和 `libcamera-drm` / `libcamera-grab` / `libcamera-hold` 示例。
* **加密驱动重构：** PUFcc 的 hash/cipher 共用一个 crypto engine，并串行访问硬件；新增 AES/SM4 多种模式、SM3/HMAC、RSA（ECP 硬件加速）。
* **OTP 安全加强：** U-Boot 用 post-mask（psmsk）保护 OTP，并禁止访问 patch 区和启动相关的 OTP。
* **音频：** 新增 PDM 麦克风采集驱动，codec 新增 ADC 数字音量控制。

---

### 📦 组件更新

#### 1. 板级支持

* **DongshanPi CanMV V3：** 新增独立的 Linux/U-Boot 设备树，支持 GC2093（CSI2，I2C4）、480x640 双 lane ST7701 屏和 RTL8733BS SDIO Wi-Fi。原 DongshanPi 配置保持不变。
* **K230 EVB：** 新增 W25N01GW SPI-NAND 驱动、分区和镜像生成；EVB 的 NAND 配置合并进通用 EVB 配置。
* **GT6700：** 打开 UART1/UART2，更新 `S00resizemmc`、`S31canaan_isp`、`S99showversion` 启动脚本。
* **K230D Junroc AI Cam：** rootfs 分区改为 `mmcblk1p2`，新增中文 README。
* **MRT：** 新增 RTL8189FS 厂商驱动和射频测试文档。

#### 2. U-Boot

* **DDR：** 修复 DDR 容量识别错误导致的启动卡死。
* **启动流程：** 大核复位、等待 WFI 移到跳转 U-Boot 之前；SPL 复位路径改为进入 USB 烧录模式，之前会一直循环。
* **OTP：** 只在读写 OTP 期间临时解除 psmsk 屏蔽，其余时间保持锁定，并删掉了多余的 PMP OTP 保护；禁止访问 patch 区和启动相关的 OTP。
* **启动介质：** 恢复 k230_canmv 的 SDIO1 启动模式覆盖，降低 eMMC 最高频率。

#### 3. Linux 内核与驱动

* **加密：** 重构 PUFcc crypto 驱动，修复 SHA-256 多个调用方共享 DMA buffer 导致的并发问题；新增 AES/SM4 ECB/CBC/CTR/OFB/CFB、SM3、HMAC、RSA，并附带并发测试和性能测试程序。
* **显示：** DRM VO 的 vblank 改为在垂直前肩（VFP）由 hrtimer 上报，帧地址直接在 atomic commit 里写入，DSI 时序改为按面板参数计算。
* **音频：** 新增 PDM 麦克风采集（ASoC），inno_k230 codec 新增 ADC 数字音量控制。
* **设备树：**
  * K230D 系列交换 mmc0/mmc1 别名，和硬件编号保持一致。
  * 新增默认 IO0 pinctrl。
  * 修复 MMC probe 告警。
  * 补全 C908 全部 165 个原始 PMU 事件（#43）。
  * BPI-CanMV-K230D-Zero 新增 i2c4 alias。

#### 4. 多媒体（Camera / ISP / Display）

* **libcamera：** 新增 vvcam libcamera pipeline 和示例程序，并修复 libcamera 编译错误。
* **ISP：** 新增手动 AE（曝光）控制接口。
* **GC2093：** 新增镜像/翻转参数和 1080p@15fps 模式。
* **问题修复：**
  * BG3P 格式 RGB 输出异常
  * CSI2 GC2093 首次运行闪烁（MCLK 问题）
  * BPI-CanMV-K230D-Zero 运行 IMX335 失败

#### 5. 构建系统与工具

* **构建：**
  * 所有板卡默认打开 `BR2_PER_PACKAGE_DIRECTORIES`，并修正 bluez_inc、vo_init 等软件包的依赖声明。
  * 修复 Ubuntu 26.04 主机上的编译错误（dbus、dbus-broker、dbus-cxx、dbus-glib、m4）。
  * python-k230 增加 lvgl 版本检查；`make clean` 更安全。
* **工具：**
  * `make list-def` 改为独立脚本 `tools/list_def.sh`，会显示当前 uboot/linux/busybox 配置、DTS 和源码目录。
  * 优化 `get_fast_url.sh` 下载源选择。
  * 修复 DDR 测试镜像脚本里的 env 路径。
* **发行版：** cp312 wheel 构建脚本改为自动解析 Ubuntu python3.12 deb 版本，修复上游删除旧版本后 `make ubuntu` 失败（Error 8）的问题。
* **CI / 网站：**
  * 修复 GitHub Actions 容器权限问题，GitLab CI 新增 EVB、GT6700 板。
  * 下载页接入百度统计，历史版本可以展开查看摘要和修改点。
  * `.last_conf` 只在 root 用户下 chmod，README 里安装工具链的命令改为用 sudo。

---

### ⚠️ 升级注意事项

* K230D 系列交换了 mmc0/mmc1 别名，rootfs 设备名可能会变（如 `mmcblk0p2` 变成 `mmcblk1p2`）。使用自定义 bootargs 的用户请检查。
* PUFcc 加密驱动重构后 Kconfig 选项变了，使用自定义内核配置的用户请重新检查加密相关选项。

**完整对比**：https://github.com/kendryte/k230_linux_sdk/compare/v1.4...v1.5

[↑ 回到目录](#版本目录)

---

## K230 Linux SDK v1.4 发布说明

发布日期：2026-09-06

v1.4 和 v1.3 指向同一个提交，只是版本号同步，代码没有变化，具体内容见 [v1.3](#k230-linux-sdk-v13-发布说明)。

[↑ 回到目录](#版本目录)

---

## K230 Linux SDK v1.3 发布说明

发布日期：2026-09-06

本版本新增 CanMV MRT 板卡，支持 MCM 多摄像头（OV13850 4K），新增 HDMI/LCD 自动识别、DRM 硬件旋转和触摸校准，提供带 VPU v4l2m2m 的 k230-ffmpeg deb 包，加强了 DMA 地址检查，并改进了音频 codec 通路。

### 🚀 主要亮点

* **新增板卡：** K230 CanMV MRT（`k230_canmv_mrt_defconfig`）。
* **多摄像头：** 支持 MCM，可接 OV13850 4K；默认使用 CSI2 DTS；新增 BF3238、GC2053、GC2093 多分辨率 ISP 配置。
* **显示：** U-Boot 支持 HDMI/LCD 自动识别（01Studio 等板卡），DRM 支持硬件旋转和触摸校准。
* **多媒体发行包：** 提供带 VPU v4l2m2m 硬件编解码的 k230-ffmpeg deb；Debian/Ubuntu 统一使用同一个 isp_media_server。
* **Python 生态：** 新增 nncaseruntime 绑定、LVGL 旋转绑定，支持构建 cp312 wheel，并改进 LVGL Python 绑定代码生成工具。

---

### 📦 组件更新

#### 1. 板级支持

* 新增 K230 CanMV MRT 板（LPDDR4-2667、U-Boot/内核 DTS）。
* Junroc 板：串口改为 UART2，修正 bank 电压；gc2093 新增 mclk1 pinctrl。
* LCKFB：新增 DTS 软链接，修复 LT9611 HDMI 检测。

#### 2. Linux 内核与驱动

* **DMA 安全：** 加强 DMA 地址检查，PDMA 新增寄存器范围检查。
* **SPI：** dw-mmio 改用 `platform_get_irq_optional`，修复 K230 上的中断获取问题。
* **音频：** codec 通路可在运行时切换（SDI1 采集），支持 4 通道双麦克风采集，修复 24-bit 问题。
* **中断：** sifive-plic 新增设置优先级的 API。
* **设备树：** 传感器 DTS 新增 mclk 设置。

#### 3. 多媒体（Camera / ISP / Display）

* 支持 MCM 多摄像头，可接 OV13850 4K；vvcam_video 支持 `mcm_mask` 参数。
* DRM 支持硬件旋转和触摸校准。
* 修复：
  * v4l2-drm 180° 旋转
  * 1280x960 反复启停时崩溃
  * `-sw`/`-sh` 参数错误
  * IMX335 无法运行
* 删除预编译的 isp_media_server ELF 文件，Debian/Ubuntu 统一使用同一版本。

#### 4. AI / Python / LVGL

* 新增 nncaseruntime Python 绑定、LVGL 旋转绑定。
* 改进 LVGL Python 绑定和代码生成工具。
* 新增 yolov8n deb 包。

#### 5. 构建系统与发行版

* SD 卡镜像改用 MBR 分区表，修复 rootfs 扩容问题。
* 新增 `fw_printenv`、`sshpass`；重构“重启进入升级模式”流程。
* 新增 `get_fast_url.sh`，自动选择较快的下载源。
* 下载网站支持中英文；CI 保存镜像前先清理旧目录。
* 修复 mmz 分配失败时的错误处理。

**完整对比**：https://github.com/kendryte/k230_linux_sdk/compare/v1.2...v1.3

[↑ 回到目录](#版本目录)

---

## K230 Linux SDK v1.2 发布说明

发布日期：2026-07-22

### 新特性

* **LVGL Python：** 新增 Canvas（ARGB8888 绘图）、Line（折线）、Menu（多级菜单页）、Animimg（帧动画）四种控件的 pybind11 绑定和示例；重构 LVGL Python API，并整理示例。
* **构建：** 镜像文件名加入 commit id，新增板卡配置。

**完整对比**：https://github.com/kendryte/k230_linux_sdk/compare/v1.1...v1.2

[↑ 回到目录](#版本目录)

---

## K230 Linux SDK v1.1 发布说明

发布日期：2026-07-10

本版本重点完善了 Python 生态，新增显示、V4L2-DRM、LVGL、Adafruit Blinka 等 Python 绑定，加入 WebRTC 源码和示例，并新增 Junroc AI Cam 板卡。

### 🚀 主要亮点

* **新增板卡：** K230D CanMV Junroc AI Cam（`k230d_canmv_junroc_ai_cam_defconfig`）。
* **Python 生态：**
  * 默认启用 `python-k230` 软件包。
  * 新增 `k230_display`、`k230_v4l2_drm`（支持 OSD）、LVGL（pybind11）Python 绑定。
  * 支持 Adafruit Blinka 和 IOMUX Python 配置。
* **WebRTC：** 加入 WebRTC 源码和示例，支持 H.265，改进 Opus 音频和 MQTT 信令的可靠性。

### 新特性

* **Camera：** 支持场景切换、镜像/翻转。
* **视频编解码：** FFmpeg 支持 ARGB 输入编解码，MJPEG 编码器支持设置 JPEG 质量。
* **显示：** VO 支持 GDMA 旋转；优化 K230 显示驱动；U-Boot 支持 01Studio 的 ili9881 800x1280 LCD。
* **Python 示例：** 新增 V4L2-DRM 版 YOLO 示例，新增 I2C/UART 示例，`face_detect.py` 移到 `py_demo/ai/`。
* **01Studio：** 支持 LED 和按键。
* **USB：** uvc-gadget 降低 CPU 占用，并给线程命名。
* **其他：** 下载网站改为 download.kendryte.com。

### 问题修复

* 修复传感器镜像/翻转问题。
* 修复关闭显示、停止 vvcam 时的内存泄漏。
* 修复 PWM 问题；修复 01Studio DTS 错误、eMMC 板启动顺序。
* Debian 下 whl 包改为解压到 Python site-packages 安装。
* 修复 LVGL 找不到 pcpp 的问题，以及多个 CI 构建错误。

**完整对比**：https://github.com/kendryte/k230_linux_sdk/compare/v1.0...v1.1

[↑ 回到目录](#版本目录)

---

## K230 Linux SDK v1.0 发布说明

发布日期：2026-04-21

### 🚀 主要亮点

* **nncase 升级到 2.11.0：** AI 示例支持 yolov8/yolo11 姿态估计和 yolo26。
* **USB 摄像头：** uvc-gadget 支持 VPU 编码视频源，新增 UVC+ACM 复合设备脚本 `uvc-gadget-acm.sh`。
* **IOMUX：** 新增 K230 IOMUX 引脚控制驱动和配置工具，驱动改用 pinctrl 通用接口实现。
* **OSD：** nonai2d 支持 OSD，参数可以动态设置，并提供接口说明 README。
* **大模型：** 新增 llama.cpp 软件包。

### 新特性

* **K230D CanMV：** 新增 I2C4、MIPI CSI2 支持，更新默认传感器。
* **01Studio XCAM EVT：** 通过 aic8800_sdio 支持蓝牙，更新引脚配置。
* **Camera：** 传感器支持复位；libmmz 新增 `kd_mpi_get_vvcam_video00()`，用于检测 vvcam 视频设备；新增 modprobe.d 配置，保证 vvcam_isp 比 nonai2d 先加载。
* **RTSP：** 改进 camera_rtsp_demo，优化接口和示例代码。
* **LCKFB：** DDR 改为 2667。
* **构建：** 新增 Ubuntu 构建脚本，更新 Dockerfile。

### 问题修复

* 修复 K230 时钟 composite 设置父时钟和计算频率的问题。
* 修复 OSD 问题。
* 改进 USB gadget 停止流程，增加清理 function 和 config 的步骤。
* 修复 pcpp 编译错误和 VSCode clangd 报错。
* 修复 AI 示例测试问题；build.sh 支持下载 nncase 2.11.0 kmodel。

**完整对比**：https://github.com/kendryte/k230_linux_sdk/compare/v0.6.9...v1.0

[↑ 回到目录](#版本目录)

---

## v0.6.9

发布日期：2026-02-04

* 新增 Debian 镜像 CI 和每日构建 CI，新增 DongshanPi CI。
* 01Studio：调整 MMC 和 HDMI 配置。
* 修复 CI tftp 保存镜像错误等多个问题。

[↑ 回到目录](#版本目录)

---

## v0.6.5

发布日期：2026-01-28

本版本覆盖 2025-04 ~ 2026-01 的开发内容，变更较多。

### 🚀 主要亮点

* **Buildroot 升级到 2025 版本，工具链升级到 3.0.2。**
* **nncase 升级到 2.10。**
* **可以生成 deb 包，Debian 支持 pip。**
* **新增板卡：** GT6700、01Studio eMMC 版（`k230_canmv_01studio_emmc_defconfig`）。

### 新特性

* **Camera：**
  * 新增 IMX335（支持 again/dgain）、GC2093 60fps。
  * GC2093 支持多个 I2C 地址。
* **显示：**
  * 新增 ili9881 800x1280 屏。
  * U-Boot 支持显示开机 logo，logo 参数可配置。
  * 启动时初始化 VO 并设置旋转，解决绿屏问题。
  * 调整 LCKFB 屏幕时序。
* **视频与 RTSP：**
  * 新增 smolrtsp，优化 RTSP 延迟，调整 V4L2 采集 buffer 数量。
  * FFmpeg 支持控制 VPU 解码缩放（FFmpeg 退回到 4.4.4）。
* **USB：** 支持 ADB、MTP、CDC ACM，加入 USB 相关补丁。
* **无线：** 新增 RTL8733BS 驱动，支持 Wi-Fi 复位。
* **AI：** 新增 YOLO 示例、ai2d 示例、云平台部署代码、OpenCV RVV 示例，以及 AI 示例安装脚本。
* **系统：**
  * 新增开机 banner、`fw_printenv`，启动后自动扩展 MMC 分区。
  * 新增 valgrind、ldd，支持 `-fsanitize=address`。
  * K230 新增 AI 相关模块的时钟和电源域支持。
* **调试与测试：** 新增 DDR 测试镜像、NOC QoS 查看工具；DongshanPi 新增 LPDDR4 初始化。
* **CI：** GitHub Actions 支持 fork 仓库运行；每日构建会同步下载目录。

### 问题修复

* 修复显示少一列像素的问题。
* 修复缓冲区溢出（MX3SDK-1761）。
* 修复 HS200 tuning 打印。
* 修复 root 用户编译报错，以及 Debian 构建错误、Debian 缺少 libjpeg.so9 的问题。
* 修复 dnn、zxing-cpp 编译错误；修复 V4L2 运行错误，改进 `v4l2_drm_stop`。
* 更新 K230 音频驱动，更新 LVGL 版本。

**完整对比**：https://github.com/kendryte/k230_linux_sdk/compare/v0.6.4...v0.6.5

[↑ 回到目录](#版本目录)

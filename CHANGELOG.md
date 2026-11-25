# K230 Linux SDK Changelog

English | [简体中文](CHANGELOG_CN.md)

This document records the major changes in each K230 Linux SDK release since v0.6.5.

* Repository: <https://github.com/kendryte/k230_linux_sdk>
* Image download: <https://download.kendryte.com/k230/release/linux_sdk_images/daily_build/>

## Table of Contents

* [v1.5](#k230-linux-sdk-release-notes-v15)
* [v1.4](#k230-linux-sdk-release-notes-v14)
* [v1.3](#k230-linux-sdk-release-notes-v13)
* [v1.2](#k230-linux-sdk-release-notes-v12)
* [v1.1](#k230-linux-sdk-release-notes-v11)
* [v1.0](#k230-linux-sdk-release-notes-v10)
* [v0.6.9](#v069)
* [v0.6.5](#v065)

---

## K230 Linux SDK Release Notes v1.5

Release date: TBD

We are pleased to announce the release of **K230 Linux SDK v1.5**. This release:

* Adds the DongshanPi CanMV V3 board.
* Adds SPI-NAND boot on the K230 EVB.
* Adds libcamera support and manual AE control for cameras.
* Reworks the PUFcc crypto drivers and strengthens OTP protection.
* Adds a PDM microphone driver.
* Fixes stability problems with DDR size detection, the boot flow and MMC aliases.
* Fixes builds on Ubuntu 26.04 hosts.

### 🚀 Key Highlights

* **New Board:** DongshanPi CanMV V3 (`dongshanpi_v3_defconfig`).
* **SPI-NAND Boot:** The K230 EVB can boot from SPI-NAND (W25N01GW), and the build can now produce NAND images.
* **libcamera:** Added a vvcam libcamera pipeline and the `libcamera-drm`, `libcamera-grab` and `libcamera-hold` examples.
* **Crypto Driver Rework:** The PUFcc hash and cipher drivers now share one crypto engine that serializes hardware access. Added AES/SM4 modes, SM3/HMAC and ECP-accelerated RSA.
* **OTP Security:** U-Boot protects OTP with the post-mask (psmsk). Access to the patch area and to boot-related OTP is no longer allowed.
* **Audio:** Added a PDM microphone capture driver and ADC digital volume control in the codec.

---

### 📦 Component Updates

#### 1. Board Support

* **DongshanPi CanMV V3:**
  * Added dedicated Linux and U-Boot device trees.
  * Supports GC2093 (CSI2 on I2C4), a two-lane 480x640 ST7701 panel and RTL8733BS SDIO Wi-Fi.
  * The original DongshanPi configuration is unchanged.
* **K230 EVB:** Added the W25N01GW SPI-NAND driver, its partition layout and NAND image generation. Merged the EVB NAND configuration into the common EVB configuration.
* **GT6700:** Enabled UART1 and UART2. Updated the `S00resizemmc`, `S31canaan_isp` and `S99showversion` init scripts.
* **K230D Junroc AI Cam:** Changed the rootfs partition to `mmcblk1p2` and added a Chinese README.
* **MRT:** Added the RTL8189FS vendor driver and RF test documents.

#### 2. U-Boot

* **DDR:** Fixed a DDR size detection error that could hang the board during boot.
* **Boot Flow:**
  * The big-core reset and WFI wait now happen before the jump to U-Boot.
  * The SPL reset path now enters USB burn mode instead of looping.
* **OTP:**
  * The psmsk is unmasked only while OTP is being read or written and stays locked the rest of the time. The redundant PMP OTP guard is removed.
  * Access to the patch area and to boot-related OTP is no longer allowed.
* **Boot Media:** Restored the SDIO1 boot mode override for k230_canmv and lowered the maximum eMMC frequency.

#### 3. Linux Kernel & Drivers

* **Crypto:**
  * Reworked the PUFcc crypto drivers. Fixed a race in which concurrent SHA-256 callers shared one DMA buffer.
  * Added AES/SM4 ECB/CBC/CTR/OFB/CFB, SM3, HMAC and RSA.
  * Added concurrency tests and benchmarks.
* **Display:**
  * The DRM VO driver now reports vblank from the vertical front porch (VFP) with an hrtimer.
  * The frame base is written directly in the atomic commit.
  * DSI timings are now calculated from the panel parameters.
* **Audio:** Added PDM microphone capture (ASoC) and ADC digital volume control in the inno_k230 codec.
* **Device Tree:**
  * Swapped the mmc0/mmc1 aliases on K230D boards to match the hardware numbering.
  * Added a default IO0 pinctrl.
  * Fixed MMC probe warnings.
  * Listed all 165 raw PMU events of the C908 (#43).
  * Added an i2c4 alias for BPI-CanMV-K230D-Zero.

#### 4. Multimedia (Camera / ISP / Display)

* **libcamera:** Added the vvcam libcamera pipeline and examples, and fixed a libcamera build error.
* **ISP:** Added manual AE (exposure) control.
* **GC2093:** Added mirror/flip parameters and a 1080p@15fps mode.
* **Fixes:**
  * Wrong RGB output for the BG3P format.
  * GC2093 on CSI2 flickered on first run (an MCLK problem).
  * IMX335 failed to run on BPI-CanMV-K230D-Zero.

#### 5. Build System & Tools

* **Build:**
  * Enabled `BR2_PER_PACKAGE_DIRECTORIES` for all boards and fixed dependency declarations in packages such as bluez_inc and vo_init.
  * Fixed build errors on Ubuntu 26.04 hosts (dbus, dbus-broker, dbus-cxx, dbus-glib, m4).
  * Added an lvgl version check for python-k230, and made `make clean` safer.
* **Tools:**
  * `make list-def` now runs a standalone script, `tools/list_def.sh`. It also shows the current uboot/linux/busybox configs, the DTS and the source directories.
  * `get_fast_url.sh` picks a download mirror faster.
  * Fixed the env path in the DDR test image script.
* **Distribution:** The cp312 wheel build script now looks up the Ubuntu python3.12 deb version at build time. This fixes `make ubuntu` failing with Error 8 after Ubuntu removed the older version.
* **CI / Website:**
  * Fixed a permission problem in the GitHub Actions container. Added the EVB and GT6700 boards to GitLab CI.
  * Added Baidu Analytics to the download page. Rows in the release history can be expanded to show a summary and changes.
  * `.last_conf` is chmod'ed only when running as root. The toolchain install commands in the README now use sudo.

---

### ⚠️ Upgrade Notes

* The mmc0/mmc1 aliases are swapped on K230D boards, so the rootfs device name may change (for example, `mmcblk0p2` becomes `mmcblk1p2`). If you use custom bootargs, check them.
* The PUFcc crypto rework changes the Kconfig options. If you use a custom kernel config, check the crypto options again.

**Full Changelog**: https://github.com/kendryte/k230_linux_sdk/compare/v1.4...v1.5

[↑ Back to top](#table-of-contents)

---

## K230 Linux SDK Release Notes v1.4

Release date: 2026-09-06

v1.4 points to the same commit as v1.3. It only updates the version number and has no code changes. See [v1.3](#k230-linux-sdk-release-notes-v13) for details.

[↑ Back to top](#table-of-contents)

---

## K230 Linux SDK Release Notes v1.3

Release date: 2026-09-06

This release:

* Adds the CanMV MRT board.
* Supports multiple cameras (MCM), including OV13850 at 4K.
* Detects HDMI or LCD automatically, and adds DRM hardware rotation and touch calibration.
* Provides a k230-ffmpeg deb with VPU v4l2m2m support.
* Checks DMA addresses more strictly.
* Improves the audio codec routing.

### 🚀 Key Highlights

* **New Board:** K230 CanMV MRT (`k230_canmv_mrt_defconfig`).
* **Multi-Camera:**
  * MCM support with OV13850 at 4K.
  * CSI2 is now the default in the DTS.
  * New ISP configs for BF3238, GC2053 and GC2093 at several resolutions.
* **Display:** U-Boot detects HDMI or LCD automatically on 01Studio and other boards. DRM supports hardware rotation and touch calibration.
* **Multimedia Packages:** A k230-ffmpeg deb with VPU v4l2m2m hardware encoding and decoding. Debian and Ubuntu now use the same isp_media_server.
* **Python:** New nncaseruntime and LVGL rotation bindings, cp312 wheel builds, and an improved code generator for the LVGL Python bindings.

---

### 📦 Component Updates

#### 1. Board Support

* Added the K230 CanMV MRT board (LPDDR4-2667, U-Boot and kernel DTS).
* Junroc board: the console is now on UART2 and the bank voltage is fixed. Added an mclk1 pinctrl for the gc2093 sensor.
* LCKFB: added a DTS symlink and fixed LT9611 HDMI detection.

#### 2. Linux Kernel & Drivers

* **DMA Security:** DMA addresses are checked more strictly, and PDMA checks the register range.
* **SPI:** dw-mmio now uses `platform_get_irq_optional`, which fixes getting the IRQ on K230.
* **Audio:** The codec route can be switched at runtime (SDI1 capture). Added 4-channel dual-mic capture and fixed a 24-bit problem.
* **IRQ:** Added an API to set priorities in sifive-plic.
* **Device Tree:** Added mclk settings to the sensor DTS.

#### 3. Multimedia (Camera / ISP / Display)

* Added MCM multi-camera support with OV13850 at 4K. vvcam_video accepts a `mcm_mask` parameter.
* Added DRM hardware rotation and touch calibration.
* Fixed:
  * v4l2-drm 180° rotation
  * A crash when repeatedly starting and stopping 1280x960
  * Wrong handling of the `-sw` and `-sh` parameters
  * IMX335 failing to run
* Removed the prebuilt isp_media_server ELF. Debian and Ubuntu use the same version.

#### 4. AI / Python / LVGL

* Added nncaseruntime and LVGL rotation Python bindings.
* Improved the LVGL Python bindings and their code generator.
* Added a yolov8n deb package.

#### 5. Build System & Distribution

* SD card images now use an MBR partition table. Fixed resizing the rootfs.
* Added `fw_printenv` and `sshpass`. Reworked rebooting into upgrade mode.
* Added `get_fast_url.sh`, which picks a faster download mirror.
* The download website is available in Chinese and English. CI removes the old image directory before saving new images.
* Fixed error handling when mmz allocation fails.

**Full Changelog**: https://github.com/kendryte/k230_linux_sdk/compare/v1.2...v1.3

[↑ Back to top](#table-of-contents)

---

## K230 Linux SDK Release Notes v1.2

Release date: 2026-07-22

### New Features

* **LVGL Python:**
  * Added pybind11 bindings and demos for four widgets: Canvas (ARGB8888 drawing), Line (polylines), Menu (menu pages) and Animimg (frame animation).
  * Reworked the LVGL Python API and cleaned up the demos.
* **Build:** Image file names now include the commit ID. Added new board configs.

**Full Changelog**: https://github.com/kendryte/k230_linux_sdk/compare/v1.1...v1.2

[↑ Back to top](#table-of-contents)

---

## K230 Linux SDK Release Notes v1.1

Release date: 2026-07-10

This release focuses on Python. It adds Python bindings for display, V4L2-DRM, LVGL and Adafruit Blinka. It also adds WebRTC source code and demos, and the Junroc AI Cam board.

### 🚀 Key Highlights

* **New Board:** K230D CanMV Junroc AI Cam (`k230d_canmv_junroc_ai_cam_defconfig`).
* **Python:**
  * The `python-k230` package is enabled by default.
  * New Python bindings: `k230_display`, `k230_v4l2_drm` (with OSD) and LVGL (pybind11).
  * Support for Adafruit Blinka and IOMUX configuration from Python.
* **WebRTC:** Added WebRTC source code and demos. Added H.265 support, and Opus audio and MQTT signaling are more reliable.

### New Features

* **Camera:** Scene switching and mirror/flip.
* **Video Codec:** FFmpeg encodes and decodes ARGB input. The MJPEG encoder lets you set the JPEG quality.
* **Display:**
  * VO supports GDMA rotation.
  * Optimized the K230 display driver.
  * U-Boot supports the ili9881 800x1280 LCD on 01Studio.
* **Python Demos:**
  * Added a YOLO demo that uses V4L2-DRM, and I2C and UART demos.
  * Moved `face_detect.py` to `py_demo/ai/`.
* **01Studio:** LED and button support.
* **USB:** uvc-gadget uses less CPU and names its threads.
* **Other:** The download website moved to download.kendryte.com.

### Bug Fixes

* Fixed sensor mirror/flip.
* Fixed memory leaks when closing the display and when stopping vvcam.
* Fixed a PWM bug, a 01Studio DTS error and the boot order on the 01Studio eMMC board.
* On Debian, whl packages are now installed by extracting them into Python site-packages.
* Fixed LVGL not finding pcpp, and several CI build errors.

**Full Changelog**: https://github.com/kendryte/k230_linux_sdk/compare/v1.0...v1.1

[↑ Back to top](#table-of-contents)

---

## K230 Linux SDK Release Notes v1.0

Release date: 2026-04-21

### 🚀 Key Highlights

* **nncase 2.11.0:** Updated nncase to 2.11.0. The AI demos now support yolov8 and yolo11 pose estimation, and yolo26.
* **USB Camera:** uvc-gadget can use a VPU-encoded video source. Added `uvc-gadget-acm.sh` for a UVC+ACM multi-function gadget.
* **IOMUX:** Added a K230 IOMUX pin control driver and a configuration tool. The driver now uses the pinctrl helpers.
* **OSD:** Added OSD to nonai2d. OSD parameters can be set at runtime, and a README documents the interface.
* **LLM:** Added a llama.cpp package.

### New Features

* **K230D CanMV:** Added I2C4 and MIPI CSI2 support, and changed the default sensor.
* **01Studio XCAM EVT:** Added Bluetooth through aic8800_sdio, and updated the pin configuration.
* **Camera:**
  * Sensors can be reset.
  * libmmz adds `kd_mpi_get_vvcam_video00()` to detect the vvcam video device.
  * A modprobe.d config makes sure vvcam_isp loads before nonai2d.
* **RTSP:** Improved camera_rtsp_demo and simplified its interface and demo code.
* **LCKFB:** DDR now runs at 2667.
* **Build:** Added an Ubuntu build script and updated the Dockerfile.

### Bug Fixes

* Fixed how the K230 clock composite sets its parent and determines its rate.
* Fixed an OSD problem.
* USB gadget stop now also cleans up functions and configs.
* Fixed a pcpp build error and a VSCode clangd error.
* Fixed an AI demo test problem. build.sh now downloads the nncase 2.11.0 kmodels.

**Full Changelog**: https://github.com/kendryte/k230_linux_sdk/compare/v0.6.9...v1.0

[↑ Back to top](#table-of-contents)

---

## v0.6.9

Release date: 2026-02-04

* Added Debian image CI, daily build CI and DongshanPi CI.
* 01Studio: changed the MMC and HDMI configuration.
* Fixed several CI problems, including an error saving images over TFTP.

[↑ Back to top](#table-of-contents)

---

## v0.6.5

Release date: 2026-01-28

This release includes development work from 2025-04 to 2026-01.

### 🚀 Key Highlights

* **Buildroot updated to 2025, toolchain updated to 3.0.2.**
* **nncase updated to 2.10.**
* **The build can produce deb packages, and Debian supports pip.**
* **New Boards:** GT6700 and the 01Studio eMMC variant (`k230_canmv_01studio_emmc_defconfig`).

### New Features

* **Camera:**
  * Added IMX335 (with analog and digital gain) and GC2093 at 60fps.
  * GC2093 supports multiple I2C addresses.
* **Display:**
  * Added the ili9881 800x1280 panel.
  * U-Boot can show a boot logo, and the logo parameters can be configured.
  * VO is initialized and rotated at startup, which fixes a green screen.
  * Changed the LCKFB LCD timings.
* **Video & RTSP:**
  * Added smolrtsp. Reduced RTSP latency and changed the number of V4L2 capture buffers.
  * FFmpeg can control VPU decode scaling. FFmpeg is back on 4.4.4.
* **USB:** ADB, MTP and CDC ACM support, plus USB patches.
* **Wireless:** Added the RTL8733BS driver and Wi-Fi reset.
* **AI:** Added YOLO demos, ai2d examples, cloud deployment code, an OpenCV RVV demo and an AI demo install script.
* **System:**
  * Added a boot banner and `fw_printenv`. The MMC partition is expanded automatically at boot.
  * Added valgrind and ldd, and support for `-fsanitize=address`.
  * Added clock and power domain support for the K230 AI blocks.
* **Debug & Test:** Added a DDR test image and a NOC QoS viewer. Added LPDDR4 initialization for DongshanPi.
* **CI:** GitHub Actions can run in forked repositories. The daily build syncs the download directory.

### Bug Fixes

* Fixed a display that was missing one row of pixels.
* Fixed buffer overflows (MX3SDK-1761).
* Fixed HS200 tuning output.
* Fixed build errors when building as root, Debian build errors, and Debian missing libjpeg.so9.
* Fixed dnn and zxing-cpp build errors. Fixed a V4L2 runtime error and improved `v4l2_drm_stop`.
* Updated the K230 audio driver and the LVGL version.

**Full Changelog**: https://github.com/kendryte/k230_linux_sdk/compare/v0.6.4...v0.6.5

[↑ Back to top](#table-of-contents)

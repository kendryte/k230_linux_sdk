# camera_webrtc_demo

K230 Linux (Buildroot) 上的 WebRTC 局域网摄像头 Demo。V4L2 摄像头采集 → 硬件编码（H.264/H.265）→ WebRTC 推流到浏览器，支持动态 OSD 叠加。

## 功能

- H.264 / H.265 视频实时推流（CBR，编码类型/分辨率/码率可配）
- 动态 OSD 叠加（最多 8 个区域，基于 non-ai-2d 零拷贝）
- 浏览器一键连接，自带 Web UI
- SDP offer/answer 信令（HTTP）
- mDNS 隐私检测与提示

## 快速开始

```bash
# 1. menuconfig 启用 camera_webrtc_demo
#    K230 Package Configuration  →  Enable Camera WebRTC Demo  = Y
make menuconfig

# 2. 编译（在 K230 Linux SDK 根目录）
make

# 3. 开发板上运行（默认 H.264，1280x720，2000kbps）
camera_webrtc_demo

# 4. 浏览器访问
# http://<设备IP>:8080
```

## 命令行参数

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `-H` | 显示帮助信息 | - |
| `-t` | 视频编码类型：`h264` / `h265` | `h264` |
| `-w` | 编码宽度（像素） | `1280` |
| `-h` | 编码高度（像素） | `720` |
| `-b` | 编码码率 (kbps) | `2000` |
| `-p` | HTTP 信令端口 | `8080` |
| `-l` | 启用日志 (0/1) | `0` |
| `-r` | OSD 区域数量 (0-8) | `0` |

### 使用示例

```bash
# H.264 默认参数
camera_webrtc_demo

# H.265 编码，1080p，4000kbps
camera_webrtc_demo -t h265 -w 1920 -h 1080 -b 4000

# H.264 + 4 个 OSD 区域 + 调试日志
camera_webrtc_demo -t h264 -r 4 -l 1

# 自定义 HTTP 端口
camera_webrtc_demo -p 9090
```

## 浏览器要求

### mDNS 隐私保护

Chrome/Edge 默认启用 mDNS 隐私保护，会隐藏本地 IP 导致 WebRTC 连接失败。页面会自动检测并提示。

修复方式（任选其一）：

1. 地址栏输入 `chrome://flags/#enable-webrtc-hide-local-ips-with-mdns` → 设为 **Disabled** → 重启
2. 启动参数加 `--disable-features=WebRtcHideLocalIpsWithMdns`
3. 使用 Firefox（默认未启用 mDNS）

### H.265 浏览器兼容性

> **注意**：使用 `-t h265` 时，部分浏览器无法解码 H.265 (HEVC) WebRTC 流。

| 浏览器 | H.265 WebRTC 支持 | 说明 |
|--------|-------------------|------|
| Chrome | ✅ 支持 | 需硬件 HEVC 解码支持（大部分桌面端已具备） |
| Edge | ✅ 支持 | 与 Chrome 同内核，同样依赖硬件解码 |
| Firefox | ❌ 不支持 | WebRTC 尚未实现 HEVC 解码 |
| Safari | ⚠️ 部分支持 | macOS/iOS 上依赖硬件解码，Windows 版不支持 |

**建议**：
- 如需最大兼容性，使用默认的 H.264 编码（`-t h264`）
- 仅在确认客户端浏览器支持 H.265 时使用 `-t h265`
- H.265 在相同画质下码率更低，适合带宽受限但对客户端可控的场景

## 架构

```
V4L2 (摄像头, NV12)
  │
  ├─[osd_region > 0]──> non-ai-2d OSD (mmap → dmabuf, 零拷贝)
  │                         │
  └─────────────────────────┴──> V4L2 M2M 编码器 (h264_v4l2m2m / hevc_v4l2m2m)
                                     │
                                     ▼
                                libpeer (WebRTC)
                                     │
                              HTTP 信令 (offer/answer)
                                     │
                                     ▼
                                 浏览器
```

**数据流**：

| 阶段 | 数据格式 | 内存类型 | 拷贝 |
|------|----------|----------|------|
| 摄像头 → OSD | NV12 | mmap | 1 次 |
| OSD → 编码器 | NV12 + OSD | dma_buf | 0 次（零拷贝） |
| 编码器 → WebRTC | H.264/H.265 | 系统内存 | 1 次 |

## 文件说明

| 文件 | 职责 |
|------|------|
| `main.cpp` | 入口、命令行参数解析 |
| `camera_webrtc_demo.cpp/.h` | WebRTC 连接管理、信令处理、编码帧转发 |
| `media.cpp/.h` | V4L2 摄像头采集、硬件编码、OSD 管线 |
| `osd.cpp/.h` | OSD 管理器（non-ai-2d 滤镜封装） |
| `http_server.c/.h` | HTTP 服务器（单线程、CORS） |
| `web_page.h` | 嵌入式前端页面 |
| `OSD1_40x40_argb.c` | OSD 示例图像数据（ARGB） |
| `Makefile` | 构建脚本 |

## 依赖

- **libpeer** — WebRTC 协议栈（含 mbedTLS、SRTP、SCTP）
- **FFmpeg** — V4L2 采集、硬件编码（h264_v4l2m2m / hevc_v4l2m2m）、non-ai-2d OSD 滤镜
- **libevent** — HTTP 服务器事件循环
- **cJSON** — SDP 解析

## License

遵循 K230 Linux SDK 许可协议。

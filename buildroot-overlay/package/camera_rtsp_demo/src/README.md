# Camera RTSP Demo

基于 FFmpeg + Live555 的摄像头 RTSP 推流演示程序，支持 **硬件编码**（H.264/H.265）和 **动态 OSD 叠加**（On-Screen Display）。

---

## 目录

- [功能特性](#功能特性)
- [系统架构](#系统架构)
- [硬件要求](#硬件要求)
- [编译方法](#编译方法)
- [使用方法](#使用方法)
- [命令行参数](#命令行参数)
- [OSD 接口详解](#osd-接口详解)
- [使用示例](#使用示例)
- [性能优化](#性能优化)
- [常见问题](#常见问题)

---

## 功能特性

- ✅ **摄像头采集**：V4L2 接口采集 NV12 格式视频
- ✅ **硬件编码**：h264_v4l2m2m / hevc_v4l2m2m（零拷贝）
- ✅ **RTSP 推流**：基于 Live555，支持多客户端
- ✅ **动态 OSD**：支持 8 个独立 OSD 区域，每帧可更新位置和图像
- ✅ **零拷贝流水线**：摄像头 → OSD → 编码器（dma_buf 传递）
- ✅ **低延迟优化**：30fps 实时推流，端到端延迟 < 200ms

---

## 系统架构

```
┌─────────────────────────────────────────────────────────────────┐
│                      Camera RTSP Demo                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────┐    ┌──────────────┐    ┌──────────────┐          │
│  │ 摄像头   │    │ OSD Manager  │    │ 硬件编码器   │          │
│  │ (V4L2)   │───▶│ nonai2d_osd  │───▶│ h264_v4l2m2m │          │
│  │ NV12     │    │ (dma_buf 输出)│    │ (dma_buf 输入)│          │
│  └──────────┘    └──────────────┘    └──────────────┘          │
│       │                   │                   │                 │
│       │ mmap              │ dma_buf 句柄       │ H.264/H.265    │
│       │                   │   (零拷贝)         │ 数据包         │
│       │                   │                   │                 │
│       │                   │                   ▼                 │
│       │                   │          ┌──────────────┐          │
│       │                   │          │ RTSP 服务器  │          │
│       │                   │          │ (Live555)    │          │
│       │                   │          └──────────────┘          │
│       │                   │                   │                 │
│       └───────────────────┴───────────────────┘                 │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
                    ┌──────────────────┐
                    │   RTSP 客户端    │
                    │ VLC / FFplay     │
                    │ rtsp://IP:8554   │
                    └──────────────────┘
```

**数据流说明**：

| 阶段 | 数据格式 | 内存类型 | 拷贝次数 |
|------|----------|----------|----------|
| 摄像头 → OSD | NV12 | mmap | 1 次 |
| OSD → 编码器 | NV12 + OSD | dma_buf | **0 次**（零拷贝） |
| 编码器 → RTSP | H.264/H.265 | 系统内存 | 1 次 |

### 1. 基本用法（H.264，无 OSD）

```bash
# 在开发板上运行
/usr/bin/camera_rtsp_demo
```

默认参数：
- 编码格式：H.264
- 分辨率：1280x720
- 比特率：2000 kbps
- RTSP 端口：8554
- 流名称：`test`
- OSD 区域：0（禁用）

### 2. 启用 OSD

```bash
# 启用 4 个 OSD 区域
/usr/bin/camera_rtsp_demo -r 4
```

### 3. 自定义参数

```bash
# H.264 编码，1080p，4000kbps，启用 OSD
/usr/bin/camera_rtsp_demo -t h264 -w 1280 -h 720 -b 2000 -r 4
```

### 4. 播放 RTSP 流

在 PC 上使用 VLC 或 FFplay 播放：

```bash
# VLC
vlc rtsp://<开发板 IP>:8554/test

# FFplay
ffplay rtsp://<开发板 IP>:8554/test

# FFmpeg（测试连接）
ffprobe rtsp://<开发板 IP>:8554/test
```

---

## 命令行参数

| 参数 | 说明 | 默认值 | 示例 |
|------|------|--------|------|
| `-H` | 显示帮助信息 | - | `-H` |
| `-t` | 视频编码格式 | `h264` | `-t h264` 或 `-t h265` |
| `-w` | 编码宽度（像素） | `1280` | `-w 1920` |
| `-h` | 编码高度（像素） | `720` | `-h 1080` |
| `-b` | 编码比特率（kbps） | `2000` | `-b 4000` |
| `-l` | 启用日志（0/1） | `0` | `-l 1` |
| `-r` | OSD 区域数量（0-8） | `0` | `-r 4` |

### 参数组合示例

```bash
# 示例 1：720p H.264 低码率（适合网络环境差）
camera_rtsp_demo -w 1280 -h 720 -b 1000

# 示例 2：1080p H.265 高画质（适合本地网络）
camera_rtsp_demo -t h265 -w 1920 -h 1080 -b 6000

# 示例 3：启用 OSD + 调试日志
camera_rtsp_demo -r 4 -l 1

# 示例 4：极限低延迟配置
camera_rtsp_demo -w 640 -h 480 -b 500 -r 0
```

---

## OSD 接口详解

OSD（On-Screen Display）功能允许在视频画面上叠加动态图像（如时间戳、Logo、图标等）。

### 核心概念

- **OSD 区域**：最多支持 8 个独立区域，每个区域可独立设置位置、尺寸、图像
- **动态更新**：每帧可更新 OSD 区域的位置和图像数据
- **零拷贝**：OSD 输出通过 dma_buf 直接传递给编码器，无需内存拷贝

### 类结构

#### `OsdManager` 类

```cpp
class OsdManager {
public:
    OsdManager();                              // 构造函数
    ~OsdManager();                             // 析构函数
    
    int Init(int width, int height, AVRational time_base, 
             const char* in_mem_type, const char* out_mem_type);  // 初始化
    int Apply(AVFrame *frame, OsdRegion* regions, int region_count); // 应用 OSD
    void Deinit();                             // 反初始化
    bool IsReady() const;                      // 检查是否就绪

private:
    AVFilterGraph *graph_;                     // 滤镜图容器
    AVFilterContext *main_src_;                // 主视频输入滤镜
    AVFilterContext *osd_ctx_;                 // OSD 滤镜上下文
    AVFilterContext *sink_;                    // 输出接收滤镜
    int width_, height_;                       // 视频尺寸
    AVRational time_base_;                     // 时间基准
    bool inited_;                              // 初始化标志
    int frame_cnt_;                            // 帧计数器
    Nonai2dOsdConfig osd_config_;              // OSD 配置
};
```

#### `OsdRegion` 结构体

```cpp
typedef struct {
    int x;                           // 区域左上角 X 坐标（像素）
    int y;                           // 区域左上角 Y 坐标（像素）
    int width;                       // 区域宽度（像素）
    int height;                      // 区域高度（像素）
    const unsigned int *osd_image_data;  // OSD 图像数据指针（ARGB 格式）
    int osd_image_size;              // OSD 图像数据大小（字节）
    int enabled;                     // 是否启用（1=启用，0=禁用）
} OsdRegion;
```

---

### 完整接口详解

#### 1. 构造函数 `OsdManager::OsdManager()`

**作用**：初始化成员变量为默认值。

**初始化状态**：
| 成员 | 初始值 |
|------|--------|
| `graph_` | `nullptr` |
| `main_src_` | `nullptr` |
| `osd_ctx_` | `nullptr` |
| `sink_` | `nullptr` |
| `width_`, `height_` | `0` |
| `time_base_` | `{0, 0}` |
| `inited_` | `false` |
| `frame_cnt_` | `0` |

---

#### 2. 析构函数 `OsdManager::~OsdManager()`

**作用**：自动调用 `Deinit()` 释放资源。

---

#### 3. 初始化接口 `OsdManager::Init()`

**函数签名**：
```cpp
int Init(int width, int height, AVRational time_base, 
         const char* in_mem_type, const char* out_mem_type);
```

**参数详解**：

| 参数 | 类型 | 含义 | 示例 |
|------|------|------|------|
| `width` | `int` | 主视频宽度（像素） | `1280` |
| `height` | `int` | 主视频高度（像素） | `720` |
| `time_base` | `AVRational` | 时间基准（帧率相关） | `{1, 30}` 表示 30 帧/秒 |
| `in_mem_type` | `const char*` | 输入内存类型 | `"mmap"` 或 `"dmabuf"` |
| `out_mem_type` | `const char*` | 输出内存类型 | `"dmabuf"`（推荐）或 `"mmap"` |

**返回值**：
- `0`：成功
- `<0`：失败（FFmpeg 错误码）

**内存类型组合**：

```cpp
// 用法 1：dma_buf 零拷贝输出（推荐，用于硬件编码器）
ret = osd_manager_.Init(width, height, time_base, "mmap", "dmabuf");

// 用法 2：全系统内存（兼容性好，性能较低）
ret = osd_manager_.Init(width, height, time_base, "system", "system");

// 用法 3：全 DMA（需要输入也是 dma_buf）
ret = osd_manager_.Init(width, height, time_base, "dmabuf", "dmabuf");
```

**推荐配置解析**：
- **`in_mem_type = "mmap"`**：从摄像头采集的帧通过 mmap 映射到用户空间
- **`out_mem_type = "dmabuf"`**：输出 dma_buf 句柄，可直接传递给硬件编码器，**无需内存拷贝**

**实现流程**：

**步骤 1**：自动检测 non-ai-2d 设备
```cpp
// 遍历 /sys/class/video4linux/video*/name
// 查找包含 "non-ai-2d"、"nonai" 或 "canaan-non-ai-2d" 的设备
// 默认返回 "/dev/video0"
```

**步骤 2**：创建滤镜图容器
```cpp
graph_ = avfilter_graph_alloc();
```

**步骤 3**：创建主视频输入源（buffer 滤镜）
```cpp
char args[256];
snprintf(args, sizeof(args),
         "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=1/1",
         width, height, AV_PIX_FMT_NV12, time_base.num, time_base.den);

ret = avfilter_graph_create_filter(&main_src_, buffer, "in", args, NULL, graph_);
```

| 参数 | 说明 | 示例 |
|------|------|------|
| `video_size` | 视频分辨率 | `1280x720` |
| `pix_fmt` | 像素格式（固定） | `AV_PIX_FMT_NV12` |
| `time_base` | 时间基准 | `1/30` |
| `pixel_aspect` | 像素宽高比（固定） | `1/1` |

**步骤 4**：创建输出接收器（buffersink 滤镜）
```cpp
ret = avfilter_graph_create_filter(&sink_, buffersink, "out", NULL, NULL, graph_);
```

**步骤 5**：构建滤镜描述字符串
```cpp
char region_args[384];
snprintf(region_args, sizeof(region_args),
         "[in]nonai2d_osd@nonai2d=device=%s:in_mem_type=%s:out_mem_type=%s[out]",
         nonai2d_dev.c_str(),
         in_mem_type,
         out_mem_type);

filter_desc = region_args;
```

**滤镜描述语法**：
```
[in]nonai2d_osd@nonai2d=device=<设备路径>:in_mem_type=<输入类型>:out_mem_type=<输出类型>[out]
```

| 参数 | 含义 | 取值 |
|------|------|------|
| `device` | non-ai-2d 设备路径 | `/dev/video0` 等 |
| `in_mem_type` | 输入内存类型 | `"mmap"` 或 `"dmabuf"` |
| `out_mem_type` | 输出内存类型 | `"dmabuf"` 或 `"mmap"` |

**步骤 6**：解析并连接滤镜图
```cpp
AVFilterInOut *outputs = avfilter_inout_alloc();
AVFilterInOut *inputs = avfilter_inout_alloc();

outputs->name = av_strdup("in");
outputs->filter_ctx = main_src_;
outputs->pad_idx = 0;
outputs->next = NULL;

inputs->name = av_strdup("out");
inputs->filter_ctx = sink_;
inputs->pad_idx = 0;
inputs->next = NULL;

ret = avfilter_graph_parse_ptr(graph_, filter_desc.c_str(), &inputs, &outputs, NULL);
```

**步骤 7**：配置滤镜图
```cpp
ret = avfilter_graph_config(graph_, NULL);
```

**作用**：验证滤镜连接合法性，分配内部缓冲区。

**步骤 8**：查找 OSD 滤镜上下文
```cpp
osd_ctx_ = FindNonai2dContext(graph_);
```

**查找顺序**：
1. 查找名为 `"nonai2d"` 的滤镜
2. 查找名为 `"Parsed_nonai2d_osd_0"` 的滤镜
3. 遍历所有滤镜，查找类型为 `"nonai2d_osd"` 的滤镜

---

#### 4. 应用 OSD 接口 `OsdManager::Apply()`

**函数签名**：
```cpp
int Apply(AVFrame *frame, OsdRegion* regions, int region_count);
```

**参数详解**：

| 参数 | 类型 | 含义 | 约束 |
|------|------|------|------|
| `frame` | `AVFrame*` | 输入/输出视频帧（原地修改） | 不能为 `nullptr` |
| `regions` | `OsdRegion*` | OSD 区域数组 | 不能为 `nullptr` |
| `region_count` | `int` | 区域数量 | `1 <= region_count <= 8` |

**返回值**：
- `0`：成功
- `<0`：失败（错误码）

**实现流程**：

**步骤 1**：配置每个 OSD 区域
```cpp
for (int i = 0; i < region_count; ++i) {
    OsdRegion* region = &regions[i];
    
    if (!region->enabled || !region->osd_image_data) {
        continue;  // 跳过未启用或无数据的区域
    }
    
    char args[512];
    snprintf(args, sizeof(args),
             "%d:index:%d:x:%d:y:%d:width:%d:height:%d:valid:%d:data:%p",
             i,                    // 命令索引
             i,                    // 区域索引
             region->x,            // X 坐标
             region->y,            // Y 坐标
             region->width,        // 宽度
             region->height,       // 高度
             1,                    // valid = 1（启用）
             region->osd_image_data); // 图像数据指针
    
    char res[128];
    ret = avfilter_process_command(osd_ctx_,
                                   "config_region",  // 命令名
                                   args,             // 参数字符串
                                   res,              // 返回结果
                                   sizeof(res),
                                   0);               // 标志位
}
```

**命令格式详解**：
```
<cmd_idx>:index:<region_idx>:x:<x>:y:<y>:width:<w>:height:<h>:valid:<v>:data:<ptr>
```

| 字段 | 含义 |
|------|------|
| `cmd_idx` | 命令索引（通常与 region_idx 相同） |
| `index` | OSD 区域索引（0-7） |
| `x`, `y` | 区域左上角坐标（相对于视频画面） |
| `width`, `height` | OSD 图像尺寸（像素） |
| `valid` | 有效性标志（1=启用，0=禁用） |
| `data` | OSD 图像数据指针（ARGB 格式内存地址） |

**步骤 2**：保存配置到内部结构
```cpp
osd_config_.x[i] = region->x;
osd_config_.y[i] = region->y;
osd_config_.width[i] = region->width;
osd_config_.height[i] = region->height;
osd_config_.valid[i] = 1;
```

**步骤 3**：推送视频帧到滤镜输入
```cpp
ret = av_buffersrc_add_frame_flags(main_src_, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
```

**步骤 4**：解除帧引用
```cpp
av_frame_unref(frame);
```

**作用**：释放 `frame` 原有数据，为接收输出帧做准备。

**步骤 5**：从滤镜输出拉取处理后的帧
```cpp
ret = av_buffersink_get_frame(sink_, frame);
```

**作用**：获取已叠加 OSD 的视频帧。

**关键点**：当 `out_mem_type = "dmabuf"` 时，`frame->data[0]` 包含的是 **dma_buf 文件描述符句柄**，而非传统内存指针。此句柄可直接传递给硬件编码器。

**步骤 6**：更新帧计数器
```cpp
frame_cnt_++;
```

---

#### 5. 反初始化接口 `OsdManager::Deinit()`

**作用**：释放所有资源。

**执行操作**：
```cpp
if (graph_) {
    avfilter_graph_free(&graph_);  // 释放滤镜图
}

main_src_ = nullptr;
osd_ctx_ = nullptr;
sink_ = nullptr;
inited_ = false;
```

---

#### 6. 就绪检查接口 `OsdManager::IsReady()`

**函数签名**：
```cpp
bool IsReady() const;
```

**返回值**：
- `true`：已初始化，可以调用 `Apply()`
- `false`：未初始化或已反初始化

---

### 辅助函数

#### `FindNonai2dContext()`

**作用**：在滤镜图中查找 `nonai2d_osd` 滤镜上下文。

**函数签名**：
```cpp
static AVFilterContext *FindNonai2dContext(AVFilterGraph *graph);
```

**查找逻辑**：
1. 优先查找名为 `"nonai2d"` 的滤镜
2. 其次查找名为 `"Parsed_nonai2d_osd_0"` 的滤镜
3. 遍历所有滤镜，匹配类型名 `"nonai2d_osd"`

**返回值**：
- 成功：`AVFilterContext*` 指针
- 失败：`nullptr`

---

### 内存类型配置

| 参数 | 推荐值 | 说明 |
|------|--------|------|
| `in_mem_type` | `"mmap"` | 摄像头采集帧通过 mmap 映射 |
| `out_mem_type` | `"dmabuf"` | 输出 dma_buf 句柄给编码器（零拷贝关键） |

### OSD 图像格式要求

- **像素格式**：ARGB（每像素 4 字节：Alpha + Red + Green + Blue）
- **内存对齐**：必须 4096 字节对齐（`__attribute__((aligned(0x1000)))`）
- **尺寸限制**：建议 ≤ 200x200 像素（过大会影响性能）
- **生命周期**：调用 `Apply()` 时数据必须有效，建议在回调中实时更新

### 动态 OSD 更新回调

```cpp
// 回调函数原型
typedef void (*OsdRegionCallback)(OsdRegion* regions, int region_count, void* user_data);

// 示例：每帧更新 OSD 区域位置（圆形排列）
void OnUpdateOsdRegions(OsdRegion* regions, int region_count, void* user_data) {
    if (!regions || region_count <= 0) {
        return;
    }

    // 生成彩虹色小图标（40x40）
    GenerateSmallRainbow();  // 填充 g_small_osd_data
    
    // 计算动态位置（圆形排列）
    int time_offset = (g_frame_count / 3) % 360;
    int radius = 250;
    int center_x = 640;
    int center_y = 360;
    
    for (int i = 0; i < region_count; i++) {
        int angle = (i * 360 / region_count + time_offset) % 360;
        float rad = angle * 3.14159f / 180.0f;
        
        int x = center_x + (int)(cos(rad) * radius) - 20;
        int y = center_y + (int)(sin(rad) * radius) - 20;
        
        // 边界检查
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x > 1240) x = 1240;
        if (y > 680) y = 680;
        
        regions[i].x = x;
        regions[i].y = y;
        regions[i].width = 40;
        regions[i].height = 40;
        regions[i].osd_image_data = g_small_osd_data;
        regions[i].osd_image_size = sizeof(g_small_osd_data);
        regions[i].enabled = 1;
    }
}
```

---

## 使用示例

### 示例 1：基础 RTSP 推流（无 OSD）

```bash
# 启动推流
camera_rtsp_demo -t h264 -w 1280 -h 720 -b 2000

# 输出:
# Config parameters:
# Video encoder type: h264
# Video encoder width: 1280
# Video encoder height: 720
# Video encoder bitrate (kbps): 2000
# Enable log: false
# OSD region count: 0
#
# camera_rtsp_demo started.
# Play this stream using the URL: rtsp://<IP>:8554/test
```

### 示例 2：启用动态 OSD

```bash
# 启动推流 + 4 个 OSD 区域
camera_rtsp_demo -t h264 -w 1280 -h 720 -b 2000 -r 4 -l 1
```

**效果**：4 个彩虹色小图标在屏幕上呈圆形排列旋转。

### 示例 3：自定义 OSD 图像

修改 `src/camera_rtsp_demo.cpp` 中的 `OnUpdateOsdRegions()` 函数：

```cpp
// 静态 Logo（不随时间变化）
void OnUpdateOsdRegions(OsdRegion* regions, int region_count, void* user_data) {
    // 左上角：公司 Logo（100x50）
    regions[0].x = 20;
    regions[0].y = 20;
    regions[0].width = 100;
    regions[0].height = 50;
    regions[0].osd_image_data = logo_argb_data;
    regions[0].enabled = 1;
    
    // 右上角：时间戳（200x40）
    regions[1].x = 1160;  // 1280 - 20 - 200
    regions[1].y = 20;
    regions[1].width = 200;
    regions[1].height = 40;
    regions[1].osd_image_data = timestamp_argb_data;
    regions[1].enabled = 1;
    
    // 禁用其他区域
    for (int i = 2; i < region_count; i++) {
        regions[i].enabled = 0;
    }
}
```

### 示例 4：多客户端同时播放

RTSP 服务器支持多客户端同时连接：

```bash
# 客户端 1（VLC）
vlc rtsp://192.168.1.100:8554/test

# 客户端 2（FFplay）
ffplay rtsp://192.168.1.100:8554/test

# 客户端 3（手机）
# 使用 VLC for Android / iOS
# 打开 rtsp://192.168.1.100:8554/test
```

---

## 完整 OSD 使用示例（Camera RTSP Demo）

### 场景说明
- 摄像头采集 NV12 格式视频（v4l2 设备）
- 通过 OSD 管理器叠加动态 OSD 区域
- 输出 **dma_buf** 帧直接送入硬件编码器（h264_v4l2m2m）
- 编码后的 RTSP 流推送给客户端

### 1. 数据结构定义

```cpp
// OSD 区域配置（8 个区域）
OsdRegion regions_[NONAI2D_OSD_REGION_NUM];

// OSD 图像数据（对齐到 0x1000 边界）
static unsigned int g_small_osd_data[40 * 40] __attribute__((aligned(0x1000)));

// OSD 管理器
OsdManager osd_manager_;
```

### 2. 初始化阶段

```cpp
// 初始化 OSD 管理器（关键：输出使用 dmabuf）
int ret = osd_manager_.Init(
    input_config_.venc_width,      // 视频宽度，如 1280
    input_config_.venc_height,     // 视频高度，如 720
    (AVRational){1, 30},           // 时间基准：30fps
    "mmap",                        // 输入：mmap 映射的系统内存
    "dmabuf"                       // 输出：dma_buf 句柄（零拷贝关键）
);
if (ret < 0) {
    char errbuf[AV_ERROR_MAX_STRING_SIZE];
    av_strerror(ret, errbuf, sizeof(errbuf));
    std::cerr << "nonai2d osd init failed: " << errbuf << std::endl;
    return -1;
}

// 初始化 OSD 区域（默认禁用）
for (int i = 0; i < NONAI2D_OSD_REGION_NUM; ++i) {
    regions_[i].enabled = 0;
    regions_[i].osd_image_data = nullptr;
}
```

### 3. 编码器配置（支持 dma_buf 输入）

```cpp
// 创建硬件编码器上下文（h264_v4l2m2m）
const AVCodec *codec = avcodec_find_encoder_by_name("h264_v4l2m2m");
AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);

// 配置编码器参数
codec_ctx->width = width;
codec_ctx->height = height;
codec_ctx->pix_fmt = AV_PIX_FMT_NV12;
codec_ctx->time_base = (AVRational){1, 30};
codec_ctx->bit_rate = bitrate_kbps * 1000;
codec_ctx->gop_size = 30;
codec_ctx->max_b_frames = 0;

// 关键配置：启用 dmabuf 输入模式
AVDictionary *codec_options = nullptr;
av_dict_set_int(&codec_options, "num_output_buffers", 1, 0);
av_dict_set_int(&codec_options, "num_capture_buffers", 1, 0);

// 启用 dmabuf 模式（与 OSD 输出匹配）
av_dict_set(&codec_options, "in_mem_type", "dmabuf", 0);

// 低延迟优化
av_dict_set(&codec_options, "tune", "zerolatency", 0);

ret = avcodec_open2(codec_ctx, codec, &codec_options);
if (ret < 0) {
    std::cerr << "Cannot open codec" << std::endl;
    return -1;
}
```

### 4. 动态 OSD 区域更新回调

```cpp
// 回调函数：每帧调用，更新 OSD 区域位置和图像
void OnUpdateOsdRegions(OsdRegion* regions, int region_count, void* user_data) {
    if (!regions || region_count <= 0) {
        return;
    }

    // 示例：动态生成彩虹色小图标（40x40）
    GenerateSmallRainbow();  // 填充 g_small_osd_data
    
    // 计算动态位置（圆形排列）
    int time_offset = (g_frame_count / 3) % 360;
    int radius = 250;
    int center_x = 640;
    int center_y = 360;
    
    for (int i = 0; i < region_count; i++) {
        int angle = (i * 360 / region_count + time_offset) % 360;
        float rad = angle * 3.14159f / 180.0f;
        
        int x = center_x + (int)(cos(rad) * radius) - 20;
        int y = center_y + (int)(sin(rad) * radius) - 20;
        
        // 边界检查
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x > 1240) x = 1240;
        if (y > 680) y = 680;
        
        regions[i].x = x;
        regions[i].y = y;
        regions[i].width = 40;
        regions[i].height = 40;
        regions[i].osd_image_data = g_small_osd_data;
        regions[i].osd_image_size = sizeof(g_small_osd_data);
        regions[i].enabled = 1;
    }
}
```

### 5. 主处理循环（每帧处理）

```cpp
// 线程：从摄像头读取原始帧并编码
void *camera_venc_stream_thread(void *arg) {
    KdMedia *media = static_cast<KdMedia*>(arg);
    AVFrame *frame = media->frame_;
    AVCodecContext *codec_ctx = media->codec_ctx_;
    OsdManager &osd_manager = media->osd_manager_;
    
    while (!media->stop_flag_) {
        // 1. 从摄像头获取原始帧（AVPacket）
        AVPacket *pkt = get_camera_packet();
        if (!pkt) continue;
        
        // 2. 将原始数据映射到 AVFrame
        frame->data[0] = pkt->data;  // Y 平面
        frame->linesize[0] = width;
        frame->data[1] = pkt->data + width * height;  // UV 平面
        frame->linesize[1] = width;
        frame->pts = pkt->pts;
        
        // 3. 更新 OSD 区域（动态位置/图像）
        if (input_config.osd_region > 0 && osd_manager.IsReady()) {
            input_config.osd_callback(media->regions_, 
                                      input_config.osd_region, 
                                      input_config.osd_user_data);
            
            // 4. 应用 OSD（关键：输出 dma_buf 帧）
            ret = osd_manager.Apply(frame, media->regions_, input_config.osd_region);
            if (ret < 0) {
                std::cerr << "OSD apply failed: " << ret << std::endl;
                continue;
            }
        }
        
        // 5. 发送帧到硬件编码器（dma_buf 零拷贝）
        ret = avcodec_send_frame(codec_ctx, frame);
        if (ret < 0) {
            std::cerr << "Error sending frame to encoder" << std::endl;
            break;
        }
        
        // 6. 接收编码后的数据包
        while (true) {
            ret = avcodec_receive_packet(codec_ctx, pkt2);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }
            if (ret < 0) {
                std::cerr << "Error receiving packet" << std::endl;
                break;
            }
            
            // 7. 发送编码数据到 RTSP 服务器
            feature_config.on_venc_data->OnVEncData(
                pkt2->data, pkt2->size, 
                (pkt2->flags & AV_PKT_FLAG_KEY),
                get_precise_timestamp_us()
            );
            
            av_packet_unref(pkt2);
        }
        
        av_packet_free(&pkt);
    }
    
    return nullptr;
}
```

---

## 关键注意事项

### 1. OSD 图像数据格式
- **格式要求**：ARGB（每像素 4 字节：Alpha + Red + Green + Blue）
- **内存对齐**：必须 4096 字节对齐（`__attribute__((aligned(0x1000)))`）
- **生命周期**：调用 `Apply()` 时数据必须有效，建议在回调中实时更新

### 2. 内存类型选择（关键性能因素）

| 组合 | in_mem_type | out_mem_type | 适用场景 | 性能 |
|------|-------------|--------------|----------|------|
| **推荐** | `"mmap"` | `"dmabuf"` | 摄像头→OSD→硬件编码器 | 零拷贝 |

**零拷贝原理**：

- OSD 输出 `dmabuf` 文件描述符句柄
- 硬件编码器直接接收该句柄作为输入
- 无需在用户空间/内核空间之间复制数据

### 3. 区域数量限制
- 最大支持 `NONAI2D_OSD_REGION_NUM = 8` 个独立 OSD 区域
- `region_count` 必须在 `1` 到 `8` 之间

### 4. 错误处理
所有接口返回 `<0` 时表示失败，常见错误码：
- `AVERROR(ENOMEM)`：内存不足
- `AVERROR(EINVAL)`：参数无效
- `AVERROR_FILTER_NOT_FOUND`：找不到 `nonai2d_osd` 滤镜

### 5. 时间戳同步
- OSD 处理前后需保持 `frame->pts` 不变
- 编码器输入/输出时间戳需正确传递

---

## 依赖的 FFmpeg 组件

```cpp
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/frame.h>
#include <libavutil/rational.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
```

**核心 API**：
- `avfilter_graph_alloc()` / `avfilter_graph_free()`
- `avfilter_get_by_name()`
- `avfilter_graph_create_filter()`
- `avfilter_graph_parse_ptr()`
- `avfilter_graph_config()`
- `avfilter_process_command()`
- `av_buffersrc_add_frame_flags()`
- `av_buffersink_get_frame()`
- `av_frame_unref()`
- `avcodec_send_frame()` / `avcodec_receive_packet()`（编码器）

---

## 数据流架构图

```
┌─────────────────┐     ┌──────────────────┐     ┌─────────────────┐
│  摄像头采集     │     │   OSD Manager    │     │  硬件编码器     │
│  (v4l2 NV12)    │────▶│  nonai2d_osd     │────▶│ (h264_v4l2m2m)  │
│                 │     │                  │     │                 │
│  mmap 映射      │     │  in_mem_type=    │     │  in_mem_type=   │
│  系统内存       │     │  "mmap"          │     │  "dmabuf"       │
└─────────────────┘     │                  │     └─────────────────┘
                        │  out_mem_type=   │              │
                        │  "dmabuf"        │              │
                        │  (dma_buf 句柄)   │              │
                        └──────────────────┘              │
                                                          ▼
                                                 ┌─────────────────┐
                                                 │   RTSP 服务器   │
                                                 │   网络推送      │
                                                 └─────────────────┘
```

**关键路径**：
1. 摄像头 → OSD：mmap 系统内存（传统拷贝）
2. OSD → 编码器：**dma_buf 句柄传递（零拷贝）**
3. 编码器 → RTSP：编码后 H.264/H.265 数据包

---

## 性能优化

### 1. 降低延迟

```bash
# 降低分辨率 + 降低比特率
camera_rtsp_demo -w 640 -h 480 -b 500 -r 0
```

**预期效果**：端到端延迟 < 100ms

### 2. 提高画质

```bash
# 1080p + H.265 + 高比特率
camera_rtsp_demo -t h265 -w 1920 -h 1080 -b 6000 -r 0
```

**注意**：H.265 需要客户端支持（VLC 3.0+）

### 3. 启用 OSD 时的性能考虑

| OSD 区域数 | 额外延迟 | CPU 占用 | 推荐场景 |
|-----------|----------|----------|----------|
| 0 | 0ms | 0% | 低延迟监控 |
| 1-2 | ~5ms | 2-5% | 时间戳 + Logo |
| 3-4 | ~10ms | 5-10% | 多信息叠加 |
| 5-8 | ~20ms | 10-15% | 复杂 UI |

### 4. 网络优化

```bash
# 限制最大比特率（避免网络拥塞）
camera_rtsp_demo -b 1500
```

**网络带宽参考**：
| 分辨率 | 推荐比特率 | 最低带宽需求 |
|--------|------------|--------------|
| 640x480 | 500 kbps | 1 Mbps |
| 1280x720 | 2000 kbps | 4 Mbps |
| 1920x1080 | 4000 kbps | 8 Mbps |

---

## 许可证

本项目采用 MIT 许可证。

---

## 致谢

- **FFmpeg**：音视频处理框架
- **Live555**：RTSP 服务器库
- **librtsp_server**：自定义 RTSP 服务器封装
- **Canaan**：K230 开发板支持


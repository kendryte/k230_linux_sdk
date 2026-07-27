# v4l2-drm-scene

V4L2 + DRM 显示 demo，支持多 scene 热切换功能。

> ⚠️ **重要使用约束**：scene 之间**只能修改 ISP 参数值**（曝光、增益、白平衡
> 曲线、gamma 等），**不能开关不同的 ISP 子模块**。详见下文"限制与约束"章节。

## 特性

- 单 `/dev/videoX` 设备，NV12 + DMABUF 零拷贝
- 多 scene JSON 配置（最多 10 个，默认 NV12 / 5 buffers）
- 数字键 `1`~`9` 热切换，`g` 读回当前配置，`q` 退出
- 切换流程：`VIDIOC_S_EXT_CTRLS` 下发新 scene → 完全重启 video 设备（close/reopen）
- FPS 实时显示

## 编译

demo 通过 buildroot 打包，编译整套镜像时会自动带上；也可独立 out-of-tree 构建：

```bash
cd buildroot-overlay/package/vvcam/v4l2-drm-scene
mkdir build && cd build
cmake ..
make
```

## 使用

### 命令行

```bash
./v4l2-drm-scene -d <device> -w <width> -h <height> -s <scene_config.json>
```

| 参数 | 说明                        | 必需 | 默认值 |
|------|-----------------------------|------|--------|
| `-d` | `/dev/videoX` 的编号 X      | 否   | 0      |
| `-w` | 捕获宽度                    | 否   | 640    |
| `-h` | 捕获高度                    | 否   | 480    |
| `-s` | Scene 配置 JSON 文件路径    | 否   | 无     |

> 像素格式固定为 `V4L2_PIX_FMT_NV12`，显示格式为 `DRM_FORMAT_NV12`，申请 5 个
> buffer（DMA-BUF）。如需修改，编辑 `main.c` 里 `ctx.video_format` / `ctx.buffer_num` 重新编译。

### 示例

```bash
# 640x480，加载多 scene
./v4l2-drm-scene -d 0 -w 640 -h 480 -s /etc/vvcam/scene_config.json

# 1080p
./v4l2-drm-scene -d 0 -w 1920 -h 1080 -s /etc/vvcam/scene_config.json
```

### 热键

加载了 `-s` scene 配置之后才有意义：

| 按键      | 功能                           |
|-----------|--------------------------------|
| `1`~`9`   | 切换到对应索引的 scene（1 基）|
| `g`       | 读回当前生效的 scene 配置     |
| `q`       | 退出程序                       |

其他按键不会触发动作，会打印一条提示信息。

## Scene 配置文件

### 格式

```json
{
    "scenes": [
        {
            "name": "day",
            "calib_dir": "/etc/vvcam/day",
            "mode": 0
        },
        {
            "name": "night",
            "calib_dir": "/etc/vvcam/night",
            "mode": 0
        }
    ]
}
```

> demo 使用一个轻量行式 JSON 解析器，要求 `"key": value` 每行一对，不支持
> 多行字符串 / 注释 / 嵌套对象（数组除外）。`scene_config_example.json` 里的
> 格式可以直接作为模板。

> **校准文件路径由 `isp_media_server` 根据 sensor 名 + 分辨率自动拼接**，
> 命名规则为 `{calib_dir}/{sensor}-{W}x{H}.xml`（及对应的 `_manual.json` /
> `_auto.json`）。sensor 名来自设备树，不在 scene JSON 里配置。

### 字段说明

| 字段               | 说明                                                           | 必需 |
|--------------------|----------------------------------------------------------------|------|
| `name`             | Scene 显示名（仅用于日志）                                     | ✅   |
| `calib_dir`        | 校准文件目录（如 `/etc/vvcam/day`）；空字符串表示使用 daemon 默认 `/etc/vvcam` | ✅   |
| `mode`             | Sensor 模式索引（传给 sensor 驱动的 mode 选择）                | ❌（默认 0） |

> 字段最大长度（超出会被截断）：`name` 31、`calib_dir` 127。

### 样例文件

参考仓库里的 `scene_config_example.json`，包含 day / night 两个 scene。

## ⚠️ 限制与约束

### 1. 只能改参数，不能开关模块

两份 scene 配置之间**允许**的差异：

- ✅ 参数值：曝光、增益、AWB 增益 / 色温曲线、gamma 曲线、CCM、LSC、
  3DNR / 2DNR 强度、去马赛克强度、锐化强度等**任何模块内部的数值参数**。

两份 scene 配置之间**不允许**的差异：

- ❌ ISP 子模块启用状态：比如 scene A 里 WDR 是 on、scene B 里 WDR 是 off；
  或 scene A 打开去闪烁、scene B 关闭；或两个 scene 启用/禁用了不同的
  AE / AWB / 3DNR / 2DNR / 去噪 / 锐化等子模块组合。
- ❌ 任何导致 `libcam_engine` 运行时需要动态加载 / 卸载子模块的 XML 或 JSON
  差异。

**原因**：K230 SDK 使用闭源的 `libcam_engine`。它把一条 `CamEngine` 状态机
绑定到一个 `CamDeviceHandle` 的生命周期。当我们跨 scene 重新 create 出
`CamDeviceHandle` 时，如果两份配置启用的模块集合不同，其内部状态会留在
不一致的形态下，经过若干次切换后会触发 glibc 的 stack canary：

```
*** stack smashing detected ***: terminated
```

**编写多 scene 配置时的规则**：保持所有 scene 启用的子模块集合完全一致，
只在子模块内部调整参数值。如果业务场景确实需要开关模块，唯一可靠的做法
是退出当前应用 → 重启 `isp_media_server` → 以目标配置重新启动应用。

如何判断两份配置是否只改参数：对比两份 XML / manual JSON / auto JSON，
所有 `<ModuleXxx enable="1" .../>` 与 `<ModuleXxx enable="0" .../>` 的
结构必须完全一致，只允许数值字段不同。

### 2. 切换延时

- 走的是"完全重启 video pipe"路径（close → reopen），典型耗时 几百 ms 到
  1 秒量级，屏幕会出现短暂黑屏。
- `apply_scene_config_ctrls()` 之后 demo 里有一次显式 `sleep(1)` 等待
  daemon 把 disconnect / connect 事件处理完，再做 `restart_video_device`。

### 3. 单 sensor 单 pipe

目前 demo 固定打开一个 `/dev/videoX`、一个 DRM plane。多摄 / 多 pipe
切换不在本 demo 范围内。

### 4. 分辨率固定

`-w` / `-h` 只能设置为 sensor 支持的分辨率，且在 scene 切换时不会改变；
如果两份 scene 要求不同分辨率，需要退出程序重启。

## Scene 切换流程

```
按下 1~9 数字键
    ↓
scene_handler()：置 scene_change_requested=true, target_scene_index=N
    ↓
主循环检测到请求
    ↓
apply_scene_config_ctrls()
    └─ VIDIOC_S_EXT_CTRLS  (自定义 CID: MY_CID_BASE=0x00980900 + 0..1)
       └─ 0: calib_dir   1: mode
    ↓
sleep(1)   # 等 isp_media_server 处理 disconnect / connect 事件
    ↓
restart_video_device()
    ├─ full_cleanup()
    │   ├─ VIDIOC_STREAMOFF
    │   ├─ sleep(1)
    │   ├─ close(video_fd)
    │   ├─ 释放 DRM display_buffers
    │   └─ display_exit()
    │
    ├─ setup_device()     open /dev/videoX + VIDIOC_S_FMT
    ├─ setup_display()    display_init + display_get_plane + allocate buffers
    ├─ setup_buffers()    VIDIOC_REQBUFS(DMABUF) + VIDIOC_QBUF x N
    └─ start_capturing()  VIDIOC_STREAMON
    ↓
current_scene_index = target_scene_index
```

> 关键点：**必须走 close+reopen**。在 daemon (isp_media_server) 这一侧，
> close(video_fd) 触发 `MEDIA_EID_DESTROY_PIPELINE`，reopen 触发
> `MEDIA_EID_CREATE_PIPELINE`，从而销毁 + 重建 `CamDeviceHandle`。只做
> STREAMOFF/STREAMON 的轻量重启无法让 libcam_engine 重新加载校准文件。

## Scene 切换 ioctl 约定

demo 使用标准 `VIDIOC_S_EXT_CTRLS` + 一组自定义 private CID 下发 scene 配
置。用户空间不需要自定义 ioctl 号，任何支持 V4L2 的应用都可以用同样的
方式集成（比如 GStreamer 的 `v4l2src`）。

| Control ID               | 类型 / size     | 含义                    |
|--------------------------|-----------------|-------------------------|
| `MY_CID_BASE + 0` (0x00980900) | string          | `calib_dir`（校准目录，空则用 daemon 默认） |
| `MY_CID_BASE + 1` (0x00980901) | integer         | `mode`（sensor mode，同步到 procfs）   |

- `S_EXT_CTRLS`：内核保存 `calib_dir`/`mode` 供 G_EXT_CTRLS，并通过 `VVCAM_ISP_EVENT_SCENE_CONFIG` 通知 daemon
- `G_EXT_CTRLS`：读回当前 scene 配置（对应 `g` 热键）
- procfs **不** 显示 `calib_base`；daemon 按 `{calib_dir}/{sensor}-{W}x{H}.*` 拼接并加载

## 使用示例

### 启动

```bash
$ ./v4l2-drm-scene -d 0 -w 640 -h 480 -s /etc/vvcam/scene_config.json

v4l2-drm-scene: device=0, width=640, height=480
Loading scene config: /etc/vvcam/scene_config.json

Available scenes (3):
  [1] day: calib_dir=/etc/vvcam/day, mode=0
  [2] night: calib_dir=/etc/vvcam/night, mode=0
  [3] default: calib_dir=/etc/vvcam/default, mode=0

Press 1-3 to switch scenes, 'g' to get config, 'q' to quit
Running...
FPS: 30.00
```

### 切换到 night

```
# 按 '2'

Switch to scene 2 (night) requested
Applying scene 2 config...
Restarting video device (full: close -> reopen)...
Cleaning up all resources...
  - Stopped capturing
  - Closed video fd
  - Freed display buffers
  - Exited display
Cleanup complete
  - Opened video device
  - Set format
  - Setup display
  - Setup buffers
  - Started capturing
Video device restarted successfully
✓ Switched to scene 2: night
FPS: 30.00
```

### 读回当前配置

```
# 按 'g'

Getting current scene config...

=== Current Scene Configuration ===
  Calib Dir:       /etc/vvcam/night
  Mode:            0
====================================
```

## 架构

```
┌─────────────────────────────────────┐
│  User Space                          │
│                                      │
│  v4l2-drm-scene                      │
│   ├─ parse_scene_config (JSON)       │
│   ├─ scene_handler (stdin hotkeys)   │
│   ├─ apply_scene_config_ctrls        │
│   │    → VIDIOC_S_EXT_CTRLS          │
│   └─ restart_video_device            │
│        → close/reopen /dev/videoX    │
└──────────────┬───────────────────────┘
               │
               │  V4L2 ioctl / DMA-BUF
               ▼
┌─────────────────────────────────────┐
│  Kernel                              │
│   vvcam_video + vvcam_isp + sensor   │
│   (/dev/videoX, /dev/v4l-subdev...)  │
│                                      │
│   procfs: /proc/vsi/isp_subdev<N>    │
│   → 保存 S_EXT_CTRLS 下来的配置      │
└──────────────┬───────────────────────┘
               │
               │  read /proc/vsi/..., MEDIA events
               ▼
┌─────────────────────────────────────┐
│  isp_media_server (user-space daemon)│
│   ├─ 监听 MEDIA_EID_CREATE/DESTROY   │
│   │  PIPELINE 事件                   │
│   ├─ 从 /proc/vsi/isp_subdev<N> 读入 │
│   │  当前 scene 配置                 │
│   └─ 调用 libcam_engine 加载 XML +   │
│      manual/auto JSON，驱动 3A       │
└─────────────────────────────────────┘
```

显示侧：

```
v4l2-drm-scene → display lib → DRM (/dev/dri/cardX)
                                 │
                                 ▼
                         DRM plane (NV12)
```

## 代码结构

```
main.c
├── 类型 / 状态
│   ├── struct scene_config        每个 scene 的配置项
│   └── struct scene_context       运行时上下文（video_fd, display, buffers, ...）
│
├── 配置解析
│   ├── parse_scene_config()       行式 JSON 解析
│   └── print_available_scenes()
│
├── V4L2 / DRM 初始化
│   ├── setup_device()             open + VIDIOC_S_FMT
│   ├── setup_display()            display_init + allocate DRM buffers
│   ├── setup_buffers()            REQBUFS + QBUF
│   ├── start_capturing()          STREAMON
│   └── stop_capturing()           STREAMOFF
│
├── Scene 切换
│   ├── apply_scene_config_ctrls() VIDIOC_S_EXT_CTRLS 下发 2 个 private CID
│   ├── get_scene_config()         VIDIOC_G_EXT_CTRLS 读回
│   ├── print_current_scene()
│   ├── full_cleanup()             STREAMOFF + close + release DRM
│   └── restart_video_device()     full_cleanup + setup_* + start_capturing
│
├── 帧处理
│   ├── process_frame()            DQBUF → buffer_hold → QBUF
│   └── update_display()           display_update_buffer + display_commit
│
├── 输入
│   └── scene_handler()            stdin 非阻塞读取热键
│
└── 生命周期
    ├── cleanup()                  退出时释放
    └── main()                     参数解析 + 主循环
```

## 依赖

- **display lib**：本仓库的 DRM 显示封装
- **V4L2** (`linux/videodev2.h`)
- **DRM / libdrm** (`drm_fourcc.h`)
- **`isp_media_server`** daemon：运行时必须在后台运行（由
  `/etc/init.d/S31canaan_isp` 启动），否则 ISP 不会对下发的 scene 配置做出反应
- **内核模块**：`vvcam_isp`、`vvcam_mipi`、`vvcam_vb`、`vvcam_isp_subdev`、
  `vvcam_video`，在 S31 init 脚本里一起 modprobe

## 注意事项

1. **Scene 数量**：最多 10 个（但只有 1~9 数字键可触发切换，所以可用热键切换的上限是 9 个）。
2. **切换时间**：完整 close+reopen 路径，黑屏数百 ms 到 1 秒量级，本 demo 不做优化。
3. **分辨率**：必须是 sensor 支持的模式之一；demo 运行过程中不支持改分辨率。
4. **配置文件路径**：每条 scene 的 `calib_dir` 目录下需存在
   `{sensor}-{W}x{H}.xml` / `_manual.json` / `_auto.json`（sensor 来自设备树），
   否则 daemon 加载会失败（日志在 `/tmp/isp.err.log`）。
5. **模块开关一致性**：再次强调——只改参数，不改启用模块集合。

## 故障排除

### Scene 切换失败 / 黑屏不恢复

1. 确认 JSON 里每条 scene 的 `calib_dir` 下存在对应分辨率的校准文件
   （如 `ls /etc/vvcam/day/gc2093-1920x1080.*`）。
2. 确认 `isp_media_server` 在跑：`ps -ef | grep isp_media_server`；如果不在，
   查 `/tmp/isp.err.log`。
3. 确认 ISP 相关内核模块都已加载：`lsmod | egrep 'vvcam|nonai2d'`。

### 切换若干次后出现 "stack smashing detected"

命中了 libcam_engine 的限制（见"限制与约束"第 1 条）。先用两份配置直接
比对，如果两份 JSON / XML 里有任何 `enable=0 / enable=1` 不一致的模块，
把它们统一回同一个启用集合，只在启用的模块里改参数。

### 无法打开 video 设备

```bash
ls -l /dev/video*       # 设备是否存在
dmesg | grep vvcam      # 驱动是否加载成功
```

### DRM 初始化失败

```bash
ls -l /dev/dri/card*
```

### 查看 ISP / 驱动日志

```bash
dmesg | grep -E 'vvcam|isp'
cat /proc/vsi/isp_subdev0          # sensor / mode / i2c_bus（无 calib_base）
cat /tmp/isp.err.log               # daemon 的 stderr
```

## 调试技巧

- **当前 scene**：按 `g` 读回 `calib_dir` / `mode`（G_EXT_CTRLS）；daemon 日志可见实际加载的 xml/json 路径
- **FPS**：程序运行时在 stderr 输出。
- **对比模块集合**：
  ```bash
  diff <(grep -oE 'Module\w+[^>]*enable="[01]"' /etc/vvcam/day/gc2093-1920x1080.xml | sort) \
       <(grep -oE 'Module\w+[^>]*enable="[01]"' /etc/vvcam/night/gc2093-1920x1080.xml | sort)
  ```
  输出应为空——任意一行 `enable` 字段不同就意味着踩到上面那条限制。

---

**状态**：✅ 可用（受 libcam_engine 模块开关限制）
**版本**：3.1（多 scene + 约束文档化）
**最大 Scene 数**：10（热键可切 9 个）
**支持矩阵**：参数切换 ✅ 无限次可切；模块开关切换 ❌ 禁止

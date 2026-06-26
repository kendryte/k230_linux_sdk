# k230_assistant

## 1. 概述

k230_assistant 是使用k230作为客户端，使用云端LLM实现实时语音对话的项目。客户端和LLM服务使用webRTC建立连接，实现实时音视频流传输。

## 2. 源码说明

`libpeer` 库使用开源库(https://github.com/sepfy/libpeer)实现 WebRTC 创建连接和协商过程。`libpeer` 库的编译依赖于第三方库 `cjson`、`srtp2`、`usrsctp`、 `mbedtls`、 `mbedcrypto`、 `mbedx509`，因此先编译第三方库，库文件在`dist/lib`下，对应的头文件在`dist/include`。`libpeer` 的源码为`libpeer/libpeer/src`，头文件为`libpeer/libpeer/include`，编译生成的所有库在`dist/lib`目录下，头文件在`dist/include`目录下。

`http` 请求使用`libcurl`实现，数据序列化使用`cJSON`实现。LLM 端提供云端实时对话服务。

### 3. 代码树

下面是代码文件的说明：

```shell
k230_assistant
├── config
├── dist
│    ├── include
│    └── lib
├── libpeer
├── src
│    ├── CMakeLists.txt
│    ├── get_pcm.cc
│    ├── get_pcm.h
│    ├── http.cc
│    ├── http.h
│    ├── llm_inference.h
│    ├── main.cc
│    ├── media.cc
│    ├── media.h
│    ├── openai.cc
│    ├── play_pcm.cc
│    ├── play_pcm.h
│    ├── webrtc_audio_enhance.cc
│    ├── webrtc_audio_enhance.h
│    ├── webrtc_audio_improvement.cc
│    ├── webrtc_audio_improvement.h
│    ├── webrtc.cc
│    ├── webrtc.h
├── CMakeLists.txt
└── build_app.sh
```

下表对代码文件进行简单介绍：

| 文件名称                    |         简介     |
|--------------------------- | -----------------|
|main.cc|主函数，先启动k230上的音频配置，初始化音频编解码器|
|llm_interface.cc|大模型实时API服务接口方法|
|openai.cc| 使用 webrtc 调用 openai realtime api|
|http.h| 发送http请求头文件|
|http.cc| 按照不同的云端模型组织http请求并发送，现在仅支持openai|
|media.h|多媒体头文件，定义音频采集初始化，音频编解码，编码发送和解码播放的接口|
|media.cc|音频采集初始化，音频编解码，编码发送和解码播放的接口实现；|
|get_pcm.h|配置音频采集设备，从麦克风采集音频数据的接口|
|get_pcm.cc|音频采集设备初始化、音频采集PCM数据、反初始化接口实现|
|play_pcm.h|配置音频播放设备，播放解码器解码的音频数据接口|
|play_pcm.cc|音频播放设备的初始化、音频播放PCM数据、反初始化接口实现|
|webrtc_audio_enhance.h|使用webrtc音频处理库封装音频回声抑制、噪声抑制、音频增益的接口|
|webrtc_audio_enhance.cc|使用webrtc音频处理库进行音频回声抑制、噪声抑制、音频增益的接口实现代码|
|webrtc_audio_improvement.h|封装的音频数据处理的接口|
|webrtc_audio_improvement.cc|封装的音频数据处理的接口代码实现|
|webrtc.h|webrtc初始化接口|
|webrtc.cc|创建PeerConnection配置参数peer_connection_config并创建PeerConnection实例，创建本地SDP，设置peer_connection连接状态初始化，循环查询peer_connection状态，根据状态的不同分别开启，创建连接，握手，发送数据，接收数据的过程，设置音频播放函数play_audio作为收到数据回调函数, 并按照状态启动音频发送线程|


## 4. 编译说明

### 4.1 环境搭建

已经在k230_canmv_defconfig/k230_canmv_01studio_defconfig/k230_canmv_lckfb_defconfig/k230_canmv_v3/defconfig/k230d_canmv_defconfig/BPI-CanMV-K230D-Zero_defconfig几种配置中增加了对应依赖，可以跳过此步骤，直接编译使用。

使用如下命令搭建linux_sdk环境：

```shell
#下载k230_linux_sdk
git clone https://g.a-bug.org/maix_sw/k230_linux_sdk.git

#下载编译工具链
wget https://occ-oss-prod.oss-cn-hangzhou.aliyuncs.com/resource//1721095219235/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.1-20240712.tar.gz

# 解压工具链到/opt/toolchain
mkdir -p /opt/toolchain;
tar -zxvf Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.1-20240712.tar.gz -C /opt/toolchain;

#安装依赖
apt-get install -y   git sed make binutils build-essential diffutils gcc  g++ bash patch gzip \
        bzip2 perl  tar cpio unzip rsync file  bc findutils wget  libncurses-dev python3  \
        libssl-dev gawk cmake bison flex  bash-completion parted curl

# make menuconfig 查询并配置libcurl,cJSON,openssl,opus,ntp，webrtc_audio_processing
BR2_PACKAGE_OPUS=y
BR2_PACKAGE_LIBCURL=y
BR2_PACKAGE_LIBCURL_CURL=y
BR2_PACKAGE_LIBCURL_VERBOSE=y
BR2_LINUX_KERNEL_NEEDS_HOST_OPENSSL=y
BR2_PACKAGE_CJSON=y
BR2_PACKAGE_CJSON_UTILS=y
BR2_PACKAGE_NTP=y
BR2_PACKAGE_NTP_SNTP=y
BR2_PACKAGE_NTP_NTP_KEYGEN=y
BR2_PACKAGE_NTP_NTPDATE=y
BR2_PACKAGE_WEBRTC_AUDIO_PROCESSING=y

# 编译
make CONF=k230_canmv_defconfig  BR2_PRIMARY_SITE=https://download.kendryte.com/k230/downloads/dl/

make CONF=k230_canmv_01studio_defconfig

make CONF=k230_canmv_lckfb_defconfig
```
### 4.2 编译源码

进入`k230_linux_sdk/buildroot-overlay/package/`目录，**先编译库文件，再编译业务源码**：

```shell
# 进入库目录
cd k230_assistant/libpeer

# 编译库文件
./build_lib.sh

cd ..

# 查看编译好的库
tree dist

# 编译源码
./build.sh
```

编译后的文件为`k230_assistant/k230_bin/k230_assistant.elf`，`k230_bin` 目录下还包含`config.json`用于在免编译的前提下配置openai的url和key。

### 4.3 配置URL和Key

请您自行添加直连OpenAI API KEY，k230语音助手使用 `openai` 提供的`realtime api`，需要您**具备访问`openai api`的网络环境**和直连`key`值；**您可以在`configs/config.json`中进行替换**。

### 4.4 代码运行

烧录编译的k230 linux固件，拷贝编译后的`k230_assistant.elf`和`config.json`到某一目录下，执行下述命令：

```
./k230_assistant.elf
```

> **注意：**
> 如果连接失败，请输入`q`回车或者使用`ctrl+C`退出。

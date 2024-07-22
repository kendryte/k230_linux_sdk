# k230/k230d linux sdk快速入门

## 1.k230/k230d芯片

k230系列芯片采用全新的多异构单元加速计算架构，集成了2个RISC-V高能效计算核，内置新一代KPU（Knowledge Process Unit）智能计算单元，具备多精度AI算力，广泛支持通用的AI计算框架，部分典型网络的利用率超过了70%。

芯片同时具备丰富多样的外设接口，以及2D、2.5D等多个标量、向量、图形等专用硬件加速单元，可以对多种图像、视频、音频、AI等多样化计算任务进行全流程计算加速，具备低延迟、高性能、低功耗、快速启动、高安全性等多项特性。

![K230_block_diagram](https://developer.canaan-creative.com/k230_canmv/main/_images/K230_block_diagram.png)

>k230和k230d的主要区别是k230d内部多一个128MB的lpddr4颗粒；

## 2.sdk源码及编译

### 获取sdk代码

参考如下命令下载sdk代码

```bash
git clone git@github.com:kendryte/k230_linux_sdk.git
# git clone git@gitee.com:kendryte/k230_linux_sdk.git
cd k230_linux_sdk
```

>github上仓库地址是 git@github.com:kendryte/k230_linux_sdk.git
>
>gitee上仓库地址是 git@gitee.com:kendryte/k230_linux_sdk.git

### 安装交叉工具链
下载Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.1-20240712.tar.gz （下载链接https://www.xrvm.cn/community/download?id=4333581795569242112 ）文件，并解压缩到/opt/toolchain目录  (Refer command):

```bash
mkdir -p /opt/toolchain;
tar -zxvf Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.1-20240712.tar.gz -C /opt/toolchain;
```
> 安装新32位交叉工具链(可选 只有k230d_canmv_ilp32_defconfig配置需要)
>
> wget -c xxxx;mkdir -p  /opt/toolchain;tar -xvf xxx.tar.gz -C /opt/toolchain/
### 安装依赖

需要安装如下软件的 ubuntu22.04 或者ubuntu 24.04系统(参考安装命令)

```bash
sudo apt-get inst wget all -y git sed make binutils build-essential diffutils gcc  g++ bash patch gzip bzip2 perl tar cpio unzip rsync file bc findutils wget libncurses-dev python3 libssl-dev  gawk cmake bison flex bash-completion
```

>依赖软件包见tools/docker/Dockerfile 文件，构建和进入docker环境参考如下命令：
>
>docker  build   -f tools/docker/Dockerfile  -t wjx/d tools/docker  #构建
>
>docker run -it  -h k230  -e uid=\$(id -u) -e gid=\$(id -g) -e user=\${USER} -v \${HOME}:\${HOME}  -w \$(pwd) wjx/d:latest   #使用

### 编译

参考下面命令进行编译

```bash
make CONF=k230d_canmv_defconfig #build k230d canmv image (kernel and rootfs both 64bit);
# make CONF=k230_canmv_defconfig # build k230 canmv image
# make CONF=k230d_canmv_ilp32_defconfig  #build k230d canmv 32bit rootfs;
# make CONF=k230d_canmv_64kernel_32rootfs_defconfig #build k230d 64bit kernel  32bit rootfs image
```

>k230d_canmv_defconfig是个例子，需要替换为正确的配置文件，比如替换为k230_canmv_defconfig
>
>sdk支持的所有配置文件见buildroot-overlay/configs目录
>
>make CONF=k230d_canmv_defconfig  含义是使用k230d_canmv_defconfig配置文件

### 编译输出文件

output/k230d_canmv_defconfig/images/sysimage-sdcard.img.gz

>从嘉楠官网下载的就是这个文件，烧录前需要解压缩，烧录方法见后面
>
>k230d_canmv_defconfig 是个例子，请根据编译配置文件替换为正确名字



## 3.镜像烧写

### 镜像获取

镜像可以使用编译生成的文件，也可以从嘉楠官网 https://developer.canaan-creative.com/resource下载，

如果从官网下载，需要下载“k230d-canmv”开头的gz压缩包，解压缩得到sysimage-sdcard.img文件，即为k230d-canmv镜像。

>需要解压缩

### linux下烧录

在TF卡插到宿主机之前，输入：

```
ls -l /dev/sd\*
```

查看当前的存储设备。

将TF卡插入宿主机后，再次输入：

```
ls -l /dev/sd\*
```

查看此时的存储设备，新增加的就是TF卡设备节点。

假设/dev/sdc就是TF卡设备节点，执行如下命令烧录TF卡：

```
sudo dd if=sysimage-sdcard.img of=/dev/sdc bs=1M oflag=sync
```



### windows下烧录

Windows下可通过rufus工具对TF卡进行烧录（rufus工具下载地址 `http://rufus.ie/downloads/`）。

1）将TF卡插入PC，然后启动rufus工具，点击工具界面的”选择”按钮，选择待烧写的固件。

![rufus-flash-from-file](https://developer.canaan-creative.com/k230_canmv/main/_images/rufus_select.png)



4)启动：

把tf卡插入开发板，给开发板上电(usb接入电脑)，按下reset按键(可选)，

电脑上的串口软件可以看到打印

>板子连接及查看方法见后面

## 4.开发板连接

>开发板有差异，请参考正确的章节连接各个开发板

### 2.1k230d-canmv开发板连接

1)参考下图连接k230d-canmv开发板

![](pic\CanMV-K230_front.png)



>补充：需要硬件说明：
>typec usb线
>tf卡；



2)电脑上确认串口号：

设备上电后电脑上会多出两个串口，

windows串口显示如下：

![CanMV-K230-micropython-serial](https://developer.canaan-creative.com/k230_canmv/main/_images/CanMV-K230-micropython-serial.png)

USB-Enhanced-SERIAL-A CH342（COM80）为小核linux调试串口

USB-Enhanced-SERIAL-B CH342（COM81）为串口3--暂未使用



linux系统下串口显示如下：

/dev/ttyACM0为小核linux调试串口

/dev/ttyACM1为 为串口3--暂未使用



>不插tf卡，也可以看到串口。



3)查看串口输出

使用串口软件查看开发板串口输出



>开发板串口参数是
>不插tf卡，按下reset按键，默认串口也会输出一行打印，看到打印说明cpu工作正常。
>推荐串口软件是putty，其他串口软件(比如moblxterm xshell securecrt等)也可以。



### 2.2k230-canmv开发板连接

1)参考下图连接k230d-canmv开发板

![](Y:\k230_linux_sdk\docs\pic\CanMV-K230_front.png)



>补充：需要硬件说明：
>typec usb线
>tf卡；



2)电脑上确认串口号：

设备上电后电脑上会多出两个串口，

windows串口显示如下：

![CanMV-K230-micropython-serial](https://developer.canaan-creative.com/k230_canmv/main/_images/CanMV-K230-micropython-serial.png)

USB-Enhanced-SERIAL-A CH342（COM80）为小核linux调试串口

USB-Enhanced-SERIAL-B CH342（COM81）为串口3--暂未使用



linux系统下串口显示如下：

/dev/ttyACM0为小核linux调试串口

/dev/ttyACM1为 为串口3--暂未使用



>不插tf卡，也可以看到串口。



3)查看串口输出

使用串口软件查看开发板串口输出



>开发板默认串口参数是 115200
>不插tf卡，按下reset按键，默认串口也会输出一行打印，看到打印说明cpu工作正常。
>推荐串口软件是putty，其他串口软件(比如moblxterm xshell securecrt等)也可以。


## 5.sdk构建解析(可选)

### 说明

本sdk基于2024.02版本的buildroot进行构建，

### SDK目录结构

```shell
k230_linux_sdk/
├── buildroot-overlay  #buildroot 有修改的文件，会用这个目录覆盖原始的buildroot目录。
│   ├── board
│   │   └── canaan  #嘉楠k230相关板子的私有脚本 配置文件等
│   ├── boot
│   │   ├── opensbi  #opensbi有修改的文件
│   │   └── uboot  #uboot有修改的文件
│   ├── configs
│   │   ├── k230d_canmv_defconfig  #k230d canmv板子配置文件
│   │   └── k230_canmv_defconfig  #k230 canmv板子配置文件
│   └── package
│       ├── libdrm  #libdrm有修改的文件
│       ├── lvgl  #lvgl有修改的文件
│       ├── vg_lite
│       └── vvcam
├── docs  #文档目录
├── output   #输出目录，包含最终使用的源码，及所有的输出文件
│   ├── buildroot-2024.02.1  #最后使用的buildroot目录
│   └── k230d_canmv_defconfig  #编译输出目录，所有源代及编译输出文件
├── Makefile  #主makefile文件
├── README.md  #readme文件
└── tools  #一些脚本工具
```

### sdk编译过程概述

>本节以make CONF=k230d_canmv_defconfig 命令执行过程为例。

1)从buildroot官网下载buildroot-2024.02.1.gz压缩包，并解压缩为output/buildroot-2024.02.1/

2)用buildroot-overlay目录覆盖output/buildroot-2024.02.1/目录

```shell
rsync -a  buildroot-overlay/ output/buildroot-2024.02.1/
```

3)进入output/buildroot-2024.02.1/目录，使用k230d_canmv_defconfig配置buildroot，并指定输出目录为output/k230d_canmv_defconfig

```
make -C output/buildroot-2024.02.1 k230d_canmv_defconfig O=/home/wangjianxin/k230_linux_sdk/output/k230d_canmv_defconfig
```

4)进入output/k230d_canmv_defconfig 目录并进行编译

```shell
make -C /home/wangjianxin/k230_linux_sdk/output/k230d_canmv_defconfig all
```

>更多编译说明请参考https://buildroot.org/downloads/manual/manual.html

## 6.sdk 应用开发参考



### 编译第一个程序：hello

创建内容如下的hello.c文件

```c
//hello.c 文件内容
#include <stdio.h>
int main()
{
    printf("Hello, World!\n");
    return 0;
}

```

编译程序

```shell
output/k230d_canmv_defconfig/host/bin/riscv64-linux-gcc hello.c -o hello
```



把hello文件复制到开发板上，并执行,可以看到打印

```

```



### linux gpio demo(未完成)





### linux lvgl demo(未完成)



### vector向量demo(未完成)





## 7.rootfs定制及源码修改(未完成)

### rootfs定制：



### uboot代码修改及编译：



### opensbi代码修改及编译：



### linux kernel代码修改及编译：







参考资料：

https://developer.canaan-creative.com/k230_canmv/main/zh/CanMV-K230%E5%BF%AB%E9%80%9F%E5%85%A5%E9%97%A8%E6%8C%87%E5%8D%97.html

https://d1.docs.aw-ol.com/d1_dev/

https://yoc.docs.t-head.cn/icebook/

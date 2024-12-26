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
下载Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.2-20240712.tar.gz （下载地址1：https://www.xrvm.cn/community/download?id=4352528597269942272 ，下载地址2：https://occ-oss-prod.oss-cn-hangzhou.aliyuncs.com/resource/1836682/1725612383347/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.2-20240904.tar.gz）文件，并解压缩到/opt/toolchain目录  (参考命令如下):

```bash
mkdir -p /opt/toolchain;
tar -zxvf Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.2-20240712.tar.gz -C /opt/toolchain;
```
> 安装新32位交叉工具链（下载地址：https://github.com/ruyisdk/riscv-gnu-toolchain-rv64ilp32/releases/download/2024.06.25/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25-nightly.tar.gz）(可选, 只有k230d_canmv_ilp32_defconfig配置需要)
>
> wget -c https://github.com/ruyisdk/riscv-gnu-toolchain-rv64ilp32/releases/download/2024.06.25/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25-nightly.tar.gz ;
>
> mkdir -p  /opt/toolchain/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25/ ;
>
> tar -xvf riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25-nightly.tar.gz -C  /opt/toolchain/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25/
### 安装依赖

需要安装如下软件的 ubuntu22.04 或者ubuntu 24.04系统(参考安装命令)

```bash
sudo apt-get inst wget all -y git sed make binutils build-essential diffutils gcc  g++ bash patch gzip bzip2 perl tar cpio unzip rsync file bc findutils wget libncurses-dev python3 libssl-dev  gawk cmake bison flex bash-completion
```

>依赖软件包见tools/docker/Dockerfile 文件，构建和进入docker环境参考如下命令：
>
>docker  build   -f tools/docker/Dockerfile  -t wjx/d tools/docker  #构建
>
>docker run -it --rm  -h k230  -e uid=\$(id -u) -e gid=\$(id -g) -e user=\${USER} -v \${HOME}:\${HOME}  -w \$(pwd) wjx/d:latest   #使用

### 编译

参考下面命令进行编译

```bash
make CONF=k230d_canmv_defconfig #build k230d canmv image (kernel and rootfs both 64bit)
#make CONF=k230_canmv_01studio_defconfig # build 01studio canmv board
# make CONF=k230_canmv_defconfig # build k230 canmv image
# make CONF=k230d_canmv_ilp32_defconfig  #build k230d canmv 32bit rootfs;
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

>板子连接及串口查看方法见后面章节

## 4.开发板连接

>开发板有差异，请参考对应章节连接各个开发板

### 说明

本sdk主要支持的开发板及配置文件说明

| 开发板         | 配置文件                      | 说明                            |
| -------------- | ----------------------------- | ------------------------------- |
| canmv          | k230_canmv_defconfig          | k230 canmv board                |
| evb            | k230_evb_defconfig            | k230 evb board                  |
| canmv_01studio | k230_canmv_01studio_defconfig | 01studio  canmv board           |
| k230d_canmv    | k230d_canmv_defconfig         | k230d canmv board               |
| k230d_canmv    | k230d_canmv_ilp32_defconfig   | k230d canmv board 32bit systerm |



### k230-canmv开发板连接

1)参考下图使用Type-C连接k230-canmv开发板

使用Type-C线连接CanMV-K230如下图的位置，线另一端连接至电脑

![debug](https://gitee.com/kendryte/k230_docs/raw/main/zh/images/CanMV-K230_debug.png)

>最少硬件：
>typec usb线
>tf卡



2)电脑上确认串口号：

设备上电后电脑上会多出两个串口，

windows串口显示如下：

![CanMV-K230-micropython-serial](https://developer.canaan-creative.com/k230_canmv/main/_images/CanMV-K230-micropython-serial.png)

USB-Enhanced-SERIAL-A CH342（COM80）为小核linux调试串口

USB-Enhanced-SERIAL-B CH342（COM81）为串口3--暂未使用

>windows下如果串口识别错误，请重新安装ch342驱动(下载地址：https://www.wch.cn/downloads/CH343SER_EXE.html)

linux系统下串口显示如下：

/dev/ttyACM0为小核linux调试串口

/dev/ttyACM1为 为串口3--暂未使用



>不插tf卡，也可以看到串口。



3)查看串口输出

使用串口软件查看开发板串口输出

>开发板默认串口参数:波特率115200，数据位 8，停止位1，奇偶检验 无，流控 无
>不插tf卡，按下reset按键，默认串口也会输出一行打印，看到打印说明cpu工作正常。
>推荐串口软件是putty，其他串口软件(比如moblxterm xshell securecrt等)也可以。



### k230d-canmv开发板连接(待更新图片)

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

> windows下如果串口识别错误，请重新安装ch342驱动(下载地址：https://www.wch.cn/downloads/CH343SER_EXE.html)

linux系统下串口显示如下：

/dev/ttyACM0为小核linux调试串口

/dev/ttyACM1为 为串口3--暂未使用

>不插tf卡，也可以看到串口。

3)查看串口输出

使用串口软件查看开发板串口输出



>开发板默认串口参数:波特率115200，数据位 8，停止位1，奇偶检验 无，流控 无
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
/opt/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.2/bin/riscv64-unknown-linux-gnu-gcc hello.c  -o hello
```



把hello文件复制到开发板上，并执行,可以看到打印正确

```
[root@canaan ~ ]#./hello
Hello, World!
[root@canaan ~ ]#
```
>可以通过scp或者rz命令复制到开发板上


### gpio (未完成)



### uart

说明：

向对应串口发送数据，并且读取串口返回的数据，类似发送AT指令，并且读取AT指令返回值



代码

```c
//uart.c文件内容
#include "stdio.h"
#include "string.h"
#include <sys/prctl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <arpa/inet.h>

#include <fcntl.h>    // 包含 open 函数
#include <unistd.h>   // 包含 read 和 write 函数
#include <termios.h>



int  main(int argc ,char * argv[])
{
    int fd;
    char *devname=(char*)"/dev/ttyS3";
    char sendcmd[1024]= "AT\r";
    //char *sendcmd = (char*)"AT\r\n" ;//"AT+QCFG=\"usbnet\" \r\n";
    char read_buf[1024];
    int n_read;
    int n_written ;

    struct termios options,options_bak;  // 串口配置结构体

    if(argc > 1)
        devname=argv[1];
    if(argc >2)
        snprintf(sendcmd, sizeof(sendcmd),  "%s\r", argv[2]);


    fd = open(devname, O_RDWR|O_NONBLOCK|O_NOCTTY|O_SYNC|O_NDELAY ); // 打开串口，O_RDWR 表示读写模式
    //   O_NOCTTY 不允许进程管理串口（不太理解，一般都选上）；
    //   O_NDELAY 非阻塞（默认为阻塞，打开后也可以使用fcntl()重新设置）
    if (fd < 0) {
        perror("open_port: Unable to open /dev/ttyS0 - ");
        return(-1);
    }

    tcgetattr(fd,&options_bak); //获取当前设置
    memcpy(&options,&options_bak,sizeof(options));

    //printf("i=%x %x %x \n",options.c_iflag,options.c_oflag,options.c_cflag);

    options.c_cflag  |= B9600 | CLOCAL | CREAD; // 设置波特率，本地连接，接收使能
    options.c_cflag &= ~CSIZE; //屏蔽数据位
    options.c_cflag  |= CS8; // 数据位为 8 ，CS7 for 7
    options.c_cflag &= ~CSTOPB; // 一位停止位， 两位停止为 |= CSTOPB
    options.c_cflag &= ~PARENB; // 无校验
    options.c_cc[VTIME] = 0; // 等待时间，单位百毫秒 （读）。后有详细说明
    options.c_cc[VMIN] = 0; // 最小字节数 （读）。后有详细说明


    options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

        // 设置输入输出控制模式
    options.c_iflag &= ~(ICRNL | INLCR | IGNCR | IXON | IXOFF); // 关闭 INLCR, IGNCR, IXON, IXOFF
    options.c_oflag &= ~(ONLCR); // 关闭 ONLCR
    options.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON | ISIG | IEXTEN); // 关闭 ECHO, ECHOE, ECHOK, ECHONL, ICANON, ISIG, IEXTEN



    tcflush(fd, TCIOFLUSH); // TCIFLUSH刷清输入队列。
    tcsetattr(fd, TCSANOW, &options); // TCSANOW立即生效；
    usleep(1000*1);
    //printf("after i=%x %x %x \n",options.c_iflag,options.c_oflag,options.c_cflag);


    n_written = write(fd, sendcmd, strlen(sendcmd));
    printf("f=%s l=%d %d,send:%s\n", __func__, __LINE__, n_written, sendcmd);
    fsync(fd);
    usleep(1000*300);
    do{
        n_read = read(fd, read_buf, sizeof(read_buf));
        if(n_read<=0)
            break;
        read_buf[n_read] = '\0'; // 确保字符串以空字符结尾
        printf("%s\n", read_buf);
        usleep(1000);
    }while(1);

    tcflush(fd, TCIOFLUSH); // TCIFLUSH刷清输入队列。
    tcsetattr(fd, TCSANOW, &options_bak); // TCSANOW立即生效；
    close(fd);
    return 0;
}

```



### lvgl (未完成)



### vector向量

RVV（RISC-V Vector Extension）是指RISC-V指令集架构的向量扩展。RISC-V是一种开源的指令集架构，它的设计简洁、可扩展性强，并且具有广泛的应用领域。RVV作为RISC-V的一个可选扩展，旨在支持向量处理和并行计算。RVV定义了一组新的指令，用于执行向量操作。这些指令允许同时处理多个数据元素，从而提高计算效率和吞吐量。向量操作可以在单个指令中执行，而不需要通过循环或逐个操作来处理每个数据元素。RVV支持不同的向量长度，可以根据应用的需求选择不同的向量长度。向量长度可以是固定的，也可以是可配置的。RVV还支持不同的数据类型，包括整数、浮点数和定点数等。

RVV的引入为处理器提供了向量处理和并行计算的能力，可以加速各种应用，如图像处理、信号处理、机器学习、科学计算等。同时，RVV的开放和可扩展性也使得各个厂商和开发者根据自己的需求进行定制和优化。K230 采用的是玄铁C908双核处理器,其中大核C908带了RVV1.0扩展。



代码

```c
//保存文件为rvv.c
#include <riscv_vector.h>
#include <stdio.h>

int main() {
  int avl = 70;
  int vl = vsetvl_e32m2(avl);
  printf("vl: %d\n", vl);

  return 0;
}
```

编译：

```bash
/opt/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.2/bin/riscv64-unknown-linux-gnu-gcc  -march=rv64gcv_xtheadc rvv.c -o rvv
```

开发板上运行：

```bash
[root@canaan ~ ]#scp  wangjianxin@10.10.1.94:/home/wangjianxin/k230_linux_sdk/rvv .
wangjianxin@10.10.1.94's password:
rvv                                           100%   14KB   2.1MB/s   00:00
[root@canaan ~ ]#./rvv
vl: 8
[root@canaan ~ ]#

```



## 7.rootfs定制及源码修改(未完成)

### rootfs定制：



### uboot代码修改及编译：



### opensbi代码修改及编译：



### linux kernel代码修改及编译：







参考资料：

https://developer.canaan-creative.com/k230_canmv/main/zh/CanMV-K230%E5%BF%AB%E9%80%9F%E5%85%A5%E9%97%A8%E6%8C%87%E5%8D%97.html

https://d1.docs.aw-ol.com/d1_dev/

https://yoc.docs.t-head.cn/icebook/

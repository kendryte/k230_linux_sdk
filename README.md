# K230 Linux SDK

>This document uses the k230d_canmv_defconfig configuration as an example. If you use other configurations, replace k230d_canmv_defconfig with correct names. All the configurations supported by this sdk,  can be found in  buildroot-overlay/configs directory.

## Install toolchain and dependencies

download  Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.1-20240712.tar.gz toolchan from  `https://www.xrvm.cn/community/download?id=4333581795569242112`
uncompress the toolchain to the /opt/toolchain(Refer command):

```bash
mkdir -p /opt/toolchain;
tar -zxvf Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.1-20240712.tar.gz -C /opt/toolchain;
```

install dependencies(Refer command):

```bash
apt-get install -y   git sed make binutils build-essential diffutils gcc  g++ bash patch gzip \
        bzip2 perl  tar cpio unzip rsync file  bc findutils wget  libncurses-dev python3  \
        libssl-dev gawk cmake bison flex  bash-completion
```

>k230d_canmv_ilp32_defconfig required  ubuntu 22.04 or 24.04 and install rv64ilp32 toolchain(refer command):
>
>`wget -c https://github.com/ruyisdk/riscv-gnu-toolchain-rv64ilp32/releases/download/2024.06.25/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25-nightly.tar.gz;`
>
>`mkdir -p /opt/toolchain/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25/;`
》
>`tar -xvf riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25-nightly.tar.gz   -C /opt/toolchain/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25/;`

## build

```bash

make CONF=k230d_canmv_defconfig #build k230d canmv image (kernel and rootfs both 64bit);
# make CONF=k230_canmv_defconfig # build k230 canmv image
# make CONF=k230d_canmv_ilp32_defconfig  #build k230d canmv 32bit rootfs;
# make help # view help
```

>[BR2_PRIMARY_SIT configuration primary download site]("https://bootlin.com/pub/conferences/2011/elce/using-buildroot-real-project/using-buildroot-real-project.pdf") for example: `make CONF=k230d_canmv_defconfig  BR2_PRIMARY_SITE=https://ai.b-bug.org/~/wangjianxin/dl/`

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

>linux source code can be specified using the LINUX_OVERRIDE_SRCDIR macro,for example:
>echo LINUX_OVERRIDE_SRCDIR=/home/wangjianxin/t/linux-xuantie-kernel >output/k230d_canmv_defconfig/local.mk

## buildroot

```bash
make menuconfig #buildroot(sdk) modify configuration
make savedefconfig #buildroot(sdk) savedefconfig
```

## ai_demo

[ai demo usage guide](https://developer.canaan-creative.com/k230_linux/dev/zh/01_software/K230_AI_Demo%E4%BD%BF%E7%94%A8%E6%8C%87%E5%8D%97.html)

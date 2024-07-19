# K230 Linux SDK

>This document uses the k230d_canmv_defconfig configuration as an example. If you use other configurations, replace k230d_canmv_defconfig with correct names. All the configurations supported by this sdk,  can be found in  buildroot-overlay/configs directory.

## Install toolchain and dependencies

download  Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.1-20240712.tar.gz toolchan from  https://www.xrvm.cn/community/download?id=4333581795569242112
uncompress the toolchain to the /opt/toolchain(Refer command):

```bash
mkdir -p /opt/toolchain;
tar -zxvf Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.1-20240712.tar.gz -C /opt/toolchain;
```

install dependencies(Refer command):

```bash
apt-get install -y   git sed make binutils build-essential diffutils gcc  g++ bash patch gzip \
        bzip2 perl  tar cpio unzip rsync file  bc findutils wget  libncurses-dev python3  \
        libssl-dev gawk cmake
```

## build

```bash

make CONF=k230d_canmv_defconfig #build k230d canmv image (kernel and rootfs both 64bit);
# make CONF=k230_canmv_defconfig # build k230 canmv image
# make CONF=k230d_canmv_32bit_rootfs_defconfig  #build k230d canmv 32bit rootfs;
# make CONF=k230d_canmv_64kernel_32rootfs_defconfig #build k230d 64bit kernel and 32bit rootfs image
# make help # view help
```

>[BR2_PRIMARY_SIT configuration primary download site]("https://bootlin.com/pub/conferences/2011/elce/using-buildroot-real-project/using-buildroot-real-project.pdf") for example:make CONF=k230d_canmv_defconfig  BR2_PRIMARY_SITE=https://ai.b-bug.org/~/wangjianxin/dl/

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

make linux-rebuild opensbi-rebuild #rebuild linux,rebuild opensbi
make linux-dirclean #linux clean
```

>linux source code can be specified using the LINUX_OVERRIDE_SRCDIR macro,for example:
>echo LINUX_OVERRIDE_SRCDIR=/home/wangjianxin/t/linux-xuantie-kernel >output/k230d_canmv_defconfig/local.mk

## buildroot

```bash
make menuconfig #buildroot(sdk) modify configuration
make savedefconfig #buildroot(sdk) savedefconfig
```

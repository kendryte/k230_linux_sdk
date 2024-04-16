# K230 Liunx SDK

## build

```bash
make CONF=k230_canmv_defconfig  BR2_PRIMARY_SITE=https://ai.b-bug.org/~/wangjianxin/dl/   #compile sdk ,BR2_PRIMARY_SIT is option
make help # view help
```

>[BR2_PRIMARY_SIT configuration primary download site]("https://bootlin.com/pub/conferences/2011/elce/using-buildroot-real-project/using-buildroot-real-project.pdf")

## output

```bash
output/k230_canmv_defconfig/images/sysimage-sdcard.img.gz
```

>uncompress file , [burn to tf card]("https://gitee.com/kendryte/k230_docs/blob/main/zh/01_software/board/K230_SDK_%E4%BD%BF%E7%94%A8%E8%AF%B4%E6%98%8E.md#51-sd%E5%8D%A1%E9%95%9C%E5%83%8F%E7%83%A7%E5%BD%95"),insert  tf to device, poweron device.

## uboot

```bash
make uboot-rebuild #rebuild uboot
make uboot-dirclean #uboot clean
#uboot directory description
buildroot-overlay/boot/uboot/u-boot-2022.10-overlay/  #uboot overlay code 
output/k230_canmv_defconfig/build/uboot-2022.10/ #uboot full code,uboot build dir
```

## linux

```bash
make linux-menuconfig #modify configuration
make linux-savedefconfig #save to defconfig

make linux-rebuild opensbi-rebuild #rebuild linux,rebuild opensbi
make linux-dirclean #linux clean
#linux directory description
buildroot-overlay/linux/linux-6.6.22-overlay #linux overlay code 
output/k230_canmv_defconfig/build/linux-6.6.22 #linux full code,linux build dir
```

## buildroot

```bash
make menuconfig #buildroot(sdk) modify configuration
make savedefconfig #buildroot(sdk) savedefconfig
```

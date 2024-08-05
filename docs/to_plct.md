## k230d新32位系统切换说明

### 编译

```bash
make CONF=k230d_canmv_ilp32_defconfig
#output  output/k230d_canmv_ilp32_defconfig/images/sysimage-sdcard.img.gz
```

### 启动nuttx

```bash
#uboot 命令行执行下面命令
run bnuttx
```

### 启动64位内核+32位rootfs

```bash
run blinux
```

### 启动32位新内核+32位rootfs

```bash
run blinuxilp32
```

### 启动nuttx和linux双系统

```bash
run bnuttx;run blinux
```

## jtag更新k230 内核和设备树方法

1.参考下面命令把内核 设备树 opensbi文件复制到tools目录下面；

```bash
cp  output/k230d_canmv_64kernel_32rootfs_defconfig/build/linux-b6516a27c813582a837d73330f939beba3e019c7/arch/riscv/boot/Image  .
cp  output/k230d_canmv_64kernel_32rootfs_defconfig/build/linux-b6516a27c813582a837d73330f939beba3e019c7/arch/riscv/boot/dts/canaan/k230d-canmv.dtb  .
cp  output/k230d_canmv_64kernel_32rootfs_defconfig/build/opensbi-1.4/build/platform/generic/firmware/fw_dynamic.bin .
```

2.在tools目录下面,参考下面命令替换linux和dtb

```bash
riscv64-unknown-elf-gdb  -ex "target remote 10.100.228.83:1026" -ex "reset" -ex "reset"  -x  gdbinit
```

>10.100.228.83 请替换为正确的ip
>
>gdbinit 文件内容详见tools/gdbinit,主要动作是把内核 设备树 opensbi 通过jtag加载到内存，然后跳到opensbi执行



uboot下网络引导内核和设备树方法：
电脑端执行下面命令：
cp output/k230_canmv_defconfig/build/linux-custom/arch/riscv/boot/dts/canaan/k230-canmv.dtb  ~/tftp
cp output/k230_canmv_defconfig/build/opensbi-1.4/build/platform/generic/firmware/fw_jump.bin  ~/tftp
cp output/k230_canmv_defconfig/build/linux-custom/arch/riscv/boot/Image  ~/tftp
cd ~/tftp ；mkimage -A riscv -O linux -T kernel -C none -a 0 -e 0 -n linux -d fw_jump.bin  fw_jump_add_uboot_head.bin

uboot命令行执行下面命令
usb start;dhcp;
tftpboot 0x3000000 10.10.1.94:wjx/fw_jump_add_uboot_head.bin; tftpboot  0x200000 10.10.1.94:wjx/Image;tftpboot  0x2200000 10.10.1.94:wjx/k230-canmv.dtb;
bootm 0x3000000  - 0x2200000;

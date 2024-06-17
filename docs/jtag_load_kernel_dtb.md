# jtag更新k230 内核和设备树方法

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

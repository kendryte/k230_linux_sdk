# K230 Linux SDK

[English](README.md) | **中文**

- 文档：<https://www.kendryte.com/zh/sdkResource/230linux>
- CI 镜像：<https://download.kendryte.com/k230/release/linux_sdk_images/daily_build/>

> 本文档以 `k230d_canmv_defconfig` 配置为例。如果你使用的是其他配置，请将 `k230d_canmv_defconfig` 替换为对应的名称。所有支持的配置可在 `buildroot-overlay/configs/` 目录下找到。

## 安装工具链和依赖

```bash
git clone --depth=1 https://github.com/kendryte/k230_linux_sdk.git  # 克隆源码
cd k230_linux_sdk # 进入源码目录
sudo make toolchain_and_depend
# 安装 GCC 工具链和 SDK 依赖，详见 tools/install_toolchain_and_depend.sh
```

> - 该 SDK 已在 Ubuntu 22.04 和 24.04 上验证通过，其他版本可能会遇到编译问题。
> - 只有 `k230d_canmv_ilp32_defconfig` 需要手动安装 rv64ilp32 工具链：
>
>   ```bash
>   # 仅 k230d_canmv_ilp32_defconfig 需要
>   wget -c https://github.com/ruyisdk/riscv-gnu-toolchain-rv64ilp32/releases/download/2024.06.25/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25-nightly.tar.gz
>   mkdir -p /opt/toolchain/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25/
>   tar -xvf riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25-nightly.tar.gz \
>       -C /opt/toolchain/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25/
>   ```

## 编译

编译分为三个步骤，依次执行：

1. `make <defconfig>`：选择编译配置。只需执行一次，配置文件位于 `buildroot-overlay/configs/` 目录
2. `make source`：下载配置所需的开源软件包（可选，如果已手动准备好源码包可跳过）
3. `make`：执行编译

命令示例：

```bash
make k230d_canmv_defconfig && make source && make         # 编译 k230d CanMV 镜像（64 位内核和根文件系统）
# make k230_canmv_defconfig && make source  && make       # 编译 k230 CanMV 镜像
# make k230d_canmv_ilp32_defconfig && make source && make # 编译 k230d CanMV 镜像（32 位根文件系统）
# make help                                               # 显示帮助
```

> 首次编译耗时较长。如果想跳过编译直接体验 Linux 系统，可下载 [Canaan 提供的每日构建镜像](https://download.kendryte.com/k230/release/linux_sdk_images/daily_build/)。

## 输出

```bash
output/k230d_canmv_defconfig/images/sysimage-sdcard.img.gz
```

> `k230d_canmv_defconfig` 仅为示例，请替换为你实际使用的配置名称。
> 解压镜像后，[烧录到 TF 卡](https://www.kendryte.com/k230_linux/zh/main/userguide/how_to_flash.html)，将卡插入设备并上电。

## U-Boot

```bash
make uboot-rebuild    # 重新编译 U-Boot
make uboot-dirclean   # 清理 U-Boot 编译产物
```

目录结构：

| 目录                                                    | 说明                          |
| ------------------------------------------------------ | ----------------------------- |
| `buildroot-overlay/boot/uboot/u-boot-2022.10-overlay/` | U-Boot overlay 源码           |
| `output/k230d_canmv_defconfig/build/uboot-2022.10/`    | U-Boot 完整源码及编译目录     |

## Linux 内核

```bash
make linux-menuconfig     # 修改内核配置
make linux-savedefconfig  # 保存内核配置到 defconfig
make linux-rebuild        # 重新编译内核
make linux-dirclean       # 清理内核编译产物
```

## Buildroot（SDK 配置）

```bash
make menuconfig      # 修改 SDK 配置
make savedefconfig   # 保存 SDK 配置
```

## Debian / Ubuntu

```bash
sudo make debian  # 生成 Debian 发行版镜像
sudo make ubuntu  # 生成 Ubuntu 发行版镜像
```

> 首次编译耗时较长。如果想跳过编译，可下载 [Canaan 提供的每日构建镜像](https://download.kendryte.com/k230/release/linux_sdk_images/daily_build/)。
>
> 关于 Debian/Ubuntu 的更多信息，参见 [K230 Debian/Ubuntu 系统指南](https://www.kendryte.com/k230_linux/zh/main/app_develop_guide/user_develop/debian_ubuntu.html)。

## AI Demo

- **源码目录**：`buildroot-overlay/package/ai_demo/`

  ```bash
  ls buildroot-overlay/package/ai_demo/                # 列出所有 AI demo
  ls buildroot-overlay/package/ai_demo/face_detection/  # 查看 face_detection 源码
  ```

- **编译单个 demo**（以 `face_detection` 为例）：

  ```bash
  make face_detection-dirclean  # 清理编译产物
  make face_detection           # 编译
  ```

- **启用更多 demo**：默认只启用 `face_detection`，如需启用其他 demo，使用配置菜单：

  ```bash
  make menuconfig
  # Target packages → Canaan package → AI →
  #   [*] face detection
  #   [ ] demo mix
  #   [ ] bytetrack
  #   ...
  #   [ ] face_mesh
  #   [ ] face_parse
  #   [ ] face_pose
  #   [ ] face_verification
  ```

- **运行 demo**：在设备上进入 `/root/app/` 下对应的目录：

  ```bash
  cd /root/app/face_detection/
  ls
  # face_detect_image.sh       face_detection_320.kmodel
  # face_detect_isp.sh         face_detection_640.kmodel
  # face_detection.elf
  ./face_detect_isp.sh
  ```

- 更多信息参见 [K230 AI Demo 指南](https://www.kendryte.com/k230_linux/zh/main/app_develop_guide/ai_develop/aidemo_doc.html)。

## 常见问题

常见问题请参见 [K230 FAQ](https://www.kendryte.com/k230_linux/zh/main/faq.html)。

## 相关资源

- [K230 Linux SDK 文档](https://www.kendryte.com/zh/sdkResource/230linux)
- [K230 社区论坛](https://www.kendryte.com/answer/)
- [K230 产品中心](https://www.kendryte.com/zh/products)
- [K230 文档中心](https://www.kendryte.com/zh/document)
- [K230 Debian/Ubuntu 系统指南](https://www.kendryte.com/k230_linux/zh/main/app_develop_guide/user_develop/debian_ubuntu.html)

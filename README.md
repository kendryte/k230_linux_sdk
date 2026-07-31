# K230 Linux SDK

- Documentation: <https://www.kendryte.com/zh/sdkResource/230linux>
- CI Images: <https://download.kendryte.com/k230/release/linux_sdk_images/daily_build/>

> This document uses the `k230d_canmv_defconfig` configuration as an example. If you use a different configuration, replace `k230d_canmv_defconfig` with the appropriate name. All supported configurations can be found in the `buildroot-overlay/configs/` directory.

## Install Toolchain and Dependencies

```bash
sudo make toolchain_and_depend
# Installs the GCC toolchain and SDK dependencies. See tools/install_toolchain_and_depend.sh for details.
```

> - This SDK has been verified on Ubuntu 22.04 and 24.04. Other versions may encounter build issues.
> - Only `k230d_canmv_ilp32_defconfig` requires the rv64ilp32 toolchain to be installed manually:
>
>   ```bash
>   # Only needed for k230d_canmv_ilp32_defconfig
>   wget -c https://github.com/ruyisdk/riscv-gnu-toolchain-rv64ilp32/releases/download/2024.06.25/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25-nightly.tar.gz
>   mkdir -p /opt/toolchain/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25/
>   tar -xvf riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25-nightly.tar.gz \
>       -C /opt/toolchain/riscv64ilp32-elf-ubuntu-22.04-gcc-nightly-2024.06.25/
>   ```

## Build

```bash
make k230d_canmv_defconfig && make           # Build k230d CanMV image (64-bit kernel and rootfs)
# make k230_canmv_defconfig  && make       # Build k230 CanMV image
# make k230d_canmv_ilp32_defconfig && make  # Build k230d CanMV image (32-bit rootfs)
# make help                              # Show help
```

> The initial build takes considerable time. To skip compilation and try the Linux system directly, download the [daily build image from Canaan](https://download.kendryte.com/k230/release/linux_sdk_images/daily_build/).

## Output

```bash
output/k230d_canmv_defconfig/images/sysimage-sdcard.img.gz
```

> `k230d_canmv_defconfig` is an example — replace it with your actual configuration name.
> Uncompress the image, [burn it to a TF card](https://www.kendryte.com/k230_linux/zh/main/userguide/how_to_flash.html), insert the card into the device, and power on.

## U-Boot

```bash
make uboot-rebuild    # Rebuild U-Boot
make uboot-dirclean   # Clean U-Boot build artifacts
```

Directory layout:

| Directory                                              | Description                            |
| ------------------------------------------------------ | -------------------------------------- |
| `buildroot-overlay/boot/uboot/u-boot-2022.10-overlay/` | U-Boot overlay source                  |
| `output/k230d_canmv_defconfig/build/uboot-2022.10/`    | U-Boot full source and build directory |

## Linux Kernel

```bash
make linux-menuconfig     # Modify kernel configuration
make linux-savedefconfig  # Save kernel configuration to defconfig
make linux-rebuild        # Rebuild kernel
make linux-dirclean       # Clean kernel build artifacts
```

## Buildroot (SDK Configuration)

```bash
make menuconfig      # Modify SDK configuration
make savedefconfig   # Save SDK configuration
```

## Debian / Ubuntu

```bash
sudo make debian  # Generate Debian distribution image
sudo make ubuntu  # Generate Ubuntu distribution image
```

> The initial build takes considerable time. To skip compilation, download the [daily build image from Canaan](https://download.kendryte.com/k230/release/linux_sdk_images/daily_build/).
>
> For more information about Debian/Ubuntu, see [K230 Debian/Ubuntu System Guide](https://www.kendryte.com/k230_linux/zh/main/app_develop_guide/user_develop/debian_ubuntu.html).

## AI Demo

- **Source directory**: `buildroot-overlay/package/ai_demo/`

  ```bash
  ls buildroot-overlay/package/ai_demo/                # List all AI demos
  ls buildroot-overlay/package/ai_demo/face_detection/  # View face_detection source
  ```

- **Build a single demo** (using `face_detection` as an example):

  ```bash
  make face_detection-dirclean  # Clean build artifacts
  make face_detection           # Build
  ```

- **Enable additional demos**: By default, only `face_detection` is enabled. To enable others, use the menu:

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

- **Run a demo**: Navigate to the corresponding directory under `/root/app/` on the device:

  ```bash
  cd /root/app/face_detection/
  ls
  # face_detect_image.sh       face_detection_320.kmodel
  # face_detect_isp.sh         face_detection_640.kmodel
  # face_detection.elf
  ./face_detect_isp.sh
  ```

- For more information, see the [K230 AI Demo Guide](https://www.kendryte.com/k230_linux/zh/main/app_develop_guide/ai_develop/aidemo_doc.html).

## FAQ

For frequently asked questions, see [K230 FAQ](https://www.kendryte.com/k230_linux/zh/main/faq.html).

## Resources

- [K230 Linux SDK Documentation](https://www.kendryte.com/zh/sdkResource/230linux)
- [K230 Community Forum](https://www.kendryte.com/answer/)
- [K230 Product Center](https://www.kendryte.com/zh/products)
- [K230 Documentation Hub](https://www.kendryte.com/zh/document)
- [K230 Debian/Ubuntu System Guide](https://www.kendryte.com/k230_linux/zh/main/app_develop_guide/user_develop/debian_ubuntu.html)

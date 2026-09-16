#!/bin/bash
# 根据 buildroot .config 中启用的镜像类型，打印实际生成的镜像文件路径
BRW_BUILD_DIR="$1"
config_file="${BRW_BUILD_DIR}/.config"
images_dir="${BRW_BUILD_DIR}/images"

declare -A image_map=(
	[BR2_CANAAN_GEN_MMC_IMG]="sysimage-sdcard.img.gz"
	[BR2_CANAAN_GEN_SPI_NAND_IMG]="sysimage-nand.img.gz"
	[BR2_CANAAN_GEN_SPI_NOR_IMG]="sysimage-spinor32m.img.gz"
)

for config_option in "${!image_map[@]}"; do
	image_name="${image_map[${config_option}]}"
	if grep -q "^${config_option}=y$" "${config_file}" && [ -f "${images_dir}/${image_name}" ]; then
		echo -e "\033[32m Image: ${images_dir}/${image_name}\033[0m"
	fi
done

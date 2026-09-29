#!/bin/bash
# 打印当前使用的配置详情及支持的所有 defconfig 列表
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
CONFIGS_DIR="${SCRIPT_DIR}/../buildroot-overlay/configs"
BRW_BUILD_DIR="$1"
config_file="${BRW_BUILD_DIR}/.config"

declare -A board_note=(
	[k230_canmv_defconfig]="canmv 1.0/1.1 board,嘉楠"
	[k230_canmv_v3_defconfig]="canmv v3 board"
	[k230_canmv_01studio_defconfig]="01studio board,01科技"
	[k230_canmv_dongshanpi_defconfig]="dongshanpi board,百问网"
	[k230_canmv_lckfb_defconfig]="lushanpi ,jialichuang board,嘉立创"
	[BPI-CanMV-K230D-Zero_defconfig]="bananapi k230d"
	[k230d_canmv_ilp32_defconfig]="k230d canmv new32 board,plct use"
	[k230d_canmv_defconfig]="k230d canmv zero board"
	[BPI-CanMV-K230D-Zero_ilp32_defconfig]="plct use,new 32 board"
	[k230_evb_defconfig]="k230 evb board,嘉楠"
	[k230_canmv_gt6700_defconfig]="gt6700 board,银杏科技"
	[k230_canmv_01studio_emmc_defconfig]="01studio emmc board"
	[k230d_canmv_junroc_ai_cam_defconfig]="junroc ai cam,隽鹏"
    [k230_canmv_mrt_defconfig]="韩端"
)

get_val() {
	grep "^$1=" "${config_file}" | cut -d'"' -f2
}

get_src_dir() {
	local dir="${BRW_BUILD_DIR}/build/$1-$2"
	dir="${dir#${REPO_ROOT}/}"
	[ -d "${REPO_ROOT}/${dir}" ] || dir="${dir} (not extracted yet)"
	echo "${dir}"
}

get_src_dir_glob() {
	local dir="$(compgen -G "${BRW_BUILD_DIR}/build/$1-*" | head -1)"
	if [ -z "${dir}" ]; then
		echo "${BRW_BUILD_DIR}/build/$1-* (not extracted yet)" | sed "s#${REPO_ROOT}/##"
	else
		echo "${dir#${REPO_ROOT}/}"
	fi
}

current_conf="$(cut -d = -f2 .last_conf)"
echo "current config:"
echo "	${current_conf} --- buildroot-overlay/configs/${current_conf}"

if [ -f "${config_file}" ]; then
	uboot_board="$(get_val BR2_TARGET_UBOOT_BOARD_DEFCONFIG)"
	[ -n "${uboot_board}" ] || uboot_board="$(get_val BR2_TARGET_UBOOT_BOARDNAME)"
	uboot_srcdir="$(get_src_dir uboot "$(get_val BR2_TARGET_UBOOT_VERSION)")"

	uboot_makeopts="$(get_val BR2_TARGET_UBOOT_CUSTOM_MAKEOPTS)"
	if [[ "${uboot_makeopts}" == *DEVICE_TREE=* ]]; then
		uboot_dts="${uboot_makeopts#*DEVICE_TREE=}"
		uboot_dts="${uboot_dts%% *}"
	else
		uboot_board_defconfig="${REPO_ROOT}/${uboot_srcdir%% *}/configs/${uboot_board}_defconfig"
		if [ -f "${uboot_board_defconfig}" ]; then
			uboot_dts="$(grep '^CONFIG_DEFAULT_DEVICE_TREE=' "${uboot_board_defconfig}" | cut -d'"' -f2)"
		else
			uboot_dts="${uboot_board} (uboot 源码未展开，暂未确认实际 dts)"
		fi
	fi

	linux_dts=""
	for d in $(get_val BR2_LINUX_KERNEL_INTREE_DTS_NAME); do
		linux_dts="${linux_dts} arch/riscv/boot/dts/${d}.dts"
	done
	linux_dts="${linux_dts# }"

	echo "current config detail:"
	echo "	[uboot]"
	echo "	  srcdir  : ${uboot_srcdir}"
	echo "	  config  : configs/${uboot_board}_defconfig"
	echo "	  dts     : arch/riscv/dts/${uboot_dts}.dts"
	echo ""
	echo "	[linux]"
	echo "	  srcdir  : $(get_src_dir linux "$(get_val BR2_LINUX_KERNEL_VERSION)")"
	echo "	  config  : arch/riscv/configs/$(get_val BR2_LINUX_KERNEL_DEFCONFIG)_defconfig"
	echo "	  fragment: $(get_val BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES)"
	echo "	  dts     : ${linux_dts}"
	echo ""
	echo "	[busybox]"
	echo "	  srcdir  : $(get_src_dir_glob busybox)"
	echo "	  config  : $(get_val BR2_PACKAGE_BUSYBOX_CONFIG)"
	echo "	  fragment: $(get_val BR2_PACKAGE_BUSYBOX_CONFIG_FRAGMENT_FILES)"
	echo ""
else
	echo "	(run 'make' once to generate ${config_file} for uboot/linux config and dts detail)"
fi

echo "Available all configs and board note:"
for f in "${CONFIGS_DIR}"/*_defconfig; do
	name="$(basename "${f}")"
	note="${board_note[${name}]:-${name}}"
	printf "	%-38s --%s\n" "${name}" "${note}"
done
echo ""

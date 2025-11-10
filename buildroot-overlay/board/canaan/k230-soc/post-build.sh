#!/bin/bash

#BINARIES_DIR=/home/wangjianxin/k230_linux_sdk/output/k230_canmv_defconfig/images
#BUILDROOT_DIR=${PWD}
set -e
rootfs_dir="$1"


UBOOT_BUILD_DIR=${BUILD_DIR}/uboot-2022.10
K230_SDK_ROOT=$(dirname $(dirname ${BASE_DIR}))
GENIMAGE_CFG_SD=$(dirname $(realpath "$0"))/genimage.cfg

BR2_ROOTFS_OVERLAY=$(cat ${BASE_DIR}/.config | grep BR2_ROOTFS_OVERLAY | cut -d= -f2  |  tr -d '"' )

BR_OVERLAY_DIR=${K230_SDK_ROOT}/buildroot-overlay

#生成版本号
gen_version()
{
	local ver_file="etc/version/release_version"
	local post_copy_rootfs_dir="${BR_OVERLAY_DIR}/${BR2_ROOTFS_OVERLAY}"
	local sdk_ver="unkonwn"
	# local nncase_ver="0.0.0";
	# local nncase_ver_file="${K230_SDK_ROOT}/src/big/nncase/riscv64/nncase/include/nncase/version.h"

	# cat ${nncase_ver_file} | grep NNCASE_VERSION -w | cut -d\" -f 2 > /dev/null && \
	# 	 nncase_ver=$(cat ${nncase_ver_file} | grep NNCASE_VERSION -w | cut -d\" -f 2)

	set +e; commitid=$(awk -F- '/^sdk:/ { print $6}' ${post_copy_rootfs_dir}/${ver_file});set -e;
	set +e; last_tag=$(awk -F- '/^sdk:/ { print $1}' ${post_copy_rootfs_dir}/${ver_file}  | cut -d: -f2  ) ;set -e;



	[ "${commitid}" != "" ] || commitid="unkonwn"
	[ "${last_tag}" != "" ] || last_tag="unkonwn"

	git rev-parse --short HEAD  &&  commitid=$(git rev-parse --short HEAD)
	git describe --tags `git rev-list --tags --max-count=1` && last_tag=$(git describe --tags `git rev-list --tags --max-count=1`)
	git describe --tags --exact-match  && last_tag=$(git describe --tags --exact-match)

	cd  "${TARGET_DIR}" ;
	mkdir -p etc/version/
	sdk_ver="${last_tag}-$(date "+%Y%m%d-%H%M%S")-$(whoami)-$(hostname)-${commitid}"
	echo -e "#############SDK VERSION######################################" >${rootfs_dir}/${ver_file}
	echo -e "sdk:${sdk_ver}" >> ${rootfs_dir}/${ver_file}
	#echo -e "nncase:${nncase_ver}" >> ${ver_file}
	echo -e "##############################################################" >>${rootfs_dir}/${ver_file}
	echo "build version: ${sdk_ver}"

	mkdir -p ${post_copy_rootfs_dir}/etc/version/
	cp -f ${rootfs_dir}/${ver_file}   ${post_copy_rootfs_dir}/${ver_file}

	cd -;
}
auto_boot_proc()
{
	set -x
	#BR2_ROOTFS_OVERLAY=$(cat ${BASE_DIR}/.config | grep BR2_ROOTFS_OVERLAY | cut -d= -f2  |  tr -d '"' )
	local config="${BASE_DIR}/.config"
	local CONF=$(basename ${BASE_DIR})
	local auto_boot_f="${rootfs_dir}/etc/init.d/S100${CONF}"


	cat >${auto_boot_f} <<EOF
[ "\$1" = "stop" ] && exit 0

EOF
	BR2_CANAAN_AUTO_RUN_CMD=$(cat ${config} | grep BR2_CANAAN_AUTO_RUN_CMD | cut -d= -f2  |  tr -d '"' )
	if [ ! -z "$BR2_CANAAN_AUTO_RUN_CMD" ] ; then
		echo  "$BR2_CANAAN_AUTO_RUN_CMD" >> ${auto_boot_f}
	fi


	if  $(cat ${config} |  grep  BR2_PACKAGE_RTL8189FS=y >/dev/null 2>&1 ); then
		echo " modprobe 8189fs " >> ${auto_boot_f}
	fi

	if  $(cat ${config} |  grep  BR2_PACKAGE_BCMDHD=y >/dev/null 2>&1 ); then
		echo " modprobe bcmdhd " >> ${auto_boot_f}
	fi


	if  $(cat ${config} |  grep  BR2_PACKAGE_AIC8800=y >/dev/null 2>&1 ); then
		echo " modprobe aic_load_fw " >> ${auto_boot_f}
		echo " modprobe aic8800_fdrv " >> ${auto_boot_f}
		echo " modprobe aic_btusb " >> ${auto_boot_f}
	fi
	chmod a+x ${auto_boot_f}
}
gen_version
auto_boot_proc
[ -f "${STAGING_DIR}/lib/libasan.so.8" ] && cp "${STAGING_DIR}/lib/libasan.so.8" "${TARGET_DIR}/lib/"
cd ${TARGET_DIR};rm -rf app ; ln -s root/app app; cd -;

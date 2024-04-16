#!/bin/bash

#BINARIES_DIR=/home/wangjianxin/k230_linux_sdk/output/k230_canmv_defconfig/images
#BUILDROOT_DIR=${PWD}
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
	echo -e "#############SDK VERSION######################################" >${ver_file}
	echo -e "sdk:${sdk_ver}" >> ${ver_file}
	#echo -e "nncase:${nncase_ver}" >> ${ver_file}
	echo -e "##############################################################" >>${ver_file}
	echo "build version: ${sdk_ver}"

	mkdir -p ${post_copy_rootfs_dir}/etc/version/
	cp -f ${ver_file}  ${post_copy_rootfs_dir}/${ver_file}

	cd -;
}

gen_version
#!/bin/bash
set -e
DTB="k230-canmv.dtb"
LINUX_DIR=${BUILD_DIR}/linux-6.6.22
rootfs_ext4_file=""

[ $# -ge 2 ] && DTB="$2.dtb"
[ $# -ge 3 ] && LINUX_DIR="$3"
[ $# -ge 4 ] && rootfs_ext4_file="$4"
#echo "${DTB}, ${rootfs_ext4_file}"

#BINARIES_DIR=/home/wangjianxin/k230_linux_sdk/output/k230_canmv_defconfig/images
UBOOT_BUILD_DIR=${BUILD_DIR}/uboot-2022.10
K230_SDK_ROOT=$(dirname $(dirname ${BASE_DIR}))
GENIMAGE_CFG_SD=$(dirname $(realpath "$0"))/genimage.cfg
env_dir=$(dirname $(realpath "$0"))



#放到post build
gz_file_add_ver()
{
	[ $# -lt 1 ] && return
	local f="$1"
	local CONF=$(basename ${BASE_DIR})

	local sdk_ver="v0.0.0";
	local nncase_ver="0.0.0";

	local sdk_ver_file="${K230_SDK_ROOT}/buildroot-overlay/board/canaan/k230-soc/rootfs_overlay/etc/version/release_version"
	#local nncase_ver_file="${K230_SDK_ROOT}/src/big/nncase/riscv64/nncase/include/nncase/version.h"

	local storage="$(echo "$f" | sed -nE "s#[^-]*-([^\.]*).*#\1#p")"
	local conf_name="${CONF%%_defconfig}"


	sdk_ver=$(awk -F- '/^sdk:/ { print $1}' ${sdk_ver_file}  | cut -d: -f2 )

	# cat ${nncase_ver_file} | grep NNCASE_VERSION -w | cut -d\" -f 2 > /dev/null && \
	# 	 nncase_ver=$(cat ${nncase_ver_file} | grep NNCASE_VERSION -w | cut -d\" -f 2)
	rm -rf  ${conf_name}_${storage}_${sdk_ver}_nncase_v${nncase_ver}.img.gz;
	ln -s  $f ${conf_name}_${storage}_${sdk_ver}_nncase_v${nncase_ver}.img.gz;
}


#add_firmHead  xxx.bin  "-n"
#output fn_$1 fa_$1 fs_$1
add_firmHead()
{

	# $(UBOOT_BUILD_DIR)/tools/firmware_gen.py
	local filename="$1"
	#export PATH=\"${PATH#*host\/bin:}
	local firmware_gen="python3  ${UBOOT_BUILD_DIR}/tools/firmware_gen.py "
	PATH_BAK=${PATH};	PATH="${PATH#*host\/bin:}" ;


	if [ $# -ge 2 ]; then
		firmArgs="$2" #add k230 firmware head
		cp ${filename} ${filename}.t; ${firmware_gen}   -i ${filename}.t -o f${firmArgs##-}${filename} ${firmArgs};

	else
		#add k230 firmware head
		firmArgs="-n"; cp ${filename} ${filename}.t; ${firmware_gen}   -i ${filename}.t -o f${firmArgs##-}_${filename} ${firmArgs};
		if [ "${CONFIG_GEN_SECURITY_IMG}" = "y" ];then
			firmArgs="-s";cp ${filename} ${filename}.t;	 ${firmware_gen}   -i ${filename}.t -o f${firmArgs##-}_${filename} ${firmArgs};
			firmArgs="-a";cp ${filename} ${filename}.t;	${firmware_gen}   -i ${filename}.t -o f${firmArgs##-}_${filename} ${firmArgs};
		fi
	fi
	rm -rf  ${filename}.t
	PATH=${PATH_BAK};
}

k230_gzip()
{
	local filename="$1"
	local k230_gzip_tool="${K230_SDK_ROOT}/tools/k230_priv_gzip "
	${k230_gzip_tool} -n8  -f -k ${filename}  ||   ${k230_gzip_tool} -n9 -f -k ${filename} ||  \
	${k230_gzip_tool} -n7 -f -k ${filename}   ||   ${k230_gzip_tool} -n6 -f -k ${filename} || \
	${k230_gzip_tool} -n5 -f -k ${filename}   ||   ${k230_gzip_tool} -n4 -f -k ${filename}
	set -e ;sed -i -e "1s/\x08/\x09/"  ${filename}.gz; set +e;
}

# "-O linux -T firmware  -a ${add} -e ${add} -n ${name}"
# "-n/-a/-s"
#file_gzip_ubootHead_firmHead ${quick_boot_cfg_data_file}  "-O linux -T firmware  -a ${add} -e ${add} -n ${name}"   "-n"
#output fn_ug_xx fa_ug_xx fs_ug_xx;
bin_gzip_ubootHead_firmHead()
{

	local mkimage="${UBOOT_BUILD_DIR}/tools/mkimage"
	local file_full_path="$1"
	local filename=$(basename ${file_full_path})
	local mkimgArgs="$2"
	local firmArgs="$3"

	#[ -f ${file_full_path} ] || (echo ${filename} >${file_full_path} )
	# cd  "${BUILD_DIR}/images/";
	[ "$(dirname ${file_full_path})" == "$(pwd)" ] || cp ${file_full_path} .

	k230_gzip ${filename}

	#add uboot head
	${mkimage} -A riscv -C gzip  ${mkimgArgs} -d ${filename}.gz  ug_${filename} # ${filename}.gzu;

	add_firmHead ug_${filename}
	rm -rf ${filename}  ${filename}.gz ug_${filename}
}
#K230_SDK_ROOT  UBOOT_BUILD_DIR
gen_uboot_bin()
{
	# if [ "${CONFIG_SUPPORT_RTSMART}" = "y" ] && [ "${CONFIG_SUPPORT_LINUX}" != "y" ]; then
    #     CONFIG_MEM_LINUX_SYS_BASE="${CONFIG_MEM_RTT_SYS_BASE}"
    # fi;
	#buildroot-overlay/boot/uboot/u-boot-2022.10-overlay/board/canaan/common/sdk_autoconf.h
	local CONFIG_MEM_LINUX_SYS_BASE=$(cat ${UBOOT_BUILD_DIR}/board/canaan/common/sdk_autoconf.h | grep CONFIG_MEM_LINUX_SYS_BASE | awk '{print $3}')
	mkdir -p "${BINARIES_DIR}/uboot"

	cd ${BINARIES_DIR}/uboot
	cp ${UBOOT_BUILD_DIR}/u-boot.bin  .
	cp ${UBOOT_BUILD_DIR}/spl/u-boot-spl.bin  .


	# "-O linux -T firmware  -a ${add} -e ${add} -n ${name}"
	# "-n/-a/-s"  "-n/-a/-s"
	#fn_ug_xxx
	bin_gzip_ubootHead_firmHead  $("pwd")/u-boot.bin   \
					"-O u-boot -T firmware  -a ${CONFIG_MEM_LINUX_SYS_BASE} -e ${CONFIG_MEM_LINUX_SYS_BASE} -n uboot"

	add_firmHead  u-boot-spl.bin #
	${UBOOT_BUILD_DIR}/tools/endian-swap.py   fn_u-boot-spl.bin  swap_fn_u-boot-spl.bin

	#rm -rf u-boot-spl.bin
}


#生成可用uboot引导的linux版本文件
gen_linux_bin ()
{
	local mkimage="${UBOOT_BUILD_DIR}/tools/mkimage"

	local CONFIG_MEM_LINUX_SYS_BASE=$(cat ${UBOOT_BUILD_DIR}/board/canaan/common/sdk_autoconf.h | grep CONFIG_MEM_LINUX_SYS_BASE | awk '{print $3}')

	cd  "${BINARIES_DIR}/";

	# local LINUX_SRC_PATH="src/little/linux"
	# local LINUX_DTS_PATH="src/little/linux/arch/riscv/boot/dts/kendryte/${CONFIG_LINUX_DTB}.dts"

	# return ;
	# cd  "${BUILD_DIR}/images/little-core/" ;
	# cpp -nostdinc -I ${K230_SDK_ROOT}/${LINUX_SRC_PATH}/include -I ${K230_SDK_ROOT}/${LINUX_SRC_PATH}/arch  -undef -x assembler-with-cpp ${K230_SDK_ROOT}/${LINUX_DTS_PATH}  hw/k230.dts.txt


	# ROOTFS_BASE=`cat hw/k230.dts.txt | grep initrd-start | awk -F " " '{print $4}' | awk -F ">" '{print $1}'`
	# ROOTFS_SIZE=`ls -lt rootfs-final.cpio.gz | awk '{print $5}'`
	# ((ROOTFS_END= $ROOTFS_BASE + $ROOTFS_SIZE))
	# ROOTFS_END=`printf "0x%x" $ROOTFS_END`
	# sed -i "s/linux,initrd-end = <0x0 .*/linux,initrd-end = <0x0 $ROOTFS_END>;/g" hw/k230.dts.txt

	# ${LINUX_BUILD_DIR}/scripts/dtc/dtc -I dts -q -O dtb hw/k230.dts.txt  >k230.dtb;
	ln -s ${DTB} k.dtb
	k230_gzip fw_payload.bin;
	echo a>rd;
	${mkimage} -A riscv -O linux -T multi -C gzip -a ${CONFIG_MEM_LINUX_SYS_BASE} -e ${CONFIG_MEM_LINUX_SYS_BASE} -n linux -d fw_payload.bin.gz:rd:k.dtb  ulinux.bin;

	add_firmHead  ulinux.bin
	mv fn_ulinux.bin  linux_system.bin
	[ -f fa_ulinux.bin ] && mv fa_ulinux.bin  linux_system_aes.bin
	[ -f fs_ulinux.bin ] && mv fs_ulinux.bin  linux_system_sm.bin
	rm -rf rd;
}




#生成sd卡镜像文件
#--$1--cfg
#--$2 iamge name
gen_image()
{
	#set -x;

	local genimage="genimage "
	local cfg="$1" ; #"genimage-sdcard.cfg"
	local image_name="$2"; #"sysimage-sdcard.img"
	cd  "${BINARIES_DIR}/";

	[ -z "${rootfs_ext4_file}" ] ||  cp  ${rootfs_ext4_file}  rootfs.ext4;

	GENIMAGE_TMP="genimage.tmp" ;	rm -rf "${GENIMAGE_TMP}";
	${genimage}   	--rootpath "${TARGET_DIR}"  --tmppath "${GENIMAGE_TMP}"    \
					--inputpath "$(pwd)"  	--outputpath "$(pwd)"	--config "${cfg}"

	rm -rf "${GENIMAGE_TMP}"
	gzip -k -f ${image_name}
	chmod a+rw ${image_name} ${image_name}.gz;

	gz_file_add_ver ${image_name}.gz
}

gen_env_bin()
{
	local mkenvimage="${UBOOT_BUILD_DIR}/tools/mkenvimage"
	cd  "${BINARIES_DIR}/";
	local default_env_file=${env_dir}/default.env;
	${mkenvimage} -s 0x10000 -o uboot/env.env  ${default_env_file}
}
gen_boot_ext4()
{
	cd  "${BINARIES_DIR}/";
	mkdir boot;

	cp ${K230_SDK_ROOT}/buildroot-overlay/board/canaan/k230-soc/rootfs_overlay/boot/nuttx-7000000-uart2.bin  boot/;
	cp Image boot/;
	${UBOOT_BUILD_DIR}/tools/mkimage -A riscv -O linux -T kernel -C none -a 0 -e 0 -n linux -d ${BINARIES_DIR}/fw_jump.bin  boot/fw_jump_add_uboot_head.bin
	cp ${DTB} boot;
	cd boot; ln -s ${DTB} k.dtb; cd -;
	rm -rf boot.ext4 ;fakeroot mkfs.ext4 -d boot  -r 1 -N 0 -m 1 -L "boot" -O ^64bit boot.ext4 45M
}

gen_uboot_bin
gen_env_bin
#gen_linux_bin;
gen_boot_ext4

gen_image ${GENIMAGE_CFG_SD}   sysimage-sdcard.img

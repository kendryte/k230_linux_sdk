#!/bin/bash
set -e
BUILDROOT_PATH=$(pwd)
#BINARIES_DIR=/home/wangjianxin/k230_linux_sdk/output/k230_canmv_defconfig/images
UBOOT_BUILD_DIR=${BUILD_DIR}/uboot-2022.10
K230_SDK_ROOT=$(dirname $(dirname ${BASE_DIR}))
GENIMAGE_CFG_SD=$(dirname $(realpath "$0"))/genimage_cfg/genimage.cfg
GENIMAGE_CFG_NAND=$(dirname $(realpath "$0"))/genimage_cfg/genimage_nand.cfg
GENIMAGE_CFG_NOR=$(dirname $(realpath "$0"))/genimage_cfg/genimage_nor.cfg
env_dir=$(dirname $(realpath "$0"))/env
CONF=$(basename ${BASE_DIR})



#放到post build
gz_file_add_ver()
{
	[ $# -lt 1 ] && return
	local f="$1"

	local sdk_ver="v0.0.0";
	local nncase_ver="2.9.0";
	local commit_id="xx";

	local sdk_ver_file="${K230_SDK_ROOT}/buildroot-overlay/board/canaan/k230-soc/rootfs_overlay/etc/version/release_version"
	local nncase_ver_file="${K230_SDK_ROOT}/output/${CONF}/build/libnncase/nncase/include/nncase/version.h"


	local storage="$(echo "$f" | sed -nE "s#[^-]*-([^\.]*).*#\1#p")"

	if [ "${CONF}" = "k230_canmv_defconfig" ] ; then
		canaan_site_name="CanMV-K230_V1P0_P1";
	elif [ "${CONF}" = "k230_evb_defconfig" ] ; then
		canaan_site_name="EVB-K230";
	elif [ "${CONF}" = "k230_canmv_01studio_defconfig" ] ; then
		canaan_site_name="CanMV-K230_01studio";
	elif [ "${CONF}" = "k230_canmv_lckfb_defconfig" ] ; then
		canaan_site_name="CanMV-K230_LCKFB";
	elif [ "${CONF}" = "k230_canmv_v3_defconfig" ] ; then
		canaan_site_name="CanMV-K230_V3P0";
	elif [ "${CONF}" = "k230d_canmv_defconfig" ] ; then
		canaan_site_name="CanMV-K230D";
	elif [ "${CONF}" = "k230d_canmv_junroc_ai_cam_defconfig" ] ; then
		canaan_site_name="CanMV_K230D_JUNROC_AI_CAM";
	else
		canaan_site_name="${CONF%%_defconfig}"	;
	fi



	sdk_ver=$(awk -F- '/^sdk:/ { print $1}' ${sdk_ver_file}  | cut -d: -f2 )
	commit_id=$(awk -F- '/^sdk:/ { print $6}' ${sdk_ver_file})

	if [ -e "${nncase_ver_file}" ]; then
		cat ${nncase_ver_file} | grep NNCASE_VERSION -w | cut -d\" -f 2 > /dev/null && \
			nncase_ver=$(cat ${nncase_ver_file} | grep NNCASE_VERSION -w | cut -d\" -f 2)
	fi
	final_img_name="${canaan_site_name}_linux_${sdk_ver}_nncase_v${nncase_ver}_${commit_id}_${storage}.img.gz"
	rm -rf  ${final_img_name}; ln -s  $f ${final_img_name};
}


#add_firmHead  xxx.bin  "-n"
#output fn_$1 fa_$1 fs_$1
add_firmHead()
{

	# $(UBOOT_BUILD_DIR)/tools/firmware_gen_no_securiy.py
	local filename="$1"
	#export PATH=\"${PATH#*host\/bin:}
	local firmware_gen="python3  ${UBOOT_BUILD_DIR}/tools/firmware_gen_no_securiy.py "

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

}

k230_gzip()
{
	local filename="$1"
	local k230_gzip_tool="${K230_SDK_ROOT}/tools/k230_priv_gzip "
	${k230_gzip_tool} -n8  -f -k ${filename}  ||   ${k230_gzip_tool} -n9 -f -k ${filename} ||  \
	${k230_gzip_tool} -n7 -f -k ${filename}   ||   ${k230_gzip_tool} -n6 -f -k ${filename} || \
	${k230_gzip_tool} -n5 -f -k ${filename}   ||   ${k230_gzip_tool} -n4 -f -k ${filename}
	set -e ;sed -i -e "1s/\x08/\x09/"  ${filename}.gz;
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
	local first_dtb="$(grep BR2_LINUX_KERNEL_INTREE_DTS_NAME ${BR2_CONFIG} | cut -d / -f2 | tr -d '"' |  cut -d ' ' -f1).dtb"
	local CONFIG_MEM_LINUX_SYS_BASE=$(cat ${UBOOT_BUILD_DIR}/board/canaan/common/sdk_autoconf.h | grep CONFIG_MEM_LINUX_SYS_BASE | awk '{print $3}')

	grep -q "^BR2_CANAAN_GEN_SPI_NAND_IMG=y$" ${BR2_CONFIG} || grep -q "^BR2_CANAAN_GEN_SPI_NOR_IMG=y$" ${BR2_CONFIG} || return 0


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
	rm -rf k.dtb; ln -s ${first_dtb} k.dtb
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
	local nand_default_env_file=${env_dir}/default_nand.env;
	local nor_default_env_file=${env_dir}/default_nor.env;

	if [ ${CONF} == "k230d_canmv_ilp32_defconfig" ] || [ ${CONF} == "BPI-CanMV-K230D-Zero_ilp32_defconfig" ]; then
		sed -i 's/^bootcmd=.*$/bootcmd=run bnuttx;run blinuxilp32;/g' ${default_env_file}
	else
		sed -i 's/^bootcmd=.*$/bootcmd=run blinux;/g' ${default_env_file}
	fi
	${mkenvimage} -s 0x2000 -o uboot/env.env  ${default_env_file}
	${mkenvimage} -s 0x2000 -o uboot/nand_env.env  ${nand_default_env_file}
	${mkenvimage} -s 0x2000 -o uboot/nor_env.env  ${nor_default_env_file}
}
gen_boot_ext4_copy_dtb()
{
	cd  "${BINARIES_DIR}/boot";
	rm -rf *.dtb
	cp ../*.dtb .;

	# BR2_LINUX_KERNEL_INTREE_DTS_NAME="canaan/k230-canmv-01studio-lcd  canaan/k230-canmv-01studio"
	local dtb_num="$(grep BR2_LINUX_KERNEL_INTREE_DTS_NAME ${BR2_CONFIG} | tr -d '"' | grep -oP 'canaan/\S+' | wc -l)"

	if [ $dtb_num -ge 2 ];then
		local lcd_dtb="$(grep BR2_LINUX_KERNEL_INTREE_DTS_NAME ${BR2_CONFIG} | tr ' ' '\n' | grep 'canaan/.*lcd' | head -1 | cut -d/ -f2 | tr -d '"').dtb"
		local hdmi_dtb="$(grep BR2_LINUX_KERNEL_INTREE_DTS_NAME ${BR2_CONFIG} | tr ' ' '\n' | grep 'canaan/' | grep -v 'lcd' | head -1 | cut -d/ -f2 | tr -d '"').dtb"
		echo "${lcd_dtb}" > lcd_dtb
		echo "${hdmi_dtb}" > hdmi_dtb
    else
		local first_dtb="$(grep BR2_LINUX_KERNEL_INTREE_DTS_NAME ${BR2_CONFIG} | cut -d / -f2 | tr -d '"' |  cut -d ' ' -f1).dtb"
		echo  "${first_dtb}" >  force_dtb;
	fi

	cd  -;
}
gen_boot_ext4()
{
	local default_env_file=${env_dir}/default.env;
	local logo=$(grep CONFIG_K230_BARE_DISP_LOGO_PATH ${UBOOT_BUILD_DIR}/.config  | cut -d '"' -f2 |  sed 's/\.png$/.yuv/')

	cd  "${BINARIES_DIR}/";
	rm -rf boot; mkdir -p boot;
	gen_boot_ext4_copy_dtb

	if [ ${CONF} == "k230d_canmv_ilp32_defconfig" ] || [ ${CONF} == "BPI-CanMV-K230D-Zero_ilp32_defconfig" ] ; then
		cp ${K230_SDK_ROOT}/buildroot-overlay/board/canaan/k230-soc/rootfs_overlay/boot/nuttx-7000000-uart2.bin  boot/;
		sed -i 's/^bootcmd=.*$/bootcmd=run bnuttx;run blinuxilp32;/g' ${default_env_file}
	fi

	cp Image boot/;
	[ ! -f "Image_ilp32" ] ||  cp Image_ilp32 boot/;

	[ -z "${logo}" ]  ||  cp ${BUILDROOT_PATH}/${logo} boot/logo.yuv;
	#cd boot; rm -rf k.dtb;ln -s ${first_dtb} k.dtb; cd -;
	${UBOOT_BUILD_DIR}/tools/mkimage -A riscv -O linux -T kernel -C none -a 0 -e 0 -n linux -d ${BINARIES_DIR}/fw_jump.bin  boot/fw_jump_add_uboot_head.bin
	rm -rf boot.ext4 ;fakeroot mkfs.ext4 -d boot  -r 1 -N 0 -m 1 -L "boot" -O ^64bit boot.ext4 80M
}
gen_deb_packages_gz()
{
	cd  "${BINARIES_DIR}/deb/";
	rm -rf  Packages.gz;dpkg-scanpackages . /dev/null | gzip -9c > Packages.gz
	cd -;
}
#手动指定要从rootfs里删掉的大文件(相对TARGET_DIR的路径)
LARGE_FILE_LIST=(
	"usr/lib/libavcodec.so.58.134.100"
	"root/app/face_detect/face_detect.elf"
	"usr/lib/libpython3.13.so.1.0"
	"usr/lib/python3.13/site-packages/nncaseruntime/_nncaseruntime_k230.cpython-313-riscv64-linux-gnu.so"
	"root/app/ai2d_kpu/ai2d_kpu.elf"
	"usr/lib/libopencv_dnn.so.4.10.0"
	# "usr/lib/libcrypto.so.3" 不能删除
	"usr/bin/perl"
	"usr/lib/libavfilter.so.7.110.100"
	"usr/lib/python3.13/site-packages/numpy/core/_multiarray_umath.cpython-313-riscv64-linux-gnu.so"
	"usr/lib/libprotobuf.so.29.3.0"
	"usr/lib/libopencv_imgproc.so.4.10.0"
	"usr/lib/python3.13/site-packages/cv2/python-3.13/cv2.cpython-313-riscv64-linux-gnu.so"
	"root/app/ai2d_kpu/test.kmodel"
	"usr/lib/libavformat.so.58.76.100"
	"lib/modules/6.6.36/kernel/fs/btrfs/btrfs.ko"
	"usr/lib/libopencv_core.so.4.10.0"
	"root/app/ai2d_kpu/ai2d_input.bin"
	"usr/lib/python3.13/ensurepip/_bundled/pip-25.0.1-py3-none-any.whl"
	"root/app/camera_webrtc_demo"
	"usr/bin/radix2-big-64k"
	"lib/libasan.so.8"
	"usr/lib/python3.13/site-packages/numpy/linalg/_umath_linalg.cpython-313-riscv64-linux-gnu.so"
	"usr/lib/libsqlite3.so.0.8.6"
	"usr/lib/python3.13/site-packages/numpy/linalg/lapack_lite.cpython-313-riscv64-linux-gnu.so"
	"usr/lib/libliveMedia.so.94.0.1"
	"usr/lib/libwebrtc-audio-processing-1.so.3"
	"lib/modules/6.6.36/kernel/net/bluetooth/bluetooth.ko"

	#--- perl 解释器已经删了(usr/bin/perl),下面这整棵树留着也跑不了,直接整目录删 ---
	"usr/lib/perl5"

	#--- python 打包/构建工具链,目标板运行时不需要 ---
	"usr/lib/python3.13/site-packages/pip"
	"usr/lib/python3.13/site-packages/pip-25.0.dist-info"
	"usr/lib/python3.13/site-packages/setuptools"
	"usr/lib/python3.13/site-packages/setuptools-75.8.0.dist-info"
	"usr/lib/python3.13/site-packages/pkg_resources"
	"usr/lib/python3.13/site-packages/_distutils_hack"
	"usr/lib/python3.13/site-packages/distutils-precedence.pth"

	#--- numpy 自带的单测/过时构建子模块,运行时不需要 ---
	"usr/lib/python3.13/site-packages/numpy/core/tests"
	"usr/lib/python3.13/site-packages/numpy/lib/tests"
	"usr/lib/python3.13/site-packages/numpy/distutils"
)

delet_large_file()
{
	local f
	for f in "${LARGE_FILE_LIST[@]}"; do
		local target="${TARGET_DIR}/${f}"
		if [ -e "${target}" ]; then
			rm -rf "${target}"
		else
			: #echo "delet_large_file: not found, skip: ${target}"
		fi
	done

	#python编译缓存,运行时会自动重新生成,不需要打包进rootfs
	find "${TARGET_DIR}/usr/lib/python3.13" -type d -name '__pycache__' -exec rm -rf {} + 2>/dev/null || true
}
gen_sys_img()
{
	rm -rf ${BINARIES_DIR}/*.img.gz  ${BINARIES_DIR}/sysimage*.img;
	if grep -q "^BR2_CANAAN_GEN_MMC_IMG=y$" ${BR2_CONFIG}; then
		gen_image ${GENIMAGE_CFG_SD}   sysimage-sdcard.img
	fi
	rm -rf ${TARGET_DIR}/../target_*;

	if grep -q "^BR2_CANAAN_GEN_SPI_NAND_IMG=y$" ${BR2_CONFIG}; then
		rm -rf ${BINARIES_DIR}/../target_bak; cp -r  ${TARGET_DIR}/ ${BINARIES_DIR}/../target_bak;
		rm -rf ${TARGET_DIR}/boot/*;
		delet_large_file
		#cp -r  ${BINARIES_DIR}/boot/*  ${TARGET_DIR}/boot/;
		gen_image ${GENIMAGE_CFG_NAND}  sysimage-nand.img
		cp -r  ${TARGET_DIR}/ ${BINARIES_DIR}/../target_nand;
		rm -rf ${TARGET_DIR}; mv ${BINARIES_DIR}/../target_bak  ${TARGET_DIR};

	fi

	# if grep -q "^BR2_CANAAN_GEN_SPI_NOR_IMG=y$" ${BR2_CONFIG}; then
	# 	rm -rf ${BINARIES_DIR}/../target_bak; cp -r  ${TARGET_DIR}/ ${BINARIES_DIR}/../target_bak;
	# 	rm -rf ${TARGET_DIR}/boot/*;
	# 	delet_large_file
	# 	#cp -r  ${BINARIES_DIR}/boot/*  ${TARGET_DIR}/boot/;
	# 	gen_image ${GENIMAGE_CFG_NOR}  sysimage-spinor32m.img
	# 	cp -r  ${TARGET_DIR}/ ${BINARIES_DIR}/../target_nor;
	# 	rm -rf ${TARGET_DIR}; mv ${BINARIES_DIR}/../target_bak  ${TARGET_DIR};
	# fi
}

gen_uboot_bin
gen_env_bin
gen_linux_bin;
gen_boot_ext4
#gen_deb_packages_gz
gen_sys_img
#从nand启动；从nor启动；环境变量配置；
#修改下默认环境变量，bootargs；
#

#!/bin/bash
distribution_type="$1"
BRW_BUILD_DIR="$2"
BINARIES_DIR="${BRW_BUILD_DIR}/images"

set -e;
COLOR_NONE="\033[0m"
RED="\033[1;31;40m"
BLUE="\033[1;34;40m"
GREEN="\033[1;32;40m"
YELLOW="\033[1;33;40m"

print_red()
{
    echo -e ${RED}$*${COLOR_NONE}
}
print_blue()
{
    echo -e ${BLUE}$*${COLOR_NONE}
}

debian_gen_rootfs()
{

cat << 'EOFF' >tmp.txt
sudo rm -rf debian13
sudo apt-get update
sudo apt install qemu-user-static binfmt-support debootstrap debian-ports-archive-keyring systemd-container rsync wget
sudo debootstrap --arch=riscv64  unstable debian13 https://mirrors.aliyun.com/debian/
sudo chroot debian13/
echo "root:root" | chpasswd

cat >>/etc/network/interfaces <<EOF
auto lo
iface lo inet loopback
auto eth0
iface eth0 inet dhcp
'EOF'
apt-get install -y  net-tools ntpdate
ntpdate ntp.ntsc.ac.cn
exit


chroot /path/to/rootfs
apt-get install python3-pyqt5 vim ntp
apt-get install openssh-server
apt-get install libdrm-dev
apt-get install qtbase5-dev qtbase5-examples
apt-get install lxqt
systemctl disable sddm


cat << EOF > /etc/profile.d/qt_env.sh
export QT_QPA_PLATFORM=linuxfb
export QT_QPA_FB_DRM=1
export QT_QPA_EGLFS_KMS_CONFIG="/root/kms_config.json"
EOF




cat << EOF > /root/kms_config.json
{
"device": "/dev/dri/card0",
"outputs": [
    { "name": "HDMI1", "format": "argb8888" }
]
}
EOF

cat << EOF > /root/helloworld_pyqt.py
import sys
from PyQt5.QtWidgets import QApplication, QWidget, QLabel
from PyQt5.QtGui import QFont

def main():
    app = QApplication(sys.argv)  # 创建一个 QApplication 实例
    window = QWidget()  # 创建一个 QWidget 实例作为主窗口
    window.setWindowTitle('Hello World')  # 设置窗口标题
    label = QLabel('Hello World', window)  # 创建一个 QLabel 实例显示文本
    label.move(50, 50)  # 移动标签到窗口中的位置

    label = QLabel('王建新，，测试', window)  # 创建一个 QLabel 实例显示文本
    label.move(300, 400)  # 移动标签到窗口中的位置

    label.setFont(QFont('Arial', 30, QFont.Bold))
    label.setStyleSheet("QLabel { color: red; }")  # 设置字体颜色为红色


    label = QLabel('k230 pyqt5 测试', window)  # 创建一个 QLabel 实例显示文本
    label.move(500, 500)  # 移动标签到窗口中的位置

    label.setFont(QFont('Arial', 50, QFont.Bold))
    label.setStyleSheet("QLabel { color: red; }")  # 设置字体颜色为红色



    window.show()  # 显示窗口
    sys.exit(app.exec_())  # 进入 Qt 事件循环

if __name__ == '__main__':
    main()
EOF

echo "a"> /first_boot_flag
cat << 'EOF' > /etc/profile.d/disk.sh
bootddev=$(cat /proc/cmdline  | sed  -n  "s#root=\(\/dev\/mmcblk[0-9]\).*#\1#p" )
if [ -f /first_boot_flag ]; then
    echo "first boot flag"
    sd_size=$(parted ${bootddev} print | grep ${bootddev} | cut -d: -f2)
    parted ${bootddev} resizepart 2 ${sd_size}; resize2fs ${bootddev}p2
    rm -rf /first_boot_flag
else
    echo "not exit flag"
fi
mount ${bootddev}p1 /boot
EOF
echo "PermitRootLogin yes" >> etc/ssh/sshd_config

tar  -czf debian13.tar.gz debian13
#debian13_size="$(( "$(sudo du -sm debian13 | cut -f1 )" + 300 ))"
#sudo  mkfs.ext4 -d debian13  -r 1 -N 0 -m 1 -L "rootfs" -O ^64bit debian13.ext4 ${debian13_size}m
#tar -czvf debian13.ext4.tar.gz debian13.ext4
#debian13.ext4.tar.gz 是debian的ext4格式根文件系统压缩包

EOFF
    print_red "you need  manually execute the follow commands"
    echo -e ${BLUE}
    cat  tmp.txt
    echo -e  ${COLOR_NONE}
    rm -rf tmp.txt

    print_red "You need to manually execute the above commands one by one on linux(not docker) "
    print_red "referenc doc is <<https://developer.canaan-creative.com/k230/zh/dev/03_other/K230_debian_ubuntu%E8%AF%B4%E6%98%8E.html>>"
}
ubuntu_gen_rootfs()
{
    echo "ubuntu_gen_rootfs"
    cat << 'EOFF' >tmp.txt
#docker run  --privileged  -u root -it  -v $(pwd):$(pwd)    -w $(pwd) ubuntu24:v1  /bin/bash

rm -rf ubuntu24
sudo debootstrap --arch=riscv64   noble ubuntu24 https://mirrors.aliyun.com/ubuntu-ports/
chroot ubuntu24 /bin/bash

cat >/etc/apt/sources.list <<EOF
deb https://mirrors.aliyun.com/ubuntu-ports noble main restricted

deb https://mirrors.aliyun.com/ubuntu-ports noble-updates main restricted

deb https://mirrors.aliyun.com/ubuntu-ports noble universe
deb https://mirrors.aliyun.com/ubuntu-ports noble-updates universe

deb https://mirrors.aliyun.com/ubuntu-ports noble multiverse
deb https://mirrors.aliyun.com/ubuntu-ports noble-updates multiverse

deb https://mirrors.aliyun.com/ubuntu-ports noble-backports main restricted universe multiverse

deb https://mirrors.aliyun.com/ubuntu-ports noble-security main restricted
deb https://mirrors.aliyun.com/ubuntu-ports noble-security universe
deb https://mirrors.aliyun.com/ubuntu-ports noble-security multiverse
EOF
echo "root:root" | chpasswd
echo k230>/etc/hostname
apt-get install ssh  parted
echo "PermitRootLogin yes" >> /etc/ssh/sshd_config
echo "a" >/first_boot_flag
cat << 'EOF' > /etc/profile.d/disk.sh
bootddev=$(cat /proc/cmdline  | sed  -n  "s#root=\(\/dev\/mmcblk[0-9]\).*#\1#p" )
if [ -f /first_boot_flag ]; then
    echo "first boot flag"
    sd_size=$(parted ${bootddev} print | grep ${bootddev} | cut -d: -f2)
    parted ${bootddev} resizepart 2 ${sd_size}; resize2fs ${bootddev}p2
    rm -rf /first_boot_flag
else
    echo "not exit flag"
fi
mount ${bootddev}p1 /boot
dhcpcd
EOF

exit
sudo tar -czf ubuntu24.tar.gz ubuntu24
# debian13_size="$(( "$(sudo du -sm ubuntu24 | cut -f1 )" + 300 ))"
# mkfs.ext4  -d ubuntu24  -r 1 -N 0 -m 1 -L "rootfs" -O ^64bit ubuntu24.ext4 ${debian13_size}m
# tar -czvf ubuntu24.ext4.tar.gz ubuntu24.ext4

EOFF

    print_red "you need  manually execute the follow commands"
    echo -e ${BLUE}
    cat  tmp.txt
    echo -e  ${COLOR_NONE}
    rm -rf tmp.txt

    print_red "You need to manually execute the above commands one by one on linux(not docker) "
    print_red "referenc doc is <<https://developer.canaan-creative.com/k230/zh/dev/03_other/K230_debian_ubuntu%E8%AF%B4%E6%98%8E.html>>"

}

#放到post build
get_image_last_name()
{
	local distname="$1"
    local K230_SDK_ROOT=$(dirname $(dirname ${BRW_BUILD_DIR}))



	local f="$1"
	local CONF=$(basename ${BRW_BUILD_DIR})
    # echo ${CONF}
    # echo ${K230_SDK_ROOT}
    # exit 1;


	local sdk_ver="v0.0.0";
	local nncase_ver="2.9.0";

	local sdk_ver_file="${K230_SDK_ROOT}/buildroot-overlay/board/canaan/k230-soc/rootfs_overlay/etc/version/release_version"
	local nncase_ver_file="${K230_SDK_ROOT}/output/${CONF}/build/libnncase/nncase/include/nncase/version.h"


	#local storage="$(echo "$f" | sed -nE "s#[^-]*-([^\.]*).*#\1#p")"

	if [ "${CONF}" = "k230_canmv_defconfig" ] ; then
		canaan_site_name="CanMV-K230";
	elif [ "${CONF}" = "k230_evb_defconfig" ] ; then
		canaan_site_name="EVB-K230";
    elif [ "${CONF}" = "k230_canmv_01studio_defconfig" ] ; then
		canaan_site_name="CanMV-K230_01studio";
	else
		canaan_site_name="${CONF%%_defconfig}"	;
	fi



	sdk_ver=$(awk -F- '/^sdk:/ { print $1}' ${sdk_ver_file}  | cut -d: -f2 )

	if [ -e "${nncase_ver_file}" ]; then
		cat ${nncase_ver_file} | grep NNCASE_VERSION -w | cut -d\" -f 2 > /dev/null && \
			nncase_ver=$(cat ${nncase_ver_file} | grep NNCASE_VERSION -w | cut -d\" -f 2)
	fi
    echo   "${canaan_site_name}_${distname}_${sdk_ver}_nncase_v${nncase_ver}.img.gz"
}



#$1  dist name  #debian
#$2  dist_rootfs_ext4 name
#$3  dist_rootfs_ext4 http sit
#$3  dist_rootfs_ext4.tar.gz md5
distribution_rootfs_replace()
{
    local distname="$1"
    local distr_rootfs="$2"  # debian13
    local distr_rootfs_web_site="$3" #
    local md5_v="$4"
    local dist_img_name="${distname}.img"      #debian.img

    if [ "$(id -u)" -ne 0 ]; then
        print_red "permission denied,you need root privileges,example: sudo make debian"
        exit 1;
    fi
    if [ ! -f ${BINARIES_DIR}/sysimage-sdcard.img ]; then
        print_red "you need first build buildroot: make buildroot"
        exit 1;
    fi
    apt-get install parted curl -y

    #set -x;
    cd ${BINARIES_DIR};
    cp sysimage-sdcard.img ${dist_img_name};

    [ -f ${distr_rootfs}.tar.gz ] || wget  ${distr_rootfs_web_site}  -O  ${distr_rootfs}.tar.gz

    [ "${md5_v}" = "$(md5sum ${distr_rootfs}.tar.gz | cut -d' ' -f1 )" ]  || (print_red " ${distr_rootfs}.tar.gz error !" ;exit 1)

    tar -xf ${distr_rootfs}.tar.gz
    cp ${BINARIES_DIR}/../target/lib/modules ${distr_rootfs}/lib -r;


    { # generate ${distr_rootfs}.ext4
        rm -rf ${distr_rootfs}.ext4;
        local rootfs_size="$(( "$(sudo du -sm ${distr_rootfs} | cut -f1 )" + 300 ))"
        mkfs.ext4  -d ${distr_rootfs}  -r 1 -N 0 -m 1 -L "rootfs" -O ^64bit ${distr_rootfs}.ext4 ${rootfs_size}m
    }

    {
        #resize  img
        local img_size=$(( $(wc -c  ${distr_rootfs}.ext4 | awk '{print $1 }')   +  $(wc -c  ${dist_img_name} | awk '{print $1}') ))
        img_size="$((${img_size}/1024/1024+2))"
        truncate ${dist_img_name}  -s $((${img_size}+1))M
        echo -e "Fix\n" | parted ---pretend-input-tty ${dist_img_name} print
        #parted ${dist_img_name}  print free

        # parted ${dist_img_name}  rm 4
        local rootfs_part_id="$(parted ${dist_img_name} print free | grep rootfs | cut -d' ' -f2)"
        parted ${dist_img_name}  resizepart ${rootfs_part_id} ${img_size}MiB 2> /dev/null
    }

    {  #add dist ext4 to image
        local rootfs_off_sect="$(echo -e "unit s\n print free\n" | parted ${dist_img_name}  | grep rootfs | awk '{print $2} ' | cut -ds -f1)"
        dd if=${distr_rootfs}.ext4  of=${dist_img_name} seek=${rootfs_off_sect} conv=notrunc
    }

    #parted ${dist_img_name}  print free
    gzip -f  ${dist_img_name}

    local last_name=$(get_image_last_name ${distname})
    rm -rf ${last_name}; ln -s ${dist_img_name}.gz  ${last_name}


    print_blue "build successfull : ${BINARIES_DIR}/${last_name}"
    chmod a+w ${distr_rootfs}.tar.gz  ${dist_img_name}.gz  ${last_name}

    rm -rf ${distr_rootfs} ${distr_rootfs}.ext4;
}

if $(curl --output /dev/null --silent --head --fail https://ai.b-bug.org/k230/downloads/dl/distribution ) ;then
DISTR_DOWN_URI="https://ai.b-bug.org/k230/downloads/dl/distribution"
else
DISTR_DOWN_URI="https://kendryte-download.canaan-creative.com/k230/downloads/dl/distribution"
fi


if [ "${distribution_type}" =  "debian" ] ;then
    distribution_rootfs_replace  debian   debian13  ${DISTR_DOWN_URI}/debian13.tar.gz  "aeeda080980a6f998526e8e49e786891"
elif [ "${distribution_type}" =  "ubuntu" ] ;then
    distribution_rootfs_replace  ubuntu   ubuntu24  ${DISTR_DOWN_URI}/ubuntu24.tar.gz "32176750a7b7c283af60d5af8abbac63"
elif [ "${distribution_type}" =  "debian_rootfs" ] ;then
    debian_gen_rootfs
elif [ "${distribution_type}" =  "ubuntu_rootfs" ] ;then
    ubuntu_gen_rootfs
else
    echo "${distribution_type}"
fi

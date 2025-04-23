#!/bin/bash
set -xe
#环境：本地vwmare里面安装ubuntu24.04,执行如下脚本
rm -rf debian13
sudo apt install -y qemu-user-static binfmt-support debootstrap debian-ports-archive-keyring systemd-container rsync wget
sudo debootstrap --arch=riscv64  trixie debian13 http://deb.debian.org/debian
sudo chroot debian13 /bin/bash << 'EOFF'
echo "root:root" | chpasswd
echo "k230" > /etc/hostname
echo "deb-src http://deb.debian.org/debian trixie main" >> /etc/apt/sources.list
mount -t devpts devpts /dev/pts
mount -t proc proc /proc
#194MB

#-------locales
apt install  -y   locales
sed -i 's/^#\s*\(en_US.UTF-8 UTF-8\)/\1/' /etc/locale.gen
locale-gen
cat > /etc/default/locale <<EOF
LANG=en_US.UTF-8
LC_ALL=en_US.UTF-8
EOF
#-------locales end 254MB

#-------ssh
apt install  -y     openssh-server
echo "PermitRootLogin yes" >> etc/ssh/sshd_config
systemctl enable  ssh
#-------ssh end 274MB



#-------auto ntpdate
apt install  -y systemd-timesyncd
rm -rf /etc/localtime; ln -s /usr/share/zoneinfo/Asia/Shanghai /etc/localtime;
sed -i -E 's/^#?NTP=.*$/NTP=cn.pool.ntp.org ntp.ntsc.ac.cn  /' /etc/systemd/timesyncd.conf
sed -i -E 's/^#?FallbackNTP=.*$/FallbackNTP=ntp.aliyun.com 0.debian.pool.ntp.org/' /etc/systemd/timesyncd.conf
systemctl enable  systemd-timesyncd
#-------auto ntpdate end 274MB

#-------wifi
apt install -y wpasupplicant
echo "nameserver 8.8.8.8" >> /etc/resolv.conf
echo "nameserver 8.8.4.4" >> /etc/resolv.conf
#-------wifi end 278MB

#-------mount boot part,resize sd part
apt install -y parted libmxml-dev
cat << 'EOF' > /opt/mount_boot.sh
#!/bin/bash
bootddev=$(cat /proc/cmdline  | sed  -n  "s#root=\(\/dev\/mmcblk[0-9]\).*#\1#p" )
echo -e "Fix\n" | parted ---pretend-input-tty ${bootddev} print
sd_size=$(parted ${bootddev} print | grep ${bootddev} | cut -d: -f2)
parted ${bootddev} resizepart 2 ${sd_size}; resize2fs ${bootddev}p2
mount ${bootddev}p1 /boot
EOF
chmod a+x /opt/mount_boot.sh
cat >/etc/systemd/system/mount_boot.service <<EOF
#大模型搜索 debian系统如何 开机自动运行脚本 systemd 就可以了；
[Unit]
Description=mount_boot_part
After=rc-local.service

[Service]
Type=oneshot
ExecStart=/opt/mount_boot.sh
RemainAfterExit=no

[Install]
WantedBy=basic.target
EOF
systemctl enable mount_boot.service
#-------mount boot part,resize sd part end  279MB

#-------etho dhcp
cat >/etc/systemd/network/10-eth0.network <<EOF
[Match]
Name=en*

[Network]
DHCP=yes
EOF

systemctl disable networking
systemctl enable  systemd-networkd
#-------etho dhcp end

#other
apt install  -y libdrm2  libgomp1 libwebp7  libpng-dev  libopus-dev  v4l-utils  ffmpeg usbutils

#pyqt
apt install  -y   qtbase5-dev qtbase5-examples python3-pyqt5
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

#opencv
apt install  -y  python3 python3-pip  python3-opencv

umount  /proc/
umount  /dev/pts
exit
EOFF

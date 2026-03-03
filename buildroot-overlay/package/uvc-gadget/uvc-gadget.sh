#!/bin/sh
# 开启调试，报错即停止
set -e
# Documentation/usb/gadget_uvc.rst
CONFIGFS="/sys/kernel/config"
G="$CONFIGFS/usb_gadget/uvc_g1"
FUNCTION="$G/functions/uvc.usb0"
#UDC_NAME=$(ls /sys/class/udc/)




create_frame() {
        # Example usage:
        # create_frame <width> <height> <group> <format name>

        WIDTH=$1
        HEIGHT=$2
        FORMAT=$3
        NAME=$4

        wdir=${FUNCTION}/streaming/$FORMAT/$NAME/${HEIGHT}p
        #${FUNCTION}/streaming/mjpeg/m/720p

        mkdir -p $wdir
        echo $WIDTH > $wdir/wWidth
        echo $HEIGHT > $wdir/wHeight
        echo $(( $WIDTH * $HEIGHT * 2 )) > $wdir/dwMaxVideoFrameBufferSize
        echo "333333" > $wdir/dwDefaultFrameInterval
        cat <<EOF > $wdir/dwFrameInterval
333333
666666
100000
5000000
EOF
}




# 1. 环境准备
modprobe libcomposite
modprobe usb_f_uvc

# 确保 configfs 已挂载
mount -t configfs none /sys/kernel/config 2>/dev/null || true

# 清理旧的（如果存在则删除，不存在不报错）
if [ -d "$G" ]; then
    echo "" > "$G/UDC" || true
    # ConfigFS 必须递归删除，这里为了简单建议重启板子测试
fi

# 2. 创建 Gadget 根目录
mkdir -p "$G" && cd "$G"

echo 0x29f1 > idVendor
echo 0x0104 > idProduct
echo 0x0200 > bcdUSB
echo 0xEF > bDeviceClass
echo 0x02 > bDeviceSubClass
echo 0x01 > bDeviceProtocol

mkdir -p strings/0x409
echo "12345678" > strings/0x409/serialnumber
echo "Canaan" > strings/0x409/manufacturer
echo "K230 UVC" > strings/0x409/product

# 3. 创建 UVC 功能实例
mkdir -p $FUNCTION


# 设置传输参数
# streaming_interval sets bInterval. Values range from 1..255
echo 1 > $FUNCTION/streaming_interval
# streaming_maxpacket sets wMaxPacketSize. Valid values are 1024/2048/3072
echo 1024 > $FUNCTION/streaming_maxpacket
# streaming_maxburst sets bMaxBurst. Valid values are 1..15
echo 4 > $FUNCTION/streaming_maxburst
echo "k230 linux uvc" > $FUNCTION/function_name



#1.Formats and Frames
create_frame 640 360 mjpeg mjpeg
create_frame 1280 720 mjpeg mjpeg
create_frame 1920 1080 mjpeg mjpeg
create_frame 640 360 uncompressed yuyv
create_frame 1280 720 uncompressed yuyv
create_frame 1920 1080 uncompressed yuyv


# #2 Create a new Color Matching Descriptor

# mkdir $FUNCTION/streaming/color_matching/yuyv

# echo 1 > $FUNCTION/streaming/color_matching/yuyv/bColorPrimaries
# echo 1 > $FUNCTION/streaming/color_matching/yuyv/bTransferCharacteristics
# echo 4 > $FUNCTION/streaming/color_matching/yuyv/bMatrixCoefficients

# # Create a symlink to the Color Matching Descriptor from the format's config item
# ln -s $FUNCTION/streaming/color_matching/yuyv $FUNCTION/streaming/uncompressed/yuyv


#3 Header linking
mkdir $FUNCTION/streaming/header/h

# This section links the format descriptors and their associated frames
# to the header
cd $FUNCTION/streaming/header/h
ln -s ../../uncompressed/yuyv
ln -s ../../mjpeg/mjpeg

# This section ensures that the header will be transmitted for each
# speed's set of descriptors. If support for a particular speed is not
# needed then it can be skipped here.
cd ../../class/fs
ln -s ../../header/h
cd ../../class/hs
ln -s ../../header/h
cd ../../class/ss
ln -s ../../header/h
cd ../../../control
mkdir header/h
ln -s header/h class/fs
ln -s header/h class/ss

cd "$G";




# 7. 绑定到配置项
mkdir -p configs/c.1/strings/0x409
echo "Config 1" > configs/c.1/strings/0x409/configuration
echo 500 > configs/c.1/MaxPower

# 建立功能到配置的最终连接
ln -s ${FUNCTION} configs/c.1/f1

# 8. 启动！
sleep 1
echo "91500000.usb" > UDC

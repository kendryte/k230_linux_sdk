#!/bin/sh
# Documentation/usb/gadget_uvc.rst
CONFIGFS="/sys/kernel/config"
G="$CONFIGFS/usb_gadget/uvc_g1"
FUNCTION="$G/functions/uvc.usb0"

create_frame() {
    # Example usage:
    # create_frame <width> <height> <group> <format name>

    WIDTH=$1
    HEIGHT=$2
    FORMAT=$3
    NAME=$4

    wdir=${FUNCTION}/streaming/$FORMAT/$NAME/${HEIGHT}p
    mkdir -p $wdir
    echo $WIDTH > $wdir/wWidth
    echo $HEIGHT > $wdir/wHeight
    # For uncompressed formats (YUYV), 2 bytes per pixel
    # For compressed formats (MJPEG, H.264, H.265), use larger buffer
    case "$FORMAT" in
        uncompressed)
            case "$WIDTH" in
            1920)
                echo "$(( $WIDTH * $HEIGHT * 2 ))"  >  $wdir/dwMaxVideoFrameBufferSize
                echo "29491200" > $wdir/dwMaxBitRate
                echo "29491200" > $wdir/dwMinBitRate
            ;;   # 320KB for 1080p
            1280)
                echo "$(( $WIDTH * $HEIGHT * 2 ))" >  $wdir/dwMaxVideoFrameBufferSize
                echo "29491200" > $wdir/dwMaxBitRate
                echo "29491200" > $wdir/dwMinBitRate
            ;;   # 160KB for 720p
            640)
                echo "$(( $WIDTH * $HEIGHT * 2 ))" >  $wdir/dwMaxVideoFrameBufferSize
                echo "55296000" > $wdir/dwMaxBitRate
                echo "55296000" > $wdir/dwMinBitRate
            ;;    # 80KB for 360p
            *)
                echo "$(( $WIDTH * $HEIGHT * 2 )) " >  $wdir/dwMaxVideoFrameBufferSize
                echo "10240000" > $wdir/dwMaxBitRate
                echo "10240000" > $wdir/dwMinBitRate
            ;;   # default 160KB
            esac
        ;;

        mjpeg)
            # MJPEG buffer sizing based on resolution
            # 1080p: ~100-250KB, 720p: ~50-150KB, 360p: ~10-30KB
            case "$WIDTH" in
                1920)
                    echo "300000"  >  $wdir/dwMaxVideoFrameBufferSize  # ~300KB for 1080p
                    echo "80000000" > $wdir/dwMaxBitRate                # 30Mbps @ 30fps
                    echo "5000000"  > $wdir/dwMinBitRate
                ;;
                1280)
                    echo "150000" >  $wdir/dwMaxVideoFrameBufferSize   # ~150KB for 720p
                    echo "80000000" > $wdir/dwMaxBitRate                # 20Mbps @ 30fps
                    echo "3000000"  > $wdir/dwMinBitRate
                ;;
                640)
                    echo "50000" >  $wdir/dwMaxVideoFrameBufferSize    # ~50KB for 360p
                    echo "80000000" > $wdir/dwMaxBitRate                 # 10Mbps @ 30fps
                    echo "2000000"  > $wdir/dwMinBitRate
                ;;
                *)
                    echo "100000" >  $wdir/dwMaxVideoFrameBufferSize
                    echo "10000000" > $wdir/dwMaxBitRate
                    echo "2000000"  > $wdir/dwMinBitRate
                ;;
            esac
        ;;
        *)
        echo $(( $WIDTH * $HEIGHT * 2 )) > $wdir/dwMaxVideoFrameBufferSize
        ;;
    esac
    echo "333333" > $wdir/dwDefaultFrameInterval
    echo "333333" > $wdir/dwFrameInterval
}

start_uvc_gadget() {
    # 1. 环境准备
    modprobe libcomposite
    modprobe usb_f_uvc

    # 确保 configfs 已挂载
    mount -t configfs none /sys/kernel/config 2>/dev/null || true

    # 重置 dwc2 USB 控制器（防止之前 gadget 的状态残留导致 panic）
    if [ -e /sys/bus/platform/drivers/dwc2/unbind ]; then
        echo "91500000.usb" > /sys/bus/platform/drivers/dwc2/unbind 2>/dev/null || true
        sleep 1
        echo "91500000.usb" > /sys/bus/platform/drivers/dwc2/bind 2>/dev/null || true
        sleep 1
    fi

    # 清理旧的 gadget（如果存在）
    if [ -d "$G" ]; then
            echo "" > "$G/UDC" 2>/dev/null || true
            rmdir "$G" 2>/dev/null || true
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
    echo "UVC Gadget" > strings/0x409/product

    # 3. 创建 UVC 功能实例
    mkdir -p $FUNCTION

    # 设置传输参数
    echo 2 > $FUNCTION/streaming_interval
    echo 1024 > $FUNCTION/streaming_maxpacket
    echo 4 > $FUNCTION/streaming_maxburst
    echo "k230 linux uvc" > $FUNCTION/function_name

    # 4. 创建帧描述符 (MJPEG format)
    create_frame 640 360 mjpeg mjpeg
    create_frame 1280 720 mjpeg mjpeg
    create_frame 1920 1080 mjpeg mjpeg

    # 5. Header linking
    mkdir $FUNCTION/streaming/header/h
    cd $FUNCTION/streaming/header/h
    ln -s ../../mjpeg/mjpeg

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

    cd "$G"

    # 6. 绑定到配置项
    mkdir -p configs/c.1/strings/0x409
    echo "Config 1" > configs/c.1/strings/0x409/configuration
    echo 500 > configs/c.1/MaxPower
    ln -s ${FUNCTION} configs/c.1/f1

    # 7. 启动！
    echo "91500000.usb" > UDC

    echo "UVC gadget started"

    # 8. 启动 uvc-gadget 应用
    /usr/bin/uvc-gadget -v /dev/video1 &
}

stop_uvc_gadget() {
    # 1. 停止 uvc-gadget 应用
    killall uvc-gadget 2>/dev/null || true

    # 2. 解绑 UDC
    if [ -e "${G}/UDC" ]; then
        echo "" > "${G}/UDC" 2>/dev/null || true
    fi

    # 3. 删除 function 链接
    rm -f "${G}/configs/c.1/f1" 2>/dev/null

    # 4. 删除 function
    rmdir "${G}/functions/uvc.usb0" 2>/dev/null || true

    # 5. 删除 config/strings 目录
    rmdir "${G}/configs/c.1/strings/0x409" 2>/dev/null || true
    rmdir "${G}/configs/c.1" 2>/dev/null || true
    rmdir "${G}/strings/0x409" 2>/dev/null || true

    # 6. 删除 gadget
    cd / && rmdir "${G}" 2>/dev/null || true

    # 7. 等待 USB 连接完全断开
    sleep 1

    # 8. 卸载模块（先卸载功能模块，再卸载 composite）
    rmmod usb_f_uvc 2>/dev/null || true
    rmmod libcomposite 2>/dev/null || true

    # 9. 卸载 configfs
    umount /sys/kernel/config 2>/dev/null || true

    echo "UVC gadget stopped"
}

case "$1" in
    start)
        /etc/init.d/S99adb_mtp stop
        start_uvc_gadget
        ;;
    stop)
        stop_uvc_gadget
        ;;
    restart)
        stop_uvc_gadget
        sleep 1
        start_uvc_gadget
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}" >&2
        exit 3
        ;;
esac

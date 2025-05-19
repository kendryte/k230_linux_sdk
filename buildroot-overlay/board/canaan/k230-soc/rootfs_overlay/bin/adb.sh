#!/bin/sh
#adb.exe device
#adb.exe shell
case "$1" in
  start)
    modprobe libcomposite.ko
    modprobe usb_f_fs.ko

    test -d /sys/kernel/config || mkdir /sys/kernel/config
    mount -t configfs none /sys/kernel/config
    mkdir /sys/kernel/config/usb_gadget/g1
    cd /sys/kernel/config/usb_gadget/g1

    echo 0x2207 > idVendor
    echo 0x0006 > idProduct

    mkdir strings/0x409
    echo 0123456789ABCDEF > strings/0x409/serialnumber

    echo "canaan" > strings/0x409/manufacturer
    echo "k230" > strings/0x409/product

    mkdir configs/b.1
    mkdir configs/b.1/strings/0x409
    echo "adb" > configs/b.1/strings/0x409/configuration
    #echo "Conf 1" > configs/b.1/strings/0x409/configuration
    #echo 120 > configs/b.1/MaxPower

    cur_path=`pwd`
    mkdir -p $cur_path'/functions/ffs.adb'
    ln -s /sys/kernel/config/usb_gadget/g1/functions/ffs.adb /sys/kernel/config/usb_gadget/g1/configs/b.1/ffs.adb

    mkdir -p /dev/usb-ffs/adb
    mount -o uid=2000,gid=2000 -t functionfs adb /dev/usb-ffs/adb

    cd /

    adbd &

    UDC=`ls /sys/class/udc/| awk 'NR==1 {print $1}'`
    sleep 1 && echo $UDC > /sys/kernel/config/usb_gadget/g1/UDC &
    ;;
  stop)
    echo none > /sys/kernel/config/usb_gadget/g1/UDC
    ;;
  restart|reload|force-reload)
    echo "Error: argument '$1' not supported" >&2
    exit 3
    ;;
  *)
    echo "Usage: adbd start|stop" >&2
    exit 3
    ;;
esac

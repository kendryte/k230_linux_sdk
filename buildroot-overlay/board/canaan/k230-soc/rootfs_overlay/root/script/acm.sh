#!/bin/sh
# ACM (Serial) USB Gadget Script
CONFIGFS="/sys/kernel/config"
G="$CONFIGFS/usb_gadget/g1"

start_acm_gadget() {
    modprobe libcomposite.ko
    modprobe usb_f_acm.ko

    test -d $CONFIGFS || mkdir $CONFIGFS
    mount -t configfs none $CONFIGFS 2>/dev/null || true

    # Clean up old gadget
    if [ -d "$G" ]; then
        echo "" > "$G/UDC" 2>/dev/null || true
        rmdir "$G" 2>/dev/null || true
    fi

    mkdir "$G"
    cd "$G"

    echo 0x29F1 > idVendor  # canaan
    echo 0x0106 > idProduct  # ACM specific PID
    echo 0x0100 > bcdDevice
    echo 0x0200 > bcdUSB
    echo 0x00 > bDeviceClass
    echo 0x00 > bDeviceSubClass
    echo 0x00 > bDeviceProtocol

    mkdir strings/0x409
    echo 0123456789ABCDEF > strings/0x409/serialnumber
    echo "canaan" > strings/0x409/manufacturer
    echo "k230_acm" > strings/0x409/product

    mkdir configs/c.1
    mkdir configs/c.1/strings/0x409
    echo "Conf 1" > configs/c.1/strings/0x409/configuration
    echo 100 > configs/c.1/MaxPower

    cur_path=`pwd`

    mkdir -p $cur_path'/functions/acm.usb0'
    ln -s $cur_path/functions/acm.usb0 $cur_path/configs/c.1/acm.usb0

    echo "91500000.usb" > UDC

    echo "ACM gadget started - /dev/ttyGS0 available"
}

stop_acm_gadget() {
    if [ -e "${G}/UDC" ]; then
        echo "" > "${G}/UDC" 2>/dev/null || true
    fi

    rm -f "${G}/configs/c.1/acm.usb0" 2>/dev/null
    rmdir "${G}/functions/acm.usb0" 2>/dev/null || true
    rmdir "${G}/configs/c.1/strings/0x409" 2>/dev/null || true
    rmdir "${G}/configs/c.1" 2>/dev/null || true
    rmdir "${G}/strings/0x409" 2>/dev/null || true
    cd / && rmdir "${G}" 2>/dev/null || true

    sleep 1

    rmmod usb_f_acm.ko 2>/dev/null || true
    rmmod libcomposite.ko 2>/dev/null || true
    umount $CONFIGFS 2>/dev/null || true

    echo "ACM gadget stopped"
}

case "$1" in
    start)
        start_acm_gadget
        ;;
    stop)
        stop_acm_gadget
        ;;
    restart)
        stop_acm_gadget
        sleep 1
        start_acm_gadget
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}" >&2
        exit 3
        ;;
esac

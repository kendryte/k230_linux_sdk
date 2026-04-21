    modprobe libcomposite.ko
    modprobe usb_f_fs.ko

    test -d /sys/kernel/config || mkdir /sys/kernel/config
    mount -t configfs none /sys/kernel/config
    mkdir /sys/kernel/config/usb_gadget/g1
    cd /sys/kernel/config/usb_gadget/g1

    echo 0x29F1 > idVendor  #canaan
    echo 0x0105 > idProduct
    echo 0x0100 > bcdDevice
    echo 0x0200 > bcdUSB

    mkdir strings/0x409
    echo 0123456789ABCDEF > strings/0x409/serialnumber
    echo "canaan" > strings/0x409/manufacturer
    echo "k230_linux" > strings/0x409/product

    mkdir configs/c.1
    mkdir configs/c.1/strings/0x409
    echo "Conf 1" > configs/c.1/strings/0x409/configuration
    #echo 120 > configs/c.1/MaxPower

    cur_path=`pwd`

    mkdir -p $cur_path'/functions/rndis.usb0'
    ln -s /sys/kernel/config/usb_gadget/g1/functions/rndis.usb0 /sys/kernel/config/usb_gadget/g1/configs/c.1/rndis.usb0

	echo "91500000.usb" > UDC
	ifconfig usb0 192.168.88.1 netmask 255.255.255.0

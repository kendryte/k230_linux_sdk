
set -ex
s="${1:-192.168.1.1}"
p="${2:-12345678790}"
u="wangjianxin"
t="/home/wangjianxin/k230/k230_linux_sdk/output/k230_canmv_01studio_defconfig/target"

sshpass  -p "$p" scp $u@$s:${t}/../../../buildroot-overlay/package/uvc-gadget/uvc-gadget.sh .
sshpass  -p "$p" scp $u@$s:${t}/usr/lib/libuvcgadget.so.0.4.0  /usr/lib
sshpass  -p "$p" scp $u@$s:${t}/usr/bin/uvc-gadget   /usr/bin

sshpass  -p "$p" scp $u@$s:${t}/usr/lib/libvvcam.so   /usr/lib
sshpass  -p "$p" scp $u@$s:${t}/../images/k230-canmv-01studio-lcd.dtb   /boot/
sshpass  -p "$p" scp $u@$s:${t}/../images/Image   /boot/




sh uvc-gadget.sh
uvc-gadget -v /dev/video1:mjpeg:1920:1080

uvc-gadget -v /dev/video1:mjpeg:1280:720

exit 1;

killall isp_media_server

cat /proc/vsi/isp_subdev0

ISP_MEDIA_SENSOR_DRIVER=/usr/lib/libvvcam.so /usr/bin/isp_media_server
ISP_MEDIA_SENSOR_DRIVER=/usr/lib/libvvcam.so /usr/bin/isp_media_server > /dev/null 2> /tmp/isp.err.log &

v4l2-drm -d 1 -n 5 -w 1920 -h 1080
v4l2-drm -d 1 -n 5 -w 240 -h 240

echo 0 mode=1 > /proc/vsi/isp_subdev0

echo 0 sensor=imx335 > /proc/vsi/isp_subdev0
echo 0 mode=0 > /proc/vsi/isp_subdev0
echo 0 xml=/etc/vvcam/imx335.xml > /proc/vsi/isp_subdev0
echo 0 manu_json=/etc/vvcam/imx335.xml > /proc/vsi/isp_subdev0
echo 0 auto_json=/etc/vvcam/imx335.xml > /proc/vsi/isp_subdev0

echo 0 sensor=gc2093 > /proc/vsi/isp_subdev0
echo 0 mode=0 > /proc/vsi/isp_subdev0
echo 0 xml=/etc/vvcam/gc2093-1920x1080.xml > /proc/vsi/isp_subdev0
echo 0 manu_json=/etc/vvcam/gc2093-1920x1080_manual.json > /proc/vsi/isp_subdev0
echo 0 auto_json=/etc/vvcam/gc2093-1920x1080_auto.json > /proc/vsi/isp_subdev0


gpioset 1 30=1;sleep 0.5;gpioset 1 30=0;sleep 0.5;gpioset 1 30=1;
vglite_drm
vglite_cube

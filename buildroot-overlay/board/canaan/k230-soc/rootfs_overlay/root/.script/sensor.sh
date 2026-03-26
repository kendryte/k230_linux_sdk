s="${1:-192.168.1.1}"
killall isp_media_server
scp wangjianxin@$s:/home/wangjianxin/k230/k230_linux_sdk/output/k230_canmv_01studio_defconfig/build/vvcam/buildroot-build/libvvcam.so  /usr/lib/
scp wangjianxin@$s:/home/wangjianxin/k230/k230_linux_sdk/output/k230_canmv_01studio_defconfig/images/boot/k230-canmv-01studio-lcd.dtb  /boot/k230-canmv-01studio-lcd.dtb
scp wangjianxin@$s:/home/wangjianxin/k230/k230_linux_sdk/output/k230_canmv_01studio_defconfig/images/Image  /boot/Image


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

#!/bin/bash
/etc/vvcam/S41adb_mtp  start
modprobe vvcam_isp
modprobe vvcam_mipi
modprobe vvcam_vb
modprobe vvcam_isp_subdev
modprobe vvcam_video
#ISP_MEDIA_SENSOR_DRIVER=/usr/lib/libvvcam.so
ISP_MEDIA_SENSOR_DRIVER=/lib/riscv64-linux-gnu/libvvcam.so /usr/bin/isp_media_server_debian  >/tmp/isp.err.log  2>&1 &

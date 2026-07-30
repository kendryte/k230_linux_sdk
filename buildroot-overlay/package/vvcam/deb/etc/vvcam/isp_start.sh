#!/bin/bash
/etc/vvcam/S41adb_mtp  start
modprobe vvcam_isp
modprobe vvcam_mipi
modprobe vvcam_vb
modprobe vvcam_isp_subdev
modprobe vvcam_video  mcm_mask=1

# Debian + CONFIG_RT_GROUP_SCHED: default sched_rt_runtime_us blocks SCHED_RR
# even for root (chrt EPERM). Disable bandwidth limit so isp_media_server
# can create SCHED_RR IRQ/media threads (needed to avoid motion jitter).
if [ -w /proc/sys/kernel/sched_rt_runtime_us ]; then
	echo -1 > /proc/sys/kernel/sched_rt_runtime_us
fi

#ISP_MEDIA_SENSOR_DRIVER=/usr/lib/libvvcam.so
ISP_MEDIA_SENSOR_DRIVER=/lib/riscv64-linux-gnu/libvvcam.so /usr/bin/isp_media_server  >/tmp/isp.err.log  2>&1 &

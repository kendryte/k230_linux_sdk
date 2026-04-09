#!/bin/sh

dev_name=$(cat /sys/class/video4linux/$MDEV/name 2>/dev/null)

case "$dev_name" in
    *nonai*2d*|*non-ai-2d*)
        link_name="video_nonai_2d"
        ;;
    *mvx*|*Linlon*)
        link_name="video_vpu"
        ;;
    *vvcam*|*verisilicon*)
        link_name="$dev_name"
        ;;
    *)
        exit 0
        ;;
esac

cd /dev
rm -f "$link_name"
ln -sf "$MDEV" "$link_name"

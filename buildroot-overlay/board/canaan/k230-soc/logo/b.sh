#!/bin/bash
#sudo apt-get update;sudo apt-get install ffmpeg -y
ffmpeg -i logo.png -vf "transpose=2" -pix_fmt nv12 -f rawvideo logo_new.yuv  #逆时针 90,png to yuv nv12;
ffmpeg -f rawvideo -pix_fmt nv12 -s 800x480 -i logo.yuv -vf "transpose=1" -frames:v 1 logo_new.png #顺时针90，nv12-->png

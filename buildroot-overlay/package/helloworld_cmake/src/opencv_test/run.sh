#!/bin/bash
set -x
./inter 1.jpg 100
./media_blur 1.jpg 100
./hist 1.jpg 100
./warpaffine 1.jpg 100
./angle_main 1.jpg 100


./inter 2.jpg 100
./media_blur 2.jpg 100
./hist 2.jpg 100
./warpaffine 2.jpg 100
./angle_main 2.jpg 100

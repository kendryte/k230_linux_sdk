cd /home/wangjianxin/k230/k230_linux_sdk/buildroot-overlay/package/python-k230/lvgl


S=../../../../output/k230_canmv_01studio_defconfig/host/riscv64-buildroot-linux-gnu/sysroot/usr/include
B=../../../../output/k230_canmv_01studio_defconfig/build/lvgl-97a99a8affe966617f048830a007bbf8bea63da6/buildroot-build

python3 codegen_tools/gen_pybind11.py \
    -I $S/lvgl -I $S/lvgl_private -I $S  -I $B \
    -D LV_CONF_PATH=\"lv_conf.h\" \
    --compile-commands $B/compile_commands.json \
    -o lvgl_pybind_generated.cpp \
    --wrapper-output _wrapper.py \
    $S/lvgl/lvgl.h

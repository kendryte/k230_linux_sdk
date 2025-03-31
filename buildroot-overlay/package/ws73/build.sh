#!/bin/sh

BUILD_DIR="$1"
mkdir -p $BUILD_DIR/output/bin $BUILD_DIR/output/build
python3 $BUILD_DIR/build/scripts/hconfig.py $BUILD_DIR/Kconfig $BUILD_DIR/ws73_default.config -a $BUILD_DIR/output/bin/autoconfig.h
cp -f $BUILD_DIR/build/config/ws73_cfg_default.ini $BUILD_DIR/output/bin/ws73_cfg.ini
python3 $BUILD_DIR/build/scripts/hconfig_to_ini.py $BUILD_DIR/build/config/ini_map.csv $BUILD_DIR/Kconfig $BUILD_DIR/ws73_default.config $BUILD_DIR/output/bin/ws73_cfg.ini
make -C $BUILD_DIR platform wifi
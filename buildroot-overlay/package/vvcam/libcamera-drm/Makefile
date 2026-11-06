SDK ?= $(abspath ../../../../)
HOST := $(SDK)/output/k230_canmv_01studio_defconfig/host
CXX := $(HOST)/bin/riscv64-unknown-linux-gnu-g++
PKG_CONFIG := $(HOST)/bin/pkg-config

CPPFLAGS := $(shell $(PKG_CONFIG) --cflags libcamera display)
LDLIBS := $(shell $(PKG_CONFIG) --libs libcamera display) -ldrm -lpthread
LC_CPPFLAGS := $(shell $(PKG_CONFIG) --cflags libcamera) -I$(abspath ../include)
LC_LDLIBS := $(shell $(PKG_CONFIG) --libs libcamera) -lpthread

.PHONY: all clean
all: vvcam-libcamera-drm vvcam-libcamera-hold vvcam-libcamera-grab

vvcam-libcamera-drm: vvcam-libcamera-drm.cpp
	$(CXX) -std=c++20 -O2 -Wall -Wextra $(CPPFLAGS) -o $@ $< $(LDLIBS)

vvcam-libcamera-hold: vvcam-libcamera-hold.cpp
	$(CXX) -std=c++20 -O2 -Wall -Wextra $(LC_CPPFLAGS) -o $@ $< $(LC_LDLIBS)

vvcam-libcamera-grab: vvcam-libcamera-grab.cpp
	$(CXX) -std=c++20 -O2 -Wall -Wextra $(LC_CPPFLAGS) -o $@ $< $(LC_LDLIBS)

clean:
	rm -f vvcam-libcamera-drm vvcam-libcamera-hold vvcam-libcamera-grab

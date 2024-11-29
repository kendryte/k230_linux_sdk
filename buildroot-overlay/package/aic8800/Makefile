CONFIG_AIC_LOADFW_SUPPORT := m
CONFIG_AIC8800_WLAN_SUPPORT := m

obj-$(CONFIG_AIC_LOADFW_SUPPORT)    += aic_load_fw/
obj-$(CONFIG_AIC8800_WLAN_SUPPORT) += aic8800_fdrv/

########## config option ##########
export CONFIG_USE_FW_REQUEST = n
export CONFIG_PREALLOC_RX_SKB = n
export CONFIG_PREALLOC_TXQ = y
###################################

########## platform support list ##########
export CONFIG_PLATFORM_ROCKCHIP = n
export CONFIG_PLATFORM_ALLWINNER = n
export CONFIG_PLATFORM_AMLOGIC = n
export CONFIG_PLATFORM_HI = n
export CONFIG_PLATFORM_UBUNTU = y

ifeq ($(CONFIG_PLATFORM_ROCKCHIP), y)
ARCH = arm64
KDIR = /home/yaya/E/Rockchip/3566/firefly/Android11.0/Firefly-RK356X_Android11.0_git_20210824/RK356X_Android11.0/kernel
CROSS_COMPILE = /home/yaya/E/Rockchip/3566/firefly/Android11.0/Firefly-RK356X_Android11.0_git_20210824/RK356X_Android11.0/prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
ccflags-y += -DANDROID_PLATFORM
endif

ifeq ($(CONFIG_PLATFORM_ALLWINNER), y)
KDIR  = /home/yaya/E/Allwinner/R818/R818/AndroidQ/lichee/kernel/linux-4.9
ARCH = arm64
CROSS_COMPILE = /home/yaya/E/Allwinner/R818/R818/AndroidQ/lichee/out/gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
ccflags-y += -DANDROID_PLATFORM
endif

ifeq ($(CONFIG_PLATFORM_AMLOGIC), y)
ccflags-y += -DANDROID_PLATFORM
ARCH = arm
CROSS_COMPILE = /home/yaya/D/Workspace/CyberQuantum/JinHaoYue/amls905x3/SDK/20191101-0tt-asop/android9.0/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin/arm-linux-androidkernel-
KDIR = /home/yaya/D/Workspace/CyberQuantum/JinHaoYue/amls905x3/SDK/20191101-0tt-asop/android9.0/out/target/product/u202/obj/KERNEL_OBJ/
endif

ifeq ($(CONFIG_PLATFORM_HI), y)
ccflags-y += -DANDROID_PLATFORM
ARCH = arm
CROSS_COMPILE = /home/yaya/D/Workspace/CyberQuantum/JinHaoYue/amls905x3/SDK/20191101-0tt-asop/android9.0/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin/arm-linux-androidkernel-
KDIR = /home/yaya/D/Workspace/CyberQuantum/JinHaoYue/amls905x3/SDK/20191101-0tt-asop/android9.0/out/target/product/u202/obj/KERNEL_OBJ/
endif

ifeq ($(CONFIG_PLATFORM_UBUNTU), y)
KDIR  = /lib/modules/$(shell uname -r)/build
PWD   = $(shell pwd)
KVER = $(shell uname -r)
MODDESTDIR = /lib/modules/$(KVER)/kernel/drivers/net/wireless/aic8800
SUBARCH = $(shell uname -m | sed -e s/i.86/i386/ -e s/armv.l/arm/ -e s/aarch64/arm64/)
ARCH ?= $(SUBARCH)
CROSS_COMPILE ?=
endif

###########################################

MAKEFLAGS +=-j$(shell nproc)

all: modules
modules:
	make -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

install:
	mkdir -p $(MODDESTDIR)
	install -p -m 644 aic_load_fw/aic_load_fw.ko  $(MODDESTDIR)/
	install -p -m 644 aic8800_fdrv/aic8800_fdrv.ko  $(MODDESTDIR)/
	/sbin/depmod -a ${KVER}

uninstall:
	rm -rfv $(MODDESTDIR)/aic_load_fw.ko
	rm -rfv $(MODDESTDIR)/aic8800_fdrv.ko
	/sbin/depmod -a ${KVER}

clean:
	cd aic_load_fw/;make clean;cd ..
	cd aic8800_fdrv/;make clean;cd ..
	rm -rf modules.order Module.symvers .modules.order.cmd .Module.symvers.cmd .tmp_versions/

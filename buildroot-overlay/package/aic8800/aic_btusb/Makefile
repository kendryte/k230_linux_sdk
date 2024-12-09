CONFIG_AIC8800_BTUSB = m
MODULE_NAME = aic_btusb


obj-$(CONFIG_AIC8800_BTUSB) := $(MODULE_NAME).o
# $(MODULE_NAME)-y := \
#     aic_btusb.o

all: modules
modules:
#	make -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) LLVM=1 LLVM_IAS=1 modules
	make -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
install:
	mkdir -p $(MODDESTDIR)
	install -p -m 644 $(MODULE_NAME).ko  $(MODDESTDIR)
	/sbin/depmod -a ${KVER}

uninstall:
	rm -rfv $(MODDESTDIR)/$(MODULE_NAME).ko
	/sbin/depmod -a ${KVER}

clean:
	rm -rf *.o *.ko *.o.* *.mod.* modules.* Module.* .a* .o* .*.o.* *.mod .tmp* .cache.mk .modules.order.cmd .Module.symvers.cmd

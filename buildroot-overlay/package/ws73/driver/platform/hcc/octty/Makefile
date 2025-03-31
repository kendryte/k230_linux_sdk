include $(WSCFG_KCONFIG_CONFIG)
cur_dir = $(PWD)
securec_dir =  $(cur_dir)/../../securec
securec_inc = $(securec_dir)/include
securec_src = $(securec_dir)/src
target = octty
CFLAG =-I$(securec_inc)
all_src := octty.c
all_src += $(wildcard $(securec_src)/*.c)
ifeq ($(WSCFG_CROSS_COMPILE),)
$(error "WSCFG_CROSS_COMPILE not define")
endif
LOCAL_CROSS_COMPILE := $(subst ",,$(WSCFG_CROSS_COMPILE))
LOCALCC := $(LOCAL_CROSS_COMPILE)gcc
all:
	$(LOCALCC) -o $(target) $(all_src) $(CFLAG)

clean:
	rm -f $(cur_dir)/$(target)

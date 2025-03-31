# CROSS 为交叉编译工具链
CROSS = arm-himix100-linux-
CC	= @echo " GCC	$@"; $(CROSS)gcc
LD	= @echo " LD	$@"; $(CROSS)ld
AR	= @echo " AR	$@"; $(CROSS)ar
RM	= @echo " RM	$@"; rm -f
STRIP	= @echo "STRIP  $@"; $(CROSS)strip

# INCDIR_COMMON 和 INCDIR_BLE 为头文件路径
# INCDIR_SEC 为安全函数路径
TOPDIR = ./
INCDIR_TOP = $(TOPDIR)/inc/
INCDIR_COMMON = ../../../../include/bsle/common/
INCDIR_OSAL = ../../../../driver/platform/osal/include/
INCDIR_BLE = ../../../../include/bsle/ble/
INCDIR_SEC = ../../../../driver/platform/libc_sec/include/

SRCDIR = $(TOPDIR)/src/
LIBDIR = $(TOPDIR)/lib/

CFLAGS	+=	-I$(INCDIR_TOP) -I$(INCDIR_COMMON) -I$(INCDIR_OSAL) -I$(INCDIR_BLE) -I$(INCDIR_SEC) 
CFLAGS	+= -Wall -Os #--std=c++11
LDFLAGS	+= -lm -lpthread -ldl -Wl,--gc-sections

SRCS := $(wildcard $(SRCDIR)/*.c)
SRCS += $(wildcard $(TOPDIR)/*.c)
OBJS := $(patsubst %.c,%.o,$(filter %.c,$(SRCS)))

TARGET	= $(TOPDIR)/ble_server_sample

all: $(TARGET)

$(TARGET): $(OBJS)
	echo -e "\e[36m<):$@; <):$^\e[0m"
	$(info $($(LIBDIR)/*.a))
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -Xlinker "-(" $(LIBDIR)/*.a -Xlinker "-)"
	$(STRIP) --strip-unneeded -s $@

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS) $(EXEC_OBJS);
	find ./ -type f \( -name '*.elf*' -o -name '*.gdb' \) | xargs rm -f

# $(RM) $@;
# $(STRIP) $@

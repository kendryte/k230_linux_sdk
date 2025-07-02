#
# Makefile
#
# CPP=riscv64-unknown-linux-gnu-g++
VO_INIT_DIR_NAME ?= vo_init
VO_INIT_DEMO_DIR ?= ${shell pwd}

BIN = vo_init

INCS+=-I./include -I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/drm
LIBPATH += -L$(STAGING_DIR)/usr/lib -L./

LIBS += -ldisplay -lpthread

#LDFLAGS += -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DNO_OPENSSL=1

SRCS:=$(wildcard *.cpp)
CPPOBJS:=$(SRCS:.cpp=.o)

all:$(BIN)
	@rm -f $(CPPOBJS)
	#cp $(BIN) ~/nfs

$(CPPOBJS) : %.o: %.cpp
	$(CPP) -c $(CFLAGS) $< -o $@ $(INCS)

$(BIN):$(CPPOBJS)
	$(CPP) $(LDFLAGS) -o $(BIN) $(CPPOBJS) $(LIBPATH) $(LIBS)
	@rm -f  $(CPPOBJS)

clean:
	@rm -f $(BIN) $(CPPOBJS)

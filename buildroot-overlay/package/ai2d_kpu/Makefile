#CXX=/opt/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.1/bin/riscv64-unknown-linux-gnu-g++

OBJ = main.cpp

LIBDIR = -L ../libnncase/nncase/lib -L ../libmmz
LIBS   = -lnncase.rt_modules.k230 -lNncase.Runtime.Native -lfunctional_k230 -lmmz -lpthread
INCLUDE = -I ./ -I ../libnncase/nncase -I ../libnncase/nncase/include -I ../libmmz

#CXX_FLAG += -g  -mabi=lp64d -mcmodel=medany -std=c++17 -DLINUX_RUNTIME -Wno-multichar -Wno-unused-result
CXX_FLAG +=  -march=rv64imafdcv -mabi=lp64d -mcmodel=medany -std=c++17 -DLINUX_RUNTIME -Wno-multichar -Wno-unused-result
CXX_FLAG +=  -mcpu=c908v -mabi=lp64d  -mtune=c908  -mrvv-v0p10-compatible  -mrvv-auto-vectorize

all:
	$(CXX)  $(CXX_FLAG) $(INCLUDE) $(OBJ) -o ai2d_kpu.elf  $(LIBDIR)  $(LIBS)

clean:
	rm -f *.o *.elf

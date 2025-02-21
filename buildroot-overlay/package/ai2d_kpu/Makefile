CXX=/opt/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.2/bin/riscv64-unknown-linux-gnu-g++

OBJ = main.cpp

LIBDIR = -L ../libnncase/nncase/lib -L ../libmmz
LIBS   = -l nncase.rt_modules.k230 -l Nncase.Runtime.Native -lfunctional_k230 -lmmz -lpthread
INCLUDE = -I ./ -I ../libnncase/nncase -I ../libnncase/nncase/include -I ../libmmz

CXX_FLAG = -O2 -march=rv64imafdcv -mabi=lp64d -mcmodel=medany -std=c++17 -DLINUX_RUNTIME -Wno-multichar -Wno-unused-result

all:
	$(CXX) $(CXX_FLAG) $(INCLUDE) $(OBJ) -o ai2d_kpu.elf  $(LIBDIR)  $(LIBS)

clean:
	rm -f *.o *.elf

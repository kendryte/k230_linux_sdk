CXX=/opt/toolchain/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V2.10.1/bin/riscv64-unknown-linux-gnu-g++

OBJ = main.cc model.cc mobile_retinaface.cc anchors_320.cc util.cc

LIBDIR = -L ../libnncase/runtime_lib/nncase/lib -L ../libmmz
LIBS   = -l nncase.rt_modules.k230 -l Nncase.Runtime.Native -lfunctional_k230 -lmmz -lpthread
INCLUDE = -I ./ -I ../libnncase/runtime_lib -I ../libnncase/runtime_lib/nncase/include -I ../libmmz

CXX_FLAG = -O2 -march=rv64imafdcv -mabi=lp64d -mcmodel=medany -std=c++17 -DLINUX_RUNTIME

all:
	$(CXX) $(CXX_FLAG) $(INCLUDE) $(OBJ) -o face_detect.elf  $(LIBDIR)  $(LIBS)

clean:
	rm -f *.o *.elf

CFLAG := -mcpu=c908v -O2

ifeq ($(CC), cc)
	CC=riscv64-unknown-linux-gnu-gcc
endif

ifeq ($(AR), ar)
	AR=riscv64-unknown-linux-gnu-ar
endif

all:
	echo $(CC)
	$(CC) $(CFLAG) -c mmz.c
	$(AR) rcs libmmz.a mmz.o

clean:
	rm -f *.o
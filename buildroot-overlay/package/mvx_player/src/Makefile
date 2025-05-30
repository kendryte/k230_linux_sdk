ifeq ($(BR2_RISCV_32), y)
CFLAGS += -march=rv32gcv_xtheadc
LDFLAGS += -march=rv32gcv_xtheadc
else
CFLAGS += -mcpu=c908v -O2
LDFLAGS += -mcpu=c908v -O2
endif

PROG_1 = mvx_encoder
PROG_2 = mvx_encoder_multi
PROG_3 = mvx_decoder
PROG_4 = mvx_decoder_multi

all: ${PROG_1} ${PROG_2} ${PROG_3} ${PROG_4}
.PHONY: all

${PROG_1}: mvx_encoder.cpp mvx_player.cpp md5.c mvx_argparse.c
	$(CXX) $(CFLAGS) $^ -lstdc++ -Wl,-rpath . -lpthread  -o $@

${PROG_2}: mvx_encoder_multi.cpp mvx_player.cpp md5.c mvx_argparse.c
	$(CXX) $(CFLAGS) $^ -lstdc++ -Wl,-rpath . -lpthread  -o $@

${PROG_3}: mvx_decoder.cpp mvx_player.cpp md5.c mvx_argparse.c
	$(CXX) $(CFLAGS) $^ -lstdc++ -Wl,-rpath . -lpthread  -o $@

${PROG_4}: mvx_decoder_multi.cpp mvx_player.cpp md5.c mvx_argparse.c
	$(CXX) $(CFLAGS) $^ -lstdc++ -Wl,-rpath . -lpthread  -o $@

clean:
	rm -f *.o
	rm -f *.d
	rm -f ${PROG_1} ${PROG_2} ${PROG_3} ${PROG_4}


/*
 * Throughput/latency benchmark: K230 hardware SHA256 (drivers/crypto/canaan/
 * kendryte-hash.c, registered as "sha256-kendryte") vs. the kernel's generic
 * software SHA256 ("sha256-generic"), both driven through the AF_ALG hash
 * socket interface (CONFIG_CRYPTO_USER_API_HASH). Binding to the exact
 * cra_driver_name (rather than the generic "sha256") pins each run to one
 * implementation regardless of cra_priority, so "sha256" resolving to the
 * hardware driver (cra_priority 300) can't quietly swap in for the "sha256-
 * generic" run.
 *
 * Must run on actual K230 hardware with CONFIG_CRYPTO_DEV_KENDRYTE_HASH
 * probed -- there is no SoC-accurate emulator for this here.
 *
 * Build (cross-compile for the target):
 *   riscv64-unknown-linux-gnu-gcc -O2 -Wall -static \
 *       -o kendryte_sha256_bench kendryte_sha256_bench.c
 *
 * Run on the board:
 *   ./kendryte_sha256_bench
 *
 * Method: for each buffer size, open one AF_ALG hash socket for the given
 * driver and repeat write(full buffer) + read(32-byte digest) -- this is
 * the documented AF_ALG hash semantics: a send without MSG_MORE marks end
 * of data, the following recv() both returns the digest and resets the
 * socket to start a fresh hash, so the same fd can be reused for every
 * iteration without paying socket()/bind()/accept() per hash. Each size is
 * run for up to BENCH_SECONDS wall-clock time (capped at BENCH_MAX_ITERS
 * iterations) and reported as MB/s and average per-call latency.
 */

#include <sys/socket.h>
#include <linux/if_alg.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DIGEST_LEN 32
#define BENCH_SECONDS 1.5
#define BENCH_MAX_ITERS 2000000

static const size_t sizes[] = {
	16, 64, 256, 1024, 4096, 16384, 65536, 262144, 1048576, 4194304,
};

static int open_hash_socket(const char *driver_name)
{
	struct sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type = "hash",
	};
	int sock, op;

	strncpy((char *)sa.salg_name, driver_name, sizeof(sa.salg_name) - 1);

	sock = socket(AF_ALG, SOCK_SEQPACKET, 0);
	if (sock < 0) {
		perror("socket(AF_ALG)");
		return -1;
	}

	if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) != 0) {
		fprintf(stderr, "bind(AF_ALG, %s): %s\n", driver_name,
			strerror(errno));
		close(sock);
		return -1;
	}

	op = accept(sock, NULL, 0);
	if (op < 0) {
		perror("accept");
		close(sock);
		return -1;
	}

	close(sock);
	return op;
}

static double now_sec(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec / 1e9;
}

/* Returns MB/s, or -1.0 on error. *out_iters / *out_elapsed report the raw run. */
static double bench_one(const char *driver_name, size_t size, long *out_iters,
			 double *out_elapsed)
{
	unsigned char *buf = malloc(size ? size : 1);
	unsigned char digest[DIGEST_LEN];
	double t0, t1;
	long iters = 0;
	int op;

	if (!buf) {
		fprintf(stderr, "malloc(%zu) failed\n", size);
		return -1.0;
	}
	memset(buf, 0xA5, size);

	op = open_hash_socket(driver_name);
	if (op < 0) {
		free(buf);
		return -1.0;
	}

	t0 = now_sec();
	for (;;) {
		if (write(op, buf, size) != (ssize_t)size) {
			perror("write");
			iters = -1;
			break;
		}
		if (read(op, digest, DIGEST_LEN) != DIGEST_LEN) {
			perror("read");
			iters = -1;
			break;
		}
		iters++;
		if (iters >= BENCH_MAX_ITERS)
			break;
		/* Only check the clock every so often; clock_gettime() itself
		 * isn't free and we don't want to perturb small-buffer timing.
		 */
		if ((iters & 0xff) == 0) {
			t1 = now_sec();
			if (t1 - t0 >= BENCH_SECONDS)
				break;
		}
	}
	t1 = now_sec();

	close(op);
	free(buf);

	if (iters <= 0)
		return -1.0;

	*out_iters = iters;
	*out_elapsed = t1 - t0;

	return ((double)size * iters / (1024.0 * 1024.0)) / (t1 - t0);
}

int main(void)
{
	size_t i;

	printf("%-10s %14s %14s %10s %12s %12s\n", "size", "hw MB/s",
	       "sw MB/s", "speedup", "hw us/op", "sw us/op");

	for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
		size_t size = sizes[i];
		long hw_iters = 0, sw_iters = 0;
		double hw_elapsed = 0, sw_elapsed = 0;
		double hw_mbps = bench_one("sha256-kendryte", size, &hw_iters,
					    &hw_elapsed);
		double sw_mbps = bench_one("sha256-generic", size, &sw_iters,
					    &sw_elapsed);

		if (hw_mbps < 0 || sw_mbps < 0) {
			printf("%-10zu %s\n", size, "FAILED (see stderr)");
			continue;
		}

		printf("%-10zu %14.2f %14.2f %9.2fx %12.2f %12.2f\n", size,
		       hw_mbps, sw_mbps, hw_mbps / sw_mbps,
		       hw_elapsed * 1e6 / hw_iters, sw_elapsed * 1e6 / sw_iters);
	}

	return 0;
}

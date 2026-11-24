/*
 * Userspace test for drivers/crypto/canaan/kendryte-hash.c, driven through
 * the kernel's AF_ALG hash socket interface (CONFIG_CRYPTO_USER_API_HASH).
 *
 * Must be run on actual K230 hardware (or whatever target actually probes
 * "canaan,k230-hash") with the module built in/loaded -- it cannot be run
 * in this review environment, there is no board or SoC-accurate emulator
 * here. Binds explicitly to the driver name "sha256-kendryte" rather than
 * the generic "sha256" so it always exercises this driver specifically,
 * regardless of what else is registered or its cra_priority.
 *
 * Build (cross-compile for the target):
 *   riscv64-unknown-linux-gnu-gcc -O2 -Wall -pthread \
 *       -o kendryte_sha256_test kendryte_sha256_test.c
 * (adjust the compiler prefix to whatever toolchain this SDK ships)
 *
 * Run on the board:
 *   ./kendryte_sha256_test
 *
 * What it checks:
 *   1. Correctness against known-good SHA256 digests (computed offline
 *      with Python's hashlib, not anything from this kernel tree) for a
 *      range of lengths chosen to land on the block-boundary cases the
 *      segment() buffering logic in kendryte-hash.c has to handle: 0, 1,
 *      55, 56, 63, 64, 65, 72, 96, 100, 127, 128, 129, 192, 256, 1000,
 *      4096 bytes.
 *   2. The same data fed in one shot vs. in small irregular chunks via
 *      repeated sendmsg(MSG_MORE), to exercise the multi-call update()
 *      buffering path (hash_ctx->buff) instead of only the single-call
 *      path.
 *   3. A concurrency stress test: N threads each repeatedly hash their
 *      own distinct buffer and compare against a digest for that exact
 *      buffer computed once up front, single-threaded. This re-validates
 *      the fix for the "shared DMA buffer" bug (issue #45): a wrong
 *      digest here means one request's data leaked into another's.
 *
 * Exit status is 0 only if every check passed.
 */

#include <sys/socket.h>
#include <linux/if_alg.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DRIVER_NAME "sha256-kendryte"
#define DIGEST_LEN 32

struct vector {
	int len;
	const char *hex_digest; /* trusted reference, from Python hashlib */
};

/*
 * Reference digests for the pattern data[i] = i % 256, generated with:
 *   python3 -c "import hashlib; print(hashlib.sha256(bytes(i % 256 for i
 *   in range(L))).hexdigest())"
 * -- i.e. a well-known, independent SHA256 implementation, not anything
 * from this kernel tree or this driver.
 */
static const struct vector vectors[] = {
	{ 0, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" },
	{ 1, "6e340b9cffb37a989ca544e6bb780a2c78901d3fb33738768511a30617afa01d" },
	{ 55, "463eb28e72f82e0a96c0a4cc53690c571281131f672aa229e0d45ae59b598b59" },
	{ 56, "da2ae4d6b36748f2a318f23e7ab1dfdf45acdc9d049bd80e59de82a60895f562" },
	{ 63, "29af2686fd53374a36b0846694cc342177e428d1647515f078784d69cdb9e488" },
	{ 64, "fdeab9acf3710362bd2658cdc9a29e8f9c757fcf9811603a8c447cd1d9151108" },
	{ 65, "4bfd2c8b6f1eec7a2afeb48b934ee4b2694182027e6d0fc075074f2fabb31781" },
	{ 72, "107de2bc788e11029f7851f8e1b0b5afb4e34379c709fc840689ebd3d1f51b5b" },
	{ 96, "08359b108fa567f5dcf319fa3434da6abbc1d595f426372666447f09cc5a87dc" },
	{ 100, "bce0aff19cf5aa6a7469a30d61d04e4376e4bbf6381052ee9e7f33925c954d52" },
	{ 127, "92ca0fa6651ee2f97b884b7246a562fa71250fedefe5ebf270d31c546bfea976" },
	{ 128, "471fb943aa23c511f6f72f8d1652d9c880cfa392ad80503120547703e56a2be5" },
	{ 129, "5099c6a56203f9687f7d33f4bfdf576d31dc91f6b695ecea38b2770c87631135" },
	{ 192, "8b4a544837a1a0280fa8a7c82865c27a1064b3cc6281fda0753566b9bb104a87" },
	{ 256, "40aff2e9d2d8922e47afd4648e6967497158785fbd1da870e7110266bf944880" },
	{ 1000, "a8af099bf2e878609558dbf69d8f88f4a31040a8cf84b549a0cfa912f12ffc3f" },
	{ 4096, "c8f5d0341d54d951a71b136e6e2afcb14d11ed8489a7ae126a8fee0df6ecf193" },
};

static int g_failures;

static void fill_pattern(unsigned char *buf, int len, unsigned char marker)
{
	int i;

	for (i = 0; i < len; i++)
		buf[i] = (unsigned char)((marker != 0) ? marker : (i % 256));
}

static void hex_to_bin(const char *hex, unsigned char *out, int outlen)
{
	int i;

	for (i = 0; i < outlen; i++)
		sscanf(hex + i * 2, "%2hhx", &out[i]);
}

/* Open a bound+accepted AF_ALG hash socket for DRIVER_NAME. */
static int open_hash_socket(void)
{
	struct sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type = "hash",
		.salg_name = DRIVER_NAME,
	};
	int sock, op;

	sock = socket(AF_ALG, SOCK_SEQPACKET, 0);
	if (sock < 0) {
		perror("socket(AF_ALG)");
		return -1;
	}

	if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) != 0) {
		perror("bind(AF_ALG, " DRIVER_NAME ")");
		fprintf(stderr,
			"-> is CONFIG_CRYPTO_DEV_KENDRYTE_HASH actually probed on this board?\n");
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

/* Hash `len` bytes of `data`, sent as `nchunks` sendmsg() calls. */
static int hash_chunked(const unsigned char *data, int len, int nchunks,
			 unsigned char *digest_out)
{
	int op = open_hash_socket();
	int i, sent = 0, ret = -1;

	if (op < 0)
		return -1;

	if (nchunks < 1)
		nchunks = 1;

	for (i = 0; i < nchunks; i++) {
		int chunk = (len - sent) / (nchunks - i);
		int last = (i == nchunks - 1);
		struct msghdr msg = { 0 };
		struct iovec iov = { .iov_base = (void *)(data + sent),
				      .iov_len = chunk };

		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;

		if (sendmsg(op, &msg, last ? 0 : MSG_MORE) != chunk) {
			perror("sendmsg");
			goto out;
		}
		sent += chunk;
	}

	if (read(op, digest_out, DIGEST_LEN) != DIGEST_LEN) {
		perror("read(digest)");
		goto out;
	}

	ret = 0;
out:
	close(op);
	return ret;
}

static void dump_hex(const unsigned char *b, int len)
{
	int i;

	for (i = 0; i < len; i++)
		fprintf(stderr, "%02x", b[i]);
}

static int run_vector(const struct vector *v, int nchunks)
{
	unsigned char *data = malloc(v->len ? v->len : 1);
	unsigned char expected[DIGEST_LEN], got[DIGEST_LEN];

	fill_pattern(data, v->len, 0);
	hex_to_bin(v->hex_digest, expected, DIGEST_LEN);

	if (hash_chunked(data, v->len, nchunks, got) != 0) {
		fprintf(stderr, "len=%d chunks=%d: request failed\n", v->len,
			nchunks);
		free(data);
		return -1;
	}

	if (memcmp(expected, got, DIGEST_LEN) != 0) {
		fprintf(stderr, "len=%d chunks=%d: MISMATCH\n  expected: ",
			v->len, nchunks);
		dump_hex(expected, DIGEST_LEN);
		fprintf(stderr, "\n  got:      ");
		dump_hex(got, DIGEST_LEN);
		fprintf(stderr, "\n");
		free(data);
		return -1;
	}

	free(data);
	return 0;
}

static void test_correctness(void)
{
	size_t i;

	printf("== correctness (single-shot) ==\n");
	for (i = 0; i < sizeof(vectors) / sizeof(vectors[0]); i++) {
		int rc = run_vector(&vectors[i], 1);

		printf("  len=%-5d %s\n", vectors[i].len,
		       rc == 0 ? "PASS" : "FAIL");
		if (rc != 0)
			g_failures++;
	}

	printf("== correctness (chunked update(), 7 pieces) ==\n");
	for (i = 0; i < sizeof(vectors) / sizeof(vectors[0]); i++) {
		int rc = run_vector(&vectors[i], 7);

		printf("  len=%-5d %s\n", vectors[i].len,
		       rc == 0 ? "PASS" : "FAIL");
		if (rc != 0)
			g_failures++;
	}
}

/* ---- concurrency stress test (re-validates the issue #45 fix) ---- */

#define STRESS_THREADS 8
#define STRESS_ITERS 500
#define STRESS_BUFLEN 4096

struct stress_ctx {
	int thread_id;
	unsigned char expected[DIGEST_LEN];
	int wrong_digests;
	int errors;
};

static void *stress_worker(void *arg)
{
	struct stress_ctx *ctx = arg;
	unsigned char *data = malloc(STRESS_BUFLEN);
	int i;

	/* each thread hashes its own distinct buffer, never anyone else's */
	fill_pattern(data, STRESS_BUFLEN, (unsigned char)(0x11 * (ctx->thread_id + 1)));

	for (i = 0; i < STRESS_ITERS; i++) {
		unsigned char got[DIGEST_LEN];

		if (hash_chunked(data, STRESS_BUFLEN, 3, got) != 0) {
			ctx->errors++;
			continue;
		}
		if (memcmp(got, ctx->expected, DIGEST_LEN) != 0)
			ctx->wrong_digests++;
	}

	free(data);
	return NULL;
}

static void test_concurrency(void)
{
	pthread_t threads[STRESS_THREADS];
	struct stress_ctx ctxs[STRESS_THREADS];
	int i, total_wrong = 0, total_errors = 0;

	printf("== concurrency stress (%d threads x %d iters x %d bytes) ==\n",
	       STRESS_THREADS, STRESS_ITERS, STRESS_BUFLEN);

	for (i = 0; i < STRESS_THREADS; i++) {
		unsigned char *data = malloc(STRESS_BUFLEN);

		ctxs[i].thread_id = i;
		ctxs[i].wrong_digests = 0;
		ctxs[i].errors = 0;

		fill_pattern(data, STRESS_BUFLEN,
			     (unsigned char)(0x11 * (i + 1)));
		/* reference digest computed single-threaded, up front */
		if (hash_chunked(data, STRESS_BUFLEN, 1, ctxs[i].expected) != 0) {
			fprintf(stderr,
				"failed to compute reference digest for thread %d\n",
				i);
			exit(1);
		}
		free(data);
	}

	for (i = 0; i < STRESS_THREADS; i++)
		pthread_create(&threads[i], NULL, stress_worker, &ctxs[i]);
	for (i = 0; i < STRESS_THREADS; i++)
		pthread_join(threads[i], NULL);

	for (i = 0; i < STRESS_THREADS; i++) {
		printf("  thread %d: %d wrong digests, %d request errors\n",
		       i, ctxs[i].wrong_digests, ctxs[i].errors);
		total_wrong += ctxs[i].wrong_digests;
		total_errors += ctxs[i].errors;
	}

	if (total_wrong || total_errors) {
		printf("concurrency stress: FAIL (%d wrong digests, %d errors)\n",
		       total_wrong, total_errors);
		g_failures++;
	} else {
		printf("concurrency stress: PASS\n");
	}
}

int main(void)
{
	test_correctness();
	test_concurrency();

	if (g_failures) {
		printf("\n%d check(s) FAILED\n", g_failures);
		return 1;
	}

	printf("\nall checks PASSED\n");
	return 0;
}

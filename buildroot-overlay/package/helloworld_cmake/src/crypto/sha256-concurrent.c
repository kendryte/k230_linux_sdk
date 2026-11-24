// Concurrent SHA-256 through the kernel crypto API, to exercise whatever
// implementation the API selects for a caller like dm-verity.
//
// Background: drivers/crypto/canaan/kendryte-hash.c registers "sha256" at
// cra_priority 300, which outranks the generic implementation's 100, so the
// crypto API hands it to dm-verity. Its DMA buffer lives in two file-scope
// variables shared by every caller, nothing serialises the calls, the
// dma_alloc_coherent() return is not checked, and the buffer is freed in the
// same function that gives the engine its physical address. Two concurrent
// callers therefore overwrite each other's pointer and free each other's
// buffer.
//
// AF_ALG reaches the same ahash interface dm-verity uses, so running several
// threads here selects the same implementation and makes the same requests,
// without needing a verity device.
//
// Build:
//   riscv64-unknown-linux-gnu-gcc -O2 -Wall -Wextra -pthread
//       -o sha256-concurrent sha256-concurrent.c
// Run:
//   ./sha256-concurrent [threads] [iterations] [buffer_bytes]
//
// Each thread hashes a buffer only it uses, whose digest was computed by a
// single-threaded pass first. A digest that comes back wrong is one request's
// data reaching another request. On a kernel where the accelerator is selected,
// expect kernel oopses in the kverityd-equivalent path rather than a clean
// mismatch -- the mismatch is the benign end of the same race.
//
// Which implementation is in use: /proc/crypto, the "driver" line under
// "name: sha256" with the highest priority. dm-verity also prints it at mount:
//   device-mapper: verity: sha256 using implementation "..."

#define _GNU_SOURCE
#include <errno.h>
#include <linux/if_alg.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DIGEST 32

static unsigned n_threads = 4;
static unsigned n_iter = 2000;
static size_t buf_len = 4096;

static int alg_socket(void)
{
    struct sockaddr_alg sa = {
        .salg_family = AF_ALG,
        .salg_type = "hash",
        .salg_name = "sha256",
    };
    int fd = socket(AF_ALG, SOCK_SEQPACKET, 0);
    if (fd < 0) {
        perror("socket(AF_ALG)");
        return -1;
    }
    if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("bind(sha256) -- is CONFIG_CRYPTO_USER_API_HASH enabled?");
        close(fd);
        return -1;
    }
    return fd;
}

static int hash_once(int alg_fd, const unsigned char *in, size_t len,
                     unsigned char out[DIGEST])
{
    int op = accept(alg_fd, NULL, 0);
    if (op < 0) {
        perror("accept");
        return -1;
    }
    ssize_t n = send(op, in, len, 0);
    if (n != (ssize_t)len) {
        fprintf(stderr, "send: %zd of %zu (%s)\n", n, len, strerror(errno));
        close(op);
        return -1;
    }
    n = read(op, out, DIGEST);
    close(op);
    if (n != DIGEST) {
        fprintf(stderr, "read: %zd of %d (%s)\n", n, DIGEST, strerror(errno));
        return -1;
    }
    return 0;
}

struct work {
    unsigned id;
    unsigned char *buf;
    unsigned char expect[DIGEST];
    unsigned long mismatches;
    unsigned long errors;
    unsigned long done;
};

static void *run(void *arg)
{
    struct work *w = arg;
    unsigned char got[DIGEST];
    int fd = alg_socket();

    if (fd < 0)
        return NULL;
    for (unsigned i = 0; i < n_iter; i++) {
        if (hash_once(fd, w->buf, buf_len, got) < 0) {
            w->errors++;
            continue;
        }
        if (memcmp(got, w->expect, DIGEST) != 0)
            w->mismatches++;
        w->done++;
    }
    close(fd);
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc > 1) n_threads = (unsigned)strtoul(argv[1], NULL, 0);
    if (argc > 2) n_iter = (unsigned)strtoul(argv[2], NULL, 0);
    if (argc > 3) buf_len = strtoul(argv[3], NULL, 0);
    if (n_threads == 0 || n_iter == 0 || buf_len == 0) {
        fprintf(stderr, "usage: %s [threads] [iterations] [buffer_bytes]\n",
                argv[0]);
        return 2;
    }

    printf("sha256 through AF_ALG: %u threads x %u iterations x %zu bytes\n",
           n_threads, n_iter, buf_len);

    struct work *w = calloc(n_threads, sizeof(*w));
    pthread_t *t = calloc(n_threads, sizeof(*t));
    if (!w || !t) {
        perror("calloc");
        return 1;
    }

    // One buffer per thread, distinct contents, and the reference digest taken
    // single-threaded so the accelerator has no concurrency to get wrong yet.
    int fd = alg_socket();
    if (fd < 0)
        return 1;
    for (unsigned i = 0; i < n_threads; i++) {
        w[i].id = i;
        w[i].buf = malloc(buf_len);
        if (!w[i].buf) {
            perror("malloc");
            return 1;
        }
        for (size_t j = 0; j < buf_len; j++)
            w[i].buf[j] = (unsigned char)(j * 31u + i * 101u);
        if (hash_once(fd, w[i].buf, buf_len, w[i].expect) < 0) {
            fprintf(stderr, "reference digest for thread %u failed\n", i);
            return 1;
        }
    }
    close(fd);
    printf("reference digests taken single-threaded, starting %u threads\n",
           n_threads);

    for (unsigned i = 0; i < n_threads; i++) {
        if (pthread_create(&t[i], NULL, run, &w[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    unsigned long done = 0, mism = 0, errs = 0;
    for (unsigned i = 0; i < n_threads; i++) {
        pthread_join(t[i], NULL);
        done += w[i].done;
        mism += w[i].mismatches;
        errs += w[i].errors;
        if (w[i].mismatches || w[i].errors)
            printf("  thread %u: %lu done, %lu wrong digests, %lu errors\n",
                   i, w[i].done, w[i].mismatches, w[i].errors);
    }

    printf("%lu hashes, %lu wrong digests, %lu errors\n", done, mism, errs);
    return (mism || errs) ? 1 : 0;
}

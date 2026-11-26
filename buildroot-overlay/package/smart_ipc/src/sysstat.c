/*
 * sysstat - CPU and memory, sampled while the camera runs. See sysstat.h
 * for why these particular numbers.
 *
 * Every reading here is a small file under /proc or /sys. They are all
 * generated on read, so the cost is a seq_file walk rather than disk I/O,
 * but it is not free: this is called once every few seconds, never per
 * frame.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define _GNU_SOURCE

#include "sysstat.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <time.h>
#include <unistd.h>

static double now_sec(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

/*
 * Read a whole small file into buf and null-terminate it. Everything under
 * /proc and /sys that this file reads comes back in one go; a short read is
 * treated as the whole thing because none of these ever gets near the
 * buffer size.
 */
static bool slurp(const char *path, char *buf, size_t size)
{
	FILE *f;
	size_t n;

	f = fopen(path, "re");
	if (f == NULL)
		return false;

	n = fread(buf, 1, size - 1, f);
	fclose(f);

	buf[n] = '\0';

	return n > 0;
}

/* ------------------------------------------------------------------ *
 * CPU
 * ------------------------------------------------------------------ */

/*
 * The first line of /proc/stat is the sum over all cores:
 *
 *   cpu  user nice system idle iowait irq softirq steal guest guest_nice
 *
 * Busy is everything except idle and iowait. iowait counts as idle on
 * purpose: the core really was available, it just had nothing to run, and
 * counting it as busy is the classic way to make a storage stall look like
 * a CPU problem.
 */
static bool read_cpu(unsigned long long *busy, unsigned long long *total)
{
	char buf[256];
	unsigned long long v[8] = { 0 };
	int got;
	int i;

	if (!slurp("/proc/stat", buf, sizeof(buf)))
		return false;

	got = sscanf(buf, "cpu %llu %llu %llu %llu %llu %llu %llu %llu", &v[0],
		     &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7]);
	if (got < 4)
		return false;

	*total = 0;
	for (i = 0; i < got; i++)
		*total += v[i];

	*busy = *total - v[3] - v[4];

	return true;
}

static unsigned count_cpus(void)
{
	char buf[4096];
	unsigned n = 0;
	const char *p;

	if (!slurp("/proc/stat", buf, sizeof(buf)))
		return 1;

	/* "cpu0", "cpu1", ... one line each, after the summary line */
	for (p = buf; (p = strstr(p, "cpu")) != NULL; p++)
		if (p[3] >= '0' && p[3] <= '9')
			n++;

	return n > 0 ? n : 1;
}

/* ------------------------------------------------------------------ *
 * Memory
 * ------------------------------------------------------------------ */

struct meminfo {
	unsigned long total_kb;
	unsigned long avail_kb;
};

/*
 * /proc/meminfo in one pass. The keys are matched at the start of a line so
 * that one key cannot be found inside another key's value, and the whole file
 * is walked once rather than once per key.
 *
 * CmaTotal and CmaFree are deliberately not read here. See read_cma() for
 * why CmaFree is not an answer to "how much is left".
 */
static bool read_meminfo(struct meminfo *mi)
{
	char buf[4096];
	char *line, *next;
	const struct {
		const char *key;
		size_t len;
		unsigned long *out;
	} want[] = {
		{ "MemTotal:", 9, &mi->total_kb },
		{ "MemAvailable:", 13, &mi->avail_kb },
	};
	unsigned i;

	memset(mi, 0, sizeof(*mi));

	if (!slurp("/proc/meminfo", buf, sizeof(buf)))
		return false;

	for (line = buf; line != NULL && *line != '\0'; line = next) {
		next = strchr(line, '\n');
		if (next != NULL)
			*next++ = '\0';

		for (i = 0; i < sizeof(want) / sizeof(want[0]); i++)
			if (strncmp(line, want[i].key, want[i].len) == 0)
				sscanf(line + want[i].len, "%lu",
				       want[i].out);
	}

	return mi->total_kb > 0;
}

struct cmainfo {
	unsigned long long used_kb;	/* really held by hardware right now */
	unsigned long long free_kb;
	unsigned long long total_kb;
	unsigned long long maxchunk_kb;	/* largest single free run */
	unsigned long long fail;	/* allocations that came back empty */
	bool valid;			/* the three above mean anything */
};

/*
 * How much of the CMA area the hardware is actually holding.
 *
 * CmaFree in /proc/meminfo does NOT answer this and must not be printed as
 * if it did. CMA is reserved for the camera, the encoder and the display,
 * but the kernel lends the idle part of it to anything movable - in
 * practice the page cache - and evicts those pages when the hardware
 * finally asks. A borrowed page is "not free", so CmaFree collapses to
 * almost nothing while the hardware has taken hardly any of it. Measured on
 * this board with the camera stopped: CmaFree said 15.9 MB left, while the
 * hardware was holding 1.4 MB of 52 MB, i.e. 50.6 MB really was available.
 * CmaFree was understating the headroom by 35 MB.
 *
 * The honest numbers come from the CMA bitmap, which only cma_alloc() sets
 * and only cma_release() clears, so borrowing cannot touch it:
 *
 *   count      pages in the area
 *   used       pages the bitmap has marked out, i.e. handed to hardware
 *   maxchunk   longest run of free pages. This is the one that decides
 *              whether the next allocation succeeds: hardware needs the
 *              memory contiguous, so 20 MB free in small pieces still
 *              cannot satisfy a request for 8 MB.
 *
 * All three are already shifted to units of pages by the kernel
 * (order_per_bit is applied there), so they only need scaling by the page
 * size. They live in debugfs, which needs CONFIG_CMA_DEBUGFS=y and a mount;
 * see sysstat_init(). If they cannot be read, valid stays false and the
 * caller says so rather than falling back to CmaFree.
 */
static void read_cma(struct cmainfo *ci, long page_kb)
{
	static const char dbg[] = "/sys/kernel/debug/cma";
	static const char sysfs[] = "/sys/kernel/mm/cma";
	DIR *d;
	struct dirent *e;

	memset(ci, 0, sizeof(*ci));

	d = opendir(dbg);
	if (d != NULL) {
		while ((e = readdir(d)) != NULL) {
			char path[sizeof(dbg) + sizeof(e->d_name) + 32];
			char buf[64];
			unsigned long long count = 0, used = 0, maxchunk = 0;

			if (e->d_name[0] == '.')
				continue;

			snprintf(path, sizeof(path), "%s/%s/count", dbg,
				 e->d_name);
			if (!slurp(path, buf, sizeof(buf)) ||
			    sscanf(buf, "%llu", &count) != 1)
				continue;

			snprintf(path, sizeof(path), "%s/%s/used", dbg,
				 e->d_name);
			if (!slurp(path, buf, sizeof(buf)) ||
			    sscanf(buf, "%llu", &used) != 1)
				continue;

			snprintf(path, sizeof(path), "%s/%s/maxchunk", dbg,
				 e->d_name);
			if (slurp(path, buf, sizeof(buf)))
				sscanf(buf, "%llu", &maxchunk);

			ci->total_kb += count * (unsigned long long)page_kb;
			ci->used_kb += used * (unsigned long long)page_kb;

			/*
			 * Summed over areas for used and total, but the
			 * biggest run is a max, not a sum: two areas with
			 * 4 MB free each cannot serve an 8 MB request.
			 */
			maxchunk *= (unsigned long long)page_kb;
			if (maxchunk > ci->maxchunk_kb)
				ci->maxchunk_kb = maxchunk;

			ci->valid = true;
		}

		closedir(d);
	}

	if (ci->valid)
		ci->free_kb = ci->total_kb - ci->used_kb;

	/*
	 * The failure count is a separate thing and comes from sysfs, which
	 * is always there. It is cumulative since boot and never decremented,
	 * which is exactly what makes it useful: a failure at any point in the
	 * run is still visible afterwards.
	 */
	d = opendir(sysfs);
	if (d == NULL)
		return;

	while ((e = readdir(d)) != NULL) {
		char path[sizeof(sysfs) + sizeof(e->d_name) + 32];
		char buf[64];
		unsigned long long v;

		if (e->d_name[0] == '.')
			continue;

		snprintf(path, sizeof(path), "%s/%s/alloc_pages_fail", sysfs,
			 e->d_name);
		if (slurp(path, buf, sizeof(buf)) &&
		    sscanf(buf, "%llu", &v) == 1)
			ci->fail += v;
	}

	closedir(d);
}

/* ------------------------------------------------------------------ */

void sysstat_init(struct sysstat *s)
{
	memset(s, 0, sizeof(*s));

	s->ncpu = count_cpus();

	s->page_kb = sysconf(_SC_PAGESIZE) / 1024;
	if (s->page_kb <= 0)
		s->page_kb = 4;

	s->primed = read_cpu(&s->cpu_busy, &s->cpu_total);
	s->wall = now_sec();

	/*
	 * The CMA bitmap is in debugfs, which the board does not mount at
	 * boot. Mount it here rather than leave the only honest CMA figures
	 * unreachable; if it is already mounted this fails with EBUSY and
	 * nothing is lost. Needs root, which this program already is.
	 */
	if (access("/sys/kernel/debug/cma", F_OK) != 0)
		mount("none", "/sys/kernel/debug", "debugfs", 0, NULL);
}

void sysstat_report(struct sysstat *s, double elapsed)
{
	unsigned long long busy = 0, total = 0;
	struct meminfo mi;
	struct cmainfo ci;
	double wall;
	double cpu_all = 0.0;

	wall = now_sec();

	if (read_cpu(&busy, &total) && s->primed && total > s->cpu_total)
		cpu_all = 100.0 * (double)(busy - s->cpu_busy) /
			  (double)(total - s->cpu_total);

	s->cpu_busy = busy;
	s->cpu_total = total;
	s->wall = wall;
	s->primed = true;

	/*
	 * One line, comma-separated fields. avail and cma use the same slash
	 * shape - what is left over the total - so the two read alike.
	 */
	printf("[%6.1fs] cpu %.0f%%", elapsed, cpu_all);

	if (read_meminfo(&mi))
		printf(", avail %.1fM/%.1fM", (double)mi.avail_kb / 1024.0,
		       (double)mi.total_kb / 1024.0);

	read_cma(&ci, s->page_kb);

	if (ci.valid)
		printf(", cma %.1fM/%.1fM, max block %.1fM",
		       (double)ci.free_kb / 1024.0,
		       (double)ci.total_kb / 1024.0,
		       (double)ci.maxchunk_kb / 1024.0);
	else
		printf(", cma n/a");

	/*
	 * Only once it has happened. A column that reads 0 forever stops
	 * being looked at, and this is the one figure here that has to be
	 * noticed the moment it moves: a channel that could not get memory,
	 * while the picture still looks fine. The count is cumulative and
	 * never goes back down, so once it appears it stays on every line
	 * after.
	 */
	if (ci.fail > 0)
		printf("  *** CMA ALLOC FAILED, %llu pages ***", ci.fail);

	printf("\n");
}

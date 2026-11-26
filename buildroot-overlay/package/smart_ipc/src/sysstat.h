/*
 * sysstat - what the board is spending while the camera runs
 *
 * CPU and memory, sampled from /proc and /sys and printed every few
 * seconds. Nothing here touches the camera; it is a separate file so that
 * smart_ipc.c does not grow a /proc parser.
 *
 * Which numbers are worth printing, and which ones lie, is settled in
 * doc/linux_memory/, the 128 MB memory analysis. The short version:
 *
 *   MemAvailable  the only memory column in free(1) that means anything.
 *                 "free" being small is normal and not a problem - the
 *                 page cache is using the rest and will give it back.
 *   CmaFree       not printed, and not a fallback either. It counts a page
 *                 the page cache has borrowed from the CMA area as used,
 *                 so it reads near zero while the hardware has taken
 *                 almost none of it. Measured here: CmaFree 15.9 MB left
 *                 while the bitmap said 50.6 MB of 52 MB really was free.
 *   debugfs cma/<area>/{count,used,maxchunk}
 *                 the honest ones. The bitmap is set only by cma_alloc()
 *                 and cleared only by cma_release(), so borrowing cannot
 *                 affect it. maxchunk matters as much as free: hardware
 *                 needs contiguous memory.
 *   alloc_pages_fail  the one that must stay at 0. A camera channel or an
 *                 encoder buffer that could not be allocated turns up
 *                 here first, before anything visible goes wrong. It is
 *                 printed only when it is non-zero.
 *   alloc_pages_success  deliberately not printed. It is cumulative since
 *                 boot and cma_release() never decrements it, so it is
 *                 "ever handed out", not "in use", and sitting next to a
 *                 live figure it reads like the latter.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SYSSTAT_H
#define SYSSTAT_H

#include <stdbool.h>

/*
 * Opaque only by convention - it lives on the caller's stack. Everything in
 * it is the previous sample, which is why there has to be one: CPU time is
 * a running total, so a percentage needs two readings and the wall clock
 * between them.
 */
struct sysstat {
	unsigned long long cpu_busy;	/* /proc/stat, all cores, ticks */
	unsigned long long cpu_total;
	double wall;			/* when the above were read */
	long page_kb;			/* for the CMA bitmap, counted in pages */
	unsigned ncpu;
	bool primed;			/* a previous sample exists */
};

/*
 * Takes the first sample, which has no percentage to report yet. The caller
 * decides when to call sysstat_report().
 */
void sysstat_init(struct sysstat *s);

/* One line: CPU and memory. elapsed is only for the timestamp column. */
void sysstat_report(struct sysstat *s, double elapsed);

#endif /* SYSSTAT_H */

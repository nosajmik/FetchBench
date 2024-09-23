#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "kern/tester.h"

void enable_pmu()
{
	// printf("enabling PMU registers\n");
	int device = open("/dev/tester", O_RDONLY);
	uint64_t val = 0xdeadbeef;

	ioctl(device, IOCTL_PMUSERENR_EL0, &val);
	// printf("PMUSERENR_EL0: %lx\n", val);

	val = 1 | (1 << 2);
	// printf("write PMUSERENR_EL0: %lx\n", val);
	ioctl(device, IOCTL_PMUSERENR_EL0_WRITE, &val);
	ioctl(device, IOCTL_PMUSERENR_EL0, &val);
	// printf("PMUSERENR_EL0: %lx\n", val);

	// enable
	asm volatile("isb sy\n\tmrs %0, PMCR_EL0\nisb sy\n\t" : "=r"(val));
	// printf("PMCR_EL0: %lx\n", val);
	val &= 0xffffffffffffff80;
	val |= 1;
	// printf("write PMCR_EL0: %lx\n", val);
	asm volatile("isb sy\n\tmsr PMCR_EL0, %0\nisb sy\n\t" ::"r"(val));
	asm volatile("isb sy\n\tmrs %0, PMCR_EL0\nisb sy\n\t" : "=r"(val));
	// printf("PMCR_EL0: %lx\n", val);

	// set events
	asm volatile("isb sy\n\tmrs %0, PMCNTENSET_EL0\nisb sy\n\t" : "=r"(val));
	// printf("PMCNTENSET_EL0: %lx\n", val);
	val = (1 << 31) | (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
	asm volatile("isb sy\n\tmsr PMCNTENSET_EL0, %0\nisb sy\n\t" ::"r"(val));
	asm volatile("isb sy\n\tmrs %0, PMCNTENSET_EL0\nisb sy\n\t" : "=r"(val));
	// printf("PMCNTENSET_EL0: %lx\n", val);

	// printf("setting events up\n");
	val = 0;
	asm volatile("isb sy\n\tmsr PMSELR_EL0, %0\nisb sy\n\t" ::"r"(val));
	val = 0x42; // L1D_CACHE_REFILL_RD
	asm volatile("isb sy\n\tmsr PMXEVTYPER_EL0, %0\nisb sy\n\t" ::"r"(val));
	val = 1;
	asm volatile("isb sy\n\tmsr PMSELR_EL0, %0\nisb sy\n\t" ::"r"(val));
	val = 0x52; // CACHE_RD_REFILL (L2)
	asm volatile("isb sy\n\tmsr PMXEVTYPER_EL0, %0\nisb sy\n\t" ::"r"(val));
	val = 2;
	asm volatile("isb sy\n\tmsr PMSELR_EL0, %0\nisb sy\n\t" ::"r"(val));
	val = 0x2A; // L3D_CACHE_REFILL
	asm volatile("isb sy\n\tmsr PMXEVTYPER_EL0, %0\nisb sy\n\t" ::"r"(val));
	val = 3;
	asm volatile("isb sy\n\tmsr PMSELR_EL0, %0\nisb sy\n\t" ::"r"(val));
	val = 0x44; // L1D_CACHE_REFILL_INNER
	asm volatile("isb sy\n\tmsr PMXEVTYPER_EL0, %0\nisb sy\n\t" ::"r"(val));
	val = 4;
	asm volatile("isb sy\n\tmsr PMSELR_EL0, %0\nisb sy\n\t" ::"r"(val));
	val = 0x45; // L1D_CACHE_REFILL_OUTER
	asm volatile("isb sy\n\tmsr PMXEVTYPER_EL0, %0\nisb sy\n\t" ::"r"(val));

	// reset
	asm volatile("isb sy\n\tmrs %0, PMCR_EL0\nisb sy\n\t" : "=r"(val));
	// printf("PMCR_EL0: %lx\n", val);
	val &= 0xffffffffffffff80;
	val |= 1 | (1 << 1); // reset
	// printf("write PMCR_EL0: %lx\n", val);
	asm volatile("isb sy\n\tmsr PMCR_EL0, %0\nisb sy\n\t" ::"r"(val));
	asm volatile("isb sy\n\tmrs %0, PMCR_EL0\nisb sy\n\t" : "=r"(val));
	// printf("PMCR_EL0: %lx\n", val);

	close(device);
}

extern inline __attribute__((always_inline))
uint64_t
read_l1d_refills()
{
	uint64_t val = 0;
	asm volatile("isb sy\n\tmsr PMSELR_EL0, %0\nisb sy\n\t" ::"r"(val));
	asm volatile("isb sy\n\tmrs %0, PMXEVCNTR_EL0\nisb sy\n\t" : "=r"(val));
	return val;
}

extern inline __attribute__((always_inline))
uint64_t
read_l2d_refills()
{
	uint64_t val = 1;
	asm volatile("isb sy\n\tmsr PMSELR_EL0, %0\nisb sy\n\t" ::"r"(val));
	asm volatile("isb sy\n\tmrs %0, PMXEVCNTR_EL0\nisb sy\n\t" : "=r"(val));
	return val;
}

extern inline __attribute__((always_inline))
uint64_t
read_l3d_refills()
{
	uint64_t val = 2;
	asm volatile("isb sy\n\tmsr PMSELR_EL0, %0\nisb sy\n\t" ::"r"(val));
	asm volatile("isb sy\n\tmrs %0, PMXEVCNTR_EL0\nisb sy\n\t" : "=r"(val));
	return val;
}

int open_kernel_device()
{
	return open("/dev/tester", O_RDONLY);
}

extern inline __attribute__((always_inline)) void clear_cache_l1(int device)
{
	ioctl(device, IOCTL_CLEAR_CACHE_L1, 0);
}

extern inline __attribute__((always_inline)) void clear_cache_l2(int device)
{
	ioctl(device, IOCTL_CLEAR_CACHE_L2, 0);
}

extern inline __attribute__((always_inline)) void clear_cache_l3(int device)
{
	ioctl(device, IOCTL_CLEAR_CACHE_L3, 0);
}

#endif
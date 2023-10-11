/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: timer reset and initialization
 * mtime is at 0x200bff8 and mtimecmp is at 0x2004000 in the memory map
 * see section 3.1.15 of references/riscv-privileged-v1.10.pdf
 * and section 9.1, 9.3 of references/sifive-fe310-v19p04.pdf
 */

#include "egos.h"

#define CLINT_BASE (0x14000000UL)
/* machine timer comparison register (lower 32-bit) */
#define MTIMECMPL0 (CLINT_BASE + 0x4000)
/* machine timer comparison register (higher 32-bit) */
#define MTIMECMPH0 (CLINT_BASE + 0x4004) 

static inline uint64_t mtime_get() {
    uint64_t cnt;
    asm volatile (
        "csrr %0, time\n"
        : "=r"(cnt)
        :
        : "memory"
    );

    return cnt;
}

static void mtimecmp_set(uint64_t time) {
    *((volatile uint32_t *)(MTIMECMPH0)) = 0xFFFFFFFF;
    *((volatile uint32_t *)(MTIMECMPL0)) = (time & 0xFFFFFFFF);
    *((volatile uint32_t *)(MTIMECMPH0)) = (time >> 32);
}

static uint64_t QUANTUM;
int timer_reset() {
    uint64_t now = mtime_get();
    mtimecmp_set(now + QUANTUM);
}

void timer_init()  {
    earth->timer_reset = timer_reset;
    QUANTUM = 24 * 1000 * 250; /* 250ms. Timer runs at 24MHz */
    mtimecmp_set(0xFFFFFFFFFFFFFFFFUL);
}

/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: timer reset and initialization
 * See Chapter 9 of references/XuanTie-OpenC906-UserManual.pdf
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
    *((uint32_t *)(MTIMECMPH0)) = 0xFFFFFFFU;
    *((uint32_t *)(MTIMECMPL0)) = (time & 0xFFFFFFFF);
    *((uint32_t *)(MTIMECMPH0)) = (time >> 32);
}

static uint64_t QUANTUM;
int timer_reset() {
    /**
     * With mstatus.MPRV set, M Mode is unable to modify the MTIMECMP*
     * registers because loads and stores execute as if the privilege mode was
     * S-mode. One interesting thing to think about here is where the stack
     * pointer actually points to after jumping to mtimecmp_set.
     * 
     * Another solution to this problem is just to use the D1's extended S-mode
     * timer compare registers, STIMECMP*, so S-mode can request timer compare
     * interrupts.
    */
    asm("csrc mstatus, %0"::"r"(1 << 17));
    mtimecmp_set(mtime_get() + QUANTUM);
    asm("csrs mstatus, %0"::"r"(1 << 17));
}

void timer_init()  {
    earth->timer_reset = timer_reset;
    QUANTUM = 24 * 1000 * 250; /* 250ms. Timer runs at 24MHz */
    mtimecmp_set(0xFFFFFFFFFFFFFFFFUL);
}

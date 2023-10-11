/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: boot loader
 * i.e., the first instructions executed by the CPU when boot up
 */

#define GRASS_STACK_TOP 0x41EFFDF0

    .section .text.enter
    .global earth_enter, trap_entry_vm
earth_enter:
    /* Disable machine interrupt */
    li t0, 0x8
    csrc mstatus, t0

    /* Call main() of earth.c */
    li sp, 0x400FFDF0   /* GRASS_STACK_TOP */
    call main

# trap_entry_vm:
#     csrw mscratch, t0

#     /* Set mstatus.MPRV in order to use virtual addresses */
#     /* If mstatus.MPP is user mode, set it to supervisor mode */
#     li t0, 0x20800
#     csrs mstatus, t0

#     csrr t0, mscratch

#     /* Jump to trap_entry() without modifying any registers */
#     j trap_entry

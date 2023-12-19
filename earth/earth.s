/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: boot loader
 * i.e., the first instructions executed by the CPU when boot up
 */

.macro CTX_SAVE
    addi sp, sp, -27*8

    sd ra, 0*8(sp)
    sd t0, 1*8(sp)
    sd t1, 2*8(sp)
    sd t2, 3*8(sp)
    sd s0, 4*8(sp)
    sd s1, 5*8(sp)
    sd a0, 6*8(sp)
    sd a1, 7*8(sp)
    sd a2, 8*8(sp)
    sd a3, 9*8(sp)
    sd a4, 10*8(sp)
    sd a5, 11*8(sp)
    sd a6, 12*8(sp)
    sd a7, 13*8(sp)
    sd s2, 14*8(sp)
    sd s3, 15*8(sp)
    sd s4, 16*8(sp)
    sd s5, 17*8(sp)
    sd s6, 18*8(sp)
    sd s7, 19*8(sp)
    sd s8, 20*8(sp)
    sd s9, 21*8(sp)
    sd s10, 22*8(sp)
    sd s11, 23*8(sp)
    sd t3, 24*8(sp)
    sd t4, 25*8(sp)
    sd t5, 26*8(sp)
    sd t6, 27*8(sp)
.endm

.macro CTX_RESTORE
    ld ra, 0*8(sp)
    ld t0, 1*8(sp)
    ld t1, 2*8(sp)
    ld t2, 3*8(sp)
    ld s0, 4*8(sp)
    ld s1, 5*8(sp)
    ld a0, 6*8(sp)
    ld a1, 7*8(sp)
    ld a2, 8*8(sp)
    ld a3, 9*8(sp)
    ld a4, 10*8(sp)
    ld a5, 11*8(sp)
    ld a6, 12*8(sp)
    ld a7, 13*8(sp)
    ld s2, 14*8(sp)
    ld s3, 15*8(sp)
    ld s4, 16*8(sp)
    ld s5, 17*8(sp)
    ld s6, 18*8(sp)
    ld s7, 19*8(sp)
    ld s8, 20*8(sp)
    ld s9, 21*8(sp)
    ld s10, 22*8(sp)
    ld s11, 23*8(sp)
    ld t3, 24*8(sp)
    ld t4, 25*8(sp)
    ld t5, 26*8(sp)
    ld t6, 27*8(sp)

    addi sp, sp, 27*8
.endm

    .section .text.enter
    .global earth_enter, trap_entry_vm
earth_enter:
    /* Disable machine interrupt */
    li t0, 0x8
    csrc mstatus, t0

    /* Call main() of earth.c */
    li sp, 0x400FEFF0   /* GRASS_STACK_TOP */
    call main


/**
 * This has been changed from a wrapper function around trap_entry to having
 * its own standalone definition so mstatus.MPRV can be cleared before mret.
 * The Allwinner D1 C906 core is compliant with the 'Privileged Architecture,
 * Version 1.10.' RISC-V spec, and clearing mstatus.MPRV is only required in
 * Version 20211203 of that spec.
 */
trap_entry_vm:
    csrw mscratch, t0
    
    /* Set mstatus.MPRV in order to use virtual addresses */
    /* If mstatus.MPP is user mode, set it to supervisor mode */
    /* Use tp to remember if was was U-mode or S-mode that trapped */
    li t0, 0x20800
    csrrs tp, mstatus, t0
    li t0, 0x1800
    and tp, tp, t0

    csrr t0, mscratch

    /* Save process registers on its stack */
    CTX_SAVE

    /* Jump to handle_trap() without modifying any registers */
    call handle_trap

    /* Restore process registers to its stack */
    CTX_RESTORE

    csrw mscratch, t0

    /* clear mstatus.MPP and mstatus.MPRV */
    li t0, 0x21800
    csrc mstatus, t0
    /* Restore MPP to U/S-mode saved in tp */
    csrs mstatus, tp 
    
    csrr t0, mscratch

    mret

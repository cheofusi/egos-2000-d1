/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: _enter of grass, context start and context switch
 */

.macro CTX_SAVE
    addi sp,sp,-112
    sd s0,8(sp)     /* Save callee-saved registers */
    sd s1,16(sp)
    sd s2,24(sp)
    sd s3,32(sp)
    sd s4,40(sp)
    sd s5,48(sp)
    sd s6,56(sp)
    sd s7,64(sp)
    sd s8,72(sp)
    sd s9,80(sp)
    sd s10,88(sp)
    sd s11,96(sp)
    sd ra,104(sp)   /* Save return address */
.endm

.macro CTX_RESTORE
    ld s0,8(sp)     /* Restore callee-saved registers */
    ld s1,16(sp)
    ld s2,24(sp)
    ld s3,32(sp)
    ld s4,40(sp)
    ld s5,48(sp)
    ld s6,56(sp)
    ld s7,64(sp)
    ld s8,72(sp)
    ld s9,80(sp)
    ld s10,88(sp)
    ld s11,96(sp)
    ld ra,104(sp)   /* Restore return address */
    addi sp,sp,112
.endm

    .section .text
    .global grass_enter, ctx_start, ctx_switch
grass_enter:
    li sp,0x400FEFF0    /* GRASS_STACK_TOP */
    call main

ctx_start:
    CTX_SAVE
    sd sp,0(a0)       /* Save the current stack pointer */
    mv sp,a1          /* Switch the stack */
    call ctx_entry    /* Call ctx_entry() */

ctx_switch:
    CTX_SAVE
    sd sp,0(a0)       /* Save the current stack pointer */
    mv sp,a1          /* Switch the stack */
    CTX_RESTORE
    ret

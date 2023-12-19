/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize the stack pointer and call application main()
 */
    .section .text
    .global app_enter
app_enter:
    ld a0, 0(a0) /* a0 holds APPS_ARG, the address of integer argc */
    li sp, 0x400FCFF0    /* APP_STACK_TOP */
    call main
    call exit

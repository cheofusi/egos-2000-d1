/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: Kernel ≈ 3 handlers
 *     proc_yield() handles timer interrupt for process scheduling
 *     excp_entry() handles faults such as unauthorized memory access
 *     proc_syscall() handles system calls for inter-process communication
 */


#include "egos.h"
#include "process.h"
#include "syscall.h"
#include <string.h>

static void proc_yield();
static void proc_syscall();
static void (*kernel_entry)();

int proc_curr_idx;
struct process proc_set[MAX_NPROCESS];

#define EXCP_ID_ECALL_U    8
#define EXCP_ID_ECALL_S    9
#define EXCP_ID_ECALL_M    11

void excp_entry(int id) {
    uintptr_t mepc, mtval;

    /* An ecall exception is a system call */
    if (id == EXCP_ID_ECALL_U || id == EXCP_ID_ECALL_S || EXCP_ID_ECALL_M)
        kernel_entry = proc_syscall;

    else {
        if (curr_pid >= GPID_USER_START) {
            /**
             * non-ecall exception from user app, kill it and return control to
             * sys_shell
             */
            INFO("process %d killed due to exception %d", curr_pid, id);
            proc_free(curr_pid);

            proc_set_runnable(GPID_SHELL);
            kernel_entry = proc_yield;
        }

        else {
            asm("csrr %0, mepc" : "=r"(mepc));
            asm("csrr %0, mtval" : "=r"(mtval));
            INFO("mepc: 0x%08lx, mtval: 0x%08lx", mepc, mtval);

            FATAL("excp_entry: kernel got exception %d", id);
        }
    }

    /* increment mepc so on mret we don't trigger the same exception */
    asm("csrr %0, mepc" : "=r"(mepc));
    asm("csrw mepc, %0" ::"r"(mepc + 4));

    ctx_start(&proc_set[proc_curr_idx].sp, (void*)GRASS_STACK_TOP);
}

#define INTR_ID_SOFT       3
#define INTR_ID_TIMER      7

int proc_curr_idx;
struct process proc_set[MAX_NPROCESS];

void intr_entry(int id) {
    if (id == INTR_ID_TIMER && curr_pid < GPID_SHELL) {
        /* Do not interrupt kernel processes since IO can be stateful */
        earth->timer_reset();
        return;
    }

    else if (earth->tty_recv_intr() && curr_pid >= GPID_USER_START) {
        /* User process killed by CTRL+C. Return control to sys_shell */
        
        INFO("process %d killed by interrupt", curr_pid);
        proc_free(curr_pid);
        
        proc_set_runnable(GPID_SHELL);
        kernel_entry = proc_yield;
    }

    else if (id == INTR_ID_SOFT)
        kernel_entry = proc_syscall;

    else if (id == INTR_ID_TIMER)
        kernel_entry = proc_yield;
    
    else
        FATAL("intr_entry: got unknown interrupt %d", id);

    /* Switch to the kernel stack */
    ctx_start(&proc_set[proc_curr_idx].sp, (void*)GRASS_STACK_TOP);
}

void ctx_entry() {
    /* Now on the kernel stack */
    uintptr_t mepc, tmp;
    asm("csrr %0, mepc" : "=r"(mepc));
    proc_set[proc_curr_idx].mepc = (void*) mepc;

    /* kernel_entry() is either proc_yield() or proc_syscall() */
    kernel_entry();

    /* Switch back to the user application stack */
    mepc = (uintptr_t)proc_set[proc_curr_idx].mepc;
    asm("csrw mepc, %0" ::"r"(mepc));
    ctx_switch((void**)&tmp, proc_set[proc_curr_idx].sp);
}

static void proc_yield() {
    /* Find the next runnable process */
    int next_idx = -1;
    for (int i = 1; i <= MAX_NPROCESS; i++) {
        int s = proc_set[(proc_curr_idx + i) % MAX_NPROCESS].status;
        if (s == PROC_READY || s == PROC_RUNNING || s == PROC_RUNNABLE) {
            next_idx = (proc_curr_idx + i) % MAX_NPROCESS;
            break;
        }
    }

    if (next_idx == -1) FATAL("proc_yield: no runnable process");
    if (curr_status == PROC_RUNNING) proc_set_runnable(curr_pid);

    /* Switch to the next runnable process and reset timer */
    proc_curr_idx = next_idx;
    earth->mmu_switch(curr_pid);
    earth->timer_reset();

    /* Call the entry point for newly created process */
    if (curr_status == PROC_READY) {
        proc_set_running(curr_pid);
        /* Prepare argc and argv */
        asm("mv a0, %0" ::"r"(APPS_ARG));
        asm("mv a1, %0" ::"r"(APPS_ARG + 4));
        /* Enter application code entry using mret */
        asm("csrw mepc, %0" ::"r"(APPS_ENTRY));

        /* Modify mstatus.MPP to enter machine or user mode during mret
         * depending on whether the new process is a grass server or a user app
         */
        uintptr_t mstatus;
        asm("csrr %0, mstatus" : "=r"(mstatus));
        int pp = (curr_pid < GPID_USER_START);
        asm("csrs mstatus, %0" ::"r"((mstatus & ~(3 << 11)) | (pp << 11)));
        
        /* clear mstatus.MPRV because the D1 doesn't */
        asm("csrc mstatus, %0"::"r"(1 << 17));
        
        asm("mret");
    }

    proc_set_running(curr_pid);
}

static void proc_send(struct syscall *sc) {
    sc->msg.sender = curr_pid;
    int receiver = sc->msg.receiver;

    for (int i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].pid == receiver) {
            /* Find the receiver */
            if (proc_set[i].status != PROC_WAIT_TO_RECV) {
                curr_status = PROC_WAIT_TO_SEND;
                proc_set[proc_curr_idx].receiver_pid = receiver;
            } else {
                /* Copy message from sender to kernel stack */
                struct sys_msg tmp;
                earth->mmu_switch(curr_pid);
                memcpy(&tmp, &sc->msg, sizeof(tmp));

                /* Copy message from kernel stack to receiver */
                earth->mmu_switch(receiver);
                memcpy(&sc->msg, &tmp, sizeof(tmp));

                /* Set receiver process as runnable */
                proc_set_runnable(receiver);
            }
            proc_yield();
            return;
        }

    sc->retval = -1;
}

static void proc_recv(struct syscall *sc) {
    int sender = -1;
    for (int i = 0; i < MAX_NPROCESS; i++)
        if (proc_set[i].status == PROC_WAIT_TO_SEND &&
            proc_set[i].receiver_pid == curr_pid)
            sender = proc_set[i].pid;

    if (sender == -1) {
        curr_status = PROC_WAIT_TO_RECV;
    } else {
        /* Copy message from sender to kernel stack */
        struct sys_msg tmp;
        earth->mmu_switch(sender);
        memcpy(&tmp, &sc->msg, sizeof(tmp));

        /* Copy message from kernel stack to receiver */
        earth->mmu_switch(curr_pid);
        memcpy(&sc->msg, &tmp, sizeof(tmp));

        /* Set sender process as runnable */
        proc_set_runnable(sender);
    }

    proc_yield();
}

static void proc_syscall() {
    struct syscall *sc = (struct syscall*)SYSCALL_ARG;

    int type = sc->type;
    sc->retval = 0;
    sc->type = SYS_UNUSED;

    switch (type) {
    case SYS_RECV:
        proc_recv(sc);
        break;
    case SYS_SEND:
        proc_send(sc);
        break;
    default:
        FATAL("proc_syscall: got unknown syscall type=%d", type);
    }
}

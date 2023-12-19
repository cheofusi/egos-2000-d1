/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: Initialize the bss and data segments;
 * Initialize dev_tty, dev_disk, cpu_intr and cpu_mmu;
 * Load the grass layer binary from disk and run it.
 */

#include "elf.h"
#include "disk.h"
#include "egos.h"
#include <string.h>

void tty_init();
void disk_init();
void mmu_init();
void timer_init();
void intr_init();

struct grass *grass = (void*)GRASS_INTERFACE;
struct earth *earth = (void*)EARTH_INTERFACE;
extern char bss_start, bss_end;

static void earth_init() {
    earth->platform = D1;

    tty_init();
    CRITICAL("--- Booting on the D1 ---");

    disk_init();
    SUCCESS("Finished initializing the tty and disk devices");

    mmu_init();
    timer_init();
    intr_init();
    SUCCESS("Finished initializing the CPU MMU, timer and interrupts");
}

static int grass_read(int block_no, char* dst) {
    return earth->disk_read(GRASS_EXEC_START + block_no, 1, dst);
}

int main() {
    /* Prepare the bss and data memory regions */
    memset(&bss_start, 0, (&bss_end - &bss_start));

    /* Initialize the earth layer */
    earth_init();

    /* Load and enter the grass layer */
    elf_load(0, grass_read, 0, 0);
    if (earth->translation == PAGE_TABLE) {
        /**
         * Allow reading time csr in S-mode without triggering illegal instruction exception
         */
        uintptr_t mcounteren;
        asm("csrr %0, mcounteren" : "=r"(mcounteren));
        asm("csrw mcounteren, %0" :: "r"(mcounteren | (1 << 1)));

        /* Enter the grass layer in supervisor mode for PAGE_TABLE translation */
        uintptr_t mstatus;
        asm("csrr %0, mstatus" : "=r"(mstatus));
        asm("csrw mstatus, %0" ::"r"((mstatus & ~(3 << 11)) | (1 << 11) | (1 << 18)));
    }
    asm("csrw mepc, %0" ::"r"(GRASS_ENTRY));
    asm("mret");
}

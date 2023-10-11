/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: memory management unit (MMU)
 * implementation of 2 translation mechanisms: page table and software TLB
 */

#include "egos.h"
#include "disk.h"
#include "servers.h"
#include <string.h>

/* Interface of the paging device, see earth/dev_page.c */
void  paging_init();
int   paging_invalidate_cache(int frame_id);
int   paging_write(int frame_id, uintptr_t page_no);
char* paging_read(int frame_id, int alloc_only);

/* Allocation and free of physical frames */
#define NFRAMES 256
struct frame_mapping {
    int use;     /* Is the frame allocated? */
    int pid;     /* Which process owns the frame? */
    uintptr_t page_no; /* Which virtual page is the frame mapped to? */
} table[NFRAMES];

int mmu_alloc(int* frame_id, char** cached_addr) {
    for (int i = 0; i < NFRAMES; i++)
        if (!table[i].use) {
            *frame_id = i;
            *cached_addr = paging_read(i, 1);
            table[i].use = 1;
            return 0;
        }
    FATAL("mmu_alloc: no more available frames");
}

int mmu_free(int pid) {
    for (int i = 0; i < NFRAMES; i++)
        if (table[i].use && table[i].pid == pid) {
            paging_invalidate_cache(i);
            memset(&table[i], 0, sizeof(struct frame_mapping));
        }
}

/* Software TLB Translation */
int soft_tlb_map(int pid, uintptr_t page_no, int frame_id) {
    table[frame_id].pid = pid;
    table[frame_id].page_no = page_no;
}

int soft_tlb_switch(int pid) {
    static int curr_vm_pid = -1;
    if (pid == curr_vm_pid) return 0;

    /* Unmap curr_vm_pid from the user address space */
    for (int i = 0; i < NFRAMES; i++)
        if (table[i].use && table[i].pid == curr_vm_pid)
            paging_write(i, table[i].page_no);

    /* Map pid to the user address space */
    for (int i = 0; i < NFRAMES; i++)
        if (table[i].use && table[i].pid == pid)
            memcpy((void*)(table[i].page_no << 12), paging_read(i, 0), PAGE_SIZE);

    curr_vm_pid = pid;

    /**
     * RISC-V does not guarantee that stores to instruction memory will be made 
     * visible to instruction fetches on a RISC-V hart until that hart executes
     * a FENCE.I instruction. The D1 seems to make use of this liberty and hence
     * REQUIRES a FENCE.I instruction for subsequent instruction fetches after the 
     * above switch to work.
    */
     __sync_synchronize();
    asm volatile ("fence.i");
}

/* Page Table Translation
 *
 * The code below creates an identity mapping using RISC-V Sv32;
 * Read section4.3 of RISC-V manual (references/riscv-privileged-v1.10.pdf)
 *
 * mmu_map() and mmu_switch() using page tables is given to students
 * as a course project. After this project, every process should have
 * its own set of page tables. mmu_map() will modify entries in these
 * tables and mmu_switch() will modify satp (page table base register)
 */

void setup_identity_region(int pid, unsigned int addr, int npages, int flag) {}

void pagetable_identity_mapping(int pid) {}

int page_table_map(int pid, int page_no, int frame_id) {
    if (pid >= 32) FATAL("page_table_map: pid too large");

    /* Student's code goes here (page table translation). */

    /* Remove the following line of code and, instead,
     * (1) if page tables for pid do not exist, build the tables;
     * (2) if page tables for pid exist, update entries of the tables
     *
     * Feel free to call or modify the two helper functions:
     * pagetable_identity_mapping() and setup_identity_region()
     */
    soft_tlb_map(pid, page_no, frame_id);

    /* Student's code ends here. */
}

int page_table_switch(int pid) {
    /* Student's code goes here (page table translation). */

    /* Remove the following line of code and, instead,
     * modify the page table base register (satp) similar to
     * the code in mmu_init(); Remember to use the pid argument
     */
    soft_tlb_switch(pid);

    /* Student's code ends here. */
}

/* MMU Initialization */
void mmu_init() {
    /* Initialize the paging device */
    paging_init();

    /* Initialize MMU interface functions */
    earth->mmu_free = mmu_free;
    earth->mmu_alloc = mmu_alloc;

    /* Setup a PMP region for the whole 4GB address space */

    /* Student's code goes here (PMP memory protection). */

    /* Student's code ends here. */

    earth->translation = SOFT_TLB;
    earth->mmu_map = soft_tlb_map;
    earth->mmu_switch = soft_tlb_switch;
}

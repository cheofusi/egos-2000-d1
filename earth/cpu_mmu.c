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
#include <stdbool.h>

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

/**
 * The D1 C906 core uses Sv39 paging, which prescribes a 44-bit PPN, but the C906
 * uses a 28-bit PPN, with a 10-bit PPN[2] field.
*/
#define PTE_PPN_OFFSET (10)
#define PTE_PPN_MASK (UINT64_C(0xFFFFFFFFFFF) << PTE_PPN_OFFSET)
#define PTE_GET_PA(pte) ((pte & PTE_PPN_MASK) << 2)
#define PA_TO_PTE(pa) (((uintptr_t)pa >> 2) & PTE_PPN_MASK)
#define VA_GET_VPN(va, level) (((uintptr_t)va >> (12 + (level) * 9)) & 0x1FF) 

#define PTE_V (UINT64_C(1) << 0) 
#define OS_XWRV   UINT64_C(0xF)
#define USER_XWRV UINT64_C(0x1F)
#define PTE_A (UINT64_C(1) << 6)
#define PTE_D (UINT64_C(1) << 7) 

#define PA_PPN_OFFSET (12)
#define SATP_SV39 (UINT64_C(8) << 60)
#define MAKE_SATP(pt) (SATP_SV39 | (((uintptr_t)pt) >> PA_PPN_OFFSET))

/* 32 is a number large enough for demo purpose */
static unsigned long *pid_to_pagetable_base[32];

/**
 * Walks (and allocates if not yet allocated) the page tables in `pid`'s page
 * table tree required for mapping `page_addr`, returning the leaf page table.
 */
unsigned long *walk_page_tables(int pid, uintptr_t page_addr)
{
    static unsigned int frame_id;
    static unsigned long *root, *l1_pt, *leaf_pt;

    if (page_addr % PAGE_SIZE)
        FATAL("Address %lu not page aligned", page_addr);

    if (!pid_to_pagetable_base[pid]) {
        /* root page table has not been allocated */
        earth->mmu_alloc(&frame_id, (char **)&root);
        table[frame_id].pid = pid;
        memset(root, 0, PAGE_SIZE);
        pid_to_pagetable_base[pid] = root;
    }

    root = pid_to_pagetable_base[pid];

    /* In level 2 (root) page table... */
    int vpn2 = VA_GET_VPN(page_addr, 2);
    if (root[vpn2] & PTE_V) {
        l1_pt = (void *)PTE_GET_PA(root[vpn2]);
    } else {
        /* 1st level page table has not been allocated */
        earth->mmu_alloc(&frame_id, (char **)&l1_pt);
        table[frame_id].pid = pid;
        memset(l1_pt, 0, PAGE_SIZE);
        root[vpn2] = PA_TO_PTE(l1_pt) | PTE_V;
    }

    /* In level 1 page table... */
    int vpn1 = VA_GET_VPN(page_addr, 1);
    if (l1_pt[vpn1] & PTE_V) {
        leaf_pt = (void *)PTE_GET_PA(l1_pt[vpn1]);
    } else {
        /* leaf page table has not been allocated */
        earth->mmu_alloc(&frame_id, (char**)&leaf_pt);
        table[frame_id].pid = pid;
        memset(leaf_pt, 0, PAGE_SIZE);
        l1_pt[vpn1] = PA_TO_PTE(leaf_pt) | PTE_V;
    }

    return leaf_pt;
}

/**
 * Creates and identity map for `page_addr`
 */
void setup_identity_region(int pid, uintptr_t page_addr, int npages, int flag) {
    unsigned long *leaf_pt = walk_page_tables(pid, page_addr);

    /* Setup the entries in the leaf page table */
    int vpn0 = VA_GET_VPN(page_addr, 0);
    for (int i = 0; i < npages; i++)
    {
        if ((vpn0 + i) > 511)
            FATAL("Leaf page table overrun!!!");
        leaf_pt[vpn0 + i] = PA_TO_PTE((page_addr + i * PAGE_SIZE)) 
                        | PTE_A | PTE_D | flag;
    }
}

void pagetable_identity_mapping(int pid) {
    setup_identity_region(pid, 0x02500000, 1, OS_XWRV);  /* UART0 */
    setup_identity_region(pid, 0x04020000, 3, OS_XWRV);  /* SMHC0 */

    if (pid == 0) {
        /* direct map entire memory for process 0 i.e grass */
        setup_identity_region(pid, MEM_START, PAGE_CNT(MEM_SIZE), OS_XWRV);
    }

    else {
        setup_identity_region(pid, EARTH_ENTRY, PAGE_CNT(EARTH_SIZE), OS_XWRV);
        setup_identity_region(pid, GRASS_ENTRY, PAGE_CNT(GRASS_SIZE), OS_XWRV);
        setup_identity_region(pid, GRASS_STACK_BOTTOM, PAGE_CNT(GRASS_STACK_SIZE),
                             OS_XWRV);
        setup_identity_region(pid, GRASS_INTERFACE, 1, OS_XWRV);

        /**
         * Only GPID_PROCESS should direct map the FRAME_CACHE region as it is
         * solely responsible for loading other processes.
         */
        if (pid == GPID_PROCESS)
            setup_identity_region(pid, FRAME_CACHE_START, PAGE_CNT(FRAME_CACHE_SIZE),
                                OS_XWRV);
    }

}

/**
 * Mags for `page_addr` (page_no << 12) to frame identified by
 * 'pid_frame_id'
 */
int page_table_map(int pid, uintptr_t page_no, int pid_frame_id) {
    if (pid >= 32) FATAL("page_table_map: pid too large");

    uintptr_t page_addr = page_no << 12;

    /**
     * Each process (pid != 0) direct maps the earth, grass, grass stack,
     * earth interface, and grass interface memory regions with the U bit
     * for each associated PTE unset.
     */
    if (!pid_to_pagetable_base[pid])
        pagetable_identity_mapping(pid);

    unsigned long *leaf_pt = walk_page_tables(pid, page_addr);
    /* setup entry in leaf page table */
    int vpn0 = VA_GET_VPN(page_addr, 0);
    /// TODO: allow perms arg in eath->mmu_map ??, so e.g a page for a process'
    /// stack should not be executable.
    leaf_pt[vpn0] = PA_TO_PTE(paging_read(pid_frame_id, 1))
                | PTE_A | PTE_D | USER_XWRV;
}

int page_table_switch(int pid) {
    static int curr_vm_pid = -1;
    if (pid == curr_vm_pid) return 0;

    asm("csrw satp, %0" ::"r"(MAKE_SATP(pid_to_pagetable_base[pid])));
    asm volatile("sfence.vma zero, zero":::"memory");

    curr_vm_pid = pid;
}

/* MMU Initialization */
void mmu_init() {
    /* Initialize the paging device */
    paging_init();

    /* Initialize MMU interface functions */
    earth->mmu_free = mmu_free;
    earth->mmu_alloc = mmu_alloc;

    /* Setup a PMP region for the first 4GB of address space */
    /// TODO: make this region more precise
    asm volatile("csrw pmpaddr0, %0" : : "r" (0xffffffffULL >> 2));
    asm volatile("csrw pmpcfg0, %0" : : "r" (0x0FULL));

    earth->translation = SOFT_TLB;
    earth->mmu_map = soft_tlb_map;
    earth->mmu_switch = soft_tlb_switch;

    /* Choose memory translation mechanism */
    CRITICAL("Choose a memory translation mechanism:");
    printf("Enter 0: page tables\r\nEnter 1: software TLB\r\n");

    char buf[2];
    for (buf[0] = 0; buf[0] != '0' && buf[0] != '1'; earth->tty_read(buf, 2));
    earth->translation = (buf[0] == '0') ? PAGE_TABLE : SOFT_TLB;
    INFO("%s translation is chosen", earth->translation == PAGE_TABLE ? "Page table" : "Software");

    if (earth->translation == PAGE_TABLE) {
        /* Setup an identity mapping using page tables */
        pagetable_identity_mapping(0);
        asm("csrw satp, %0" ::"r"(MAKE_SATP(pid_to_pagetable_base[0])));

        earth->mmu_map = page_table_map;
        earth->mmu_switch = page_table_switch;
    }
}

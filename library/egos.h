#pragma once
#include <stdint.h>

struct earth {
    /* CPU interface */
    int (*timer_reset)();

    int (*intr_register)(void (*handler)(int));
    int (*excp_register)(void (*handler)(int));

    int (*mmu_alloc)(int* frame_no, char** cached_addr);
    int (*mmu_free)(int pid);
    int (*mmu_map)(int pid, uintptr_t page_no, int frame_no);
    int (*mmu_switch)(int pid);

    /* Devices interface */
    int (*disk_read)(int block_no, int nblocks, char* dst);
    int (*disk_write)(int block_no, int nblocks, char* src);

    int (*tty_recv_intr)();
    int (*tty_read)(char* buf, int len);
    int (*tty_write)(char* buf, int len);

    int (*tty_printf)(const char *format, ...);
    int (*tty_info)(const char *format, ...);
    int (*tty_fatal)(const char *format, ...);
    int (*tty_success)(const char *format, ...);
    int (*tty_critical)(const char *format, ...);

    /* Some information about earth layer configuration */
    enum { D1 } platform;
    enum { PAGE_TABLE, SOFT_TLB } translation;
};

struct grass {
    /* Shell environment variables */
    int workdir_ino;
    char workdir[128];

    /* Process control interface */
    int  (*proc_alloc)();
    void (*proc_free)(int pid);
    void (*proc_set_ready)(int pid);

    /* System call interface */
    void (*sys_exit)(int status);
    int  (*sys_send)(int pid, char* msg, int size);
    int  (*sys_recv)(int* pid, char* buf, int size);
};

extern struct earth *earth;
extern struct grass *grass;

/**
 * D1 provides 0x4000 0000---0xBFFF FFFF of DRAM space. The Lichee RV only uses
 * the bottom 512M, meaning 0x4000 0000---0x6000 0000. We'll stick using to the 
 * first 2M for now.
 * 
 */

/* Memory layout */
#define MEM_START          0x40000000   /* D1 DRAM location */
#define MEM_SIZE           0x200000    /* only use 2M */
#define MEM_END            (MEM_START + MEM_SIZE)

#define PAGE_SIZE          4096
#define FRAME_CACHE_SIZE   1024 * 1024   /* 1M frame cache */
#define FRAME_CACHE_START  (MEM_END - FRAME_CACHE_SIZE)
#define FRAME_CACHE_END    MEM_END

#define EARTH_INTERFACE    (FRAME_CACHE_START - 256) /* 256 bytes for struct earth */
#define GRASS_INTERFACE    (EARTH_INTERFACE - 256) /* 256 bytes fo struct grass */

#define GRASS_STACK_TOP    (GRASS_INTERFACE - 16)
#define GRASS_STACK_SIZE   8 * 1024     /* 8K earth/grass stack */

#define APPS_STACK_TOP     ((GRASS_STACK_TOP - GRASS_STACK_SIZE) & 0xFFFFF000)
#define APPS_STACK_SIZE    6 * 1024     /* 6K app stack */
#define SYSCALL_ARG        (APPS_STACK_TOP - APPS_STACK_SIZE - 1024) /* 1K system call args */
#define APPS_ARG           (SYSCALL_ARG - 1024) /* 1K app main() argc, argv. Should be a 
                                                    page boundary 
                                                */

#define EARTH_ENTRY        MEM_START
#define EARTH_SIZE         (128 + 16) * 1024    /* 128K + 16K */

#define GRASS_ENTRY        (EARTH_ENTRY + EARTH_SIZE)
#define GRASS_SIZE         (8 + 4) * 1024   /* 8K + 4K */

#define APPS_ENTRY         (GRASS_ENTRY + GRASS_SIZE) /* should be a page boundary */
#define APPS_SIZE          16 * 1024    /* 16K virtual address range. So the frame cache supports
                                           virtualization at most 1M /16k = 64 app processes. 
                                         */ 


#ifndef LIBC_STDIO
/* Only earth/dev_tty.c uses LIBC_STDIO and does not need these macros */
#define printf             earth->tty_printf
#define INFO               earth->tty_info
#define FATAL              earth->tty_fatal
#define SUCCESS            earth->tty_success
#define CRITICAL           earth->tty_critical
#endif

/* Memory-mapped I/O register access macros */
#define ACCESS(x) (*(__typeof__(*x) volatile *)(x))
#define REGW(base, offset) (ACCESS((unsigned int*)(base + offset)))
#define REGB(base, offset) (ACCESS((unsigned char*)(base + offset)))

/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize dev_tty, dev_disk, cpu_intr and cpu_mmu;
 * load the grass layer binary from disk and run it
 */


#include "egos.h"
#include "earth.h"

int earth_init();
void grass_load();
static struct earth earth;

int main() {
    INFO("-----------------------------------");
    INFO("Start to initialize the earth layer");

    if (earth_init())
        return -1;

    if (sizeof(earth) > 0x40)
        FATAL("Size of earth interface (%d bytes) exceeds 64 bytes", sizeof(earth));
    INFO("Put earth interface (%d bytes) at 0x%.8x", sizeof(earth), EARTH_ADDR);
    memcpy((void*)EARTH_ADDR, &earth, sizeof(earth));

    INFO("Start to load the grass layer");
    grass_load();

    return 0;
}

int earth_init() {
    /* Initialize TTY */
    if (tty_init()) {
        ERROR("Failed at initializing the tty device");
        return -1;
    }
    earth.tty_read = tty_read;
    earth.tty_write = tty_write;
    SUCCESS("Finished initializing the tty device");
    
    /* Initialize disk */
    if (disk_init()) {
        ERROR("Failed at initializing the disk device");
        return -1;
    }
    earth.disk_read = disk_read;
    earth.disk_write = disk_write;
    earth.disk_busy = disk_busy;
    SUCCESS("Finished initializing the disk device");

    /* Initialize CPU interrupt */
    if (intr_init()) {
        ERROR("Failed at initializing the CPU interrupts");
        return -1;
    }
    earth.intr_enable = intr_enable;
    earth.intr_disable = intr_disable;
    earth.intr_register = intr_register;
    SUCCESS("Finished initializing the CPU interrupts");

    /* Initialize CPU memory management unit */
    if (mmu_init()) {
        ERROR("Failed at initializing the CPU interrupts");
        return -1;
    }
    earth.mmu_alloc = mmu_alloc;
    earth.mmu_map = mmu_map;
    earth.mmu_switch = mmu_switch;
    SUCCESS("Finished initializing the CPU memory management unit");

    /* Initialize the logging functions */
    earth.log.log_info = INFO;
    earth.log.log_highlight = HIGHLIGHT;
    earth.log.log_success = SUCCESS;
    earth.log.log_error = ERROR;
    earth.log.log_fatal = FATAL;

    return 0;
}

static int grass_read(int block_no, char* dst) {
    return earth.disk_read(GRASS_EXEC_START + block_no, 1, dst);
}

void grass_load() {
    struct block_store bs;
    bs.read = grass_read;
    elf_load(0, &bs, &earth);

    /* call the grass kernel entry and never return */
    void (*grass_entry)() = (void*)GRASS_BASE;
    grass_entry();
}

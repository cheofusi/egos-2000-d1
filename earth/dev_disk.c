/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a simple disk device driver
 */

#include "egos.h"
#include "disk.h"

const int sdc_no = 0;
const int sdc_bus_width = 4;

unsigned long mmc_bread(int dev_num, unsigned long start, unsigned blkcnt, void *dst);
int sunxi_mmc_init(int sdc_no, unsigned bus_width);

int disk_read(int block_no, int nblocks, char* dst) {
    mmc_bread(sdc_no, block_no, nblocks, dst);
    return 0;
}

int disk_write(int block_no, int nblocks, char* src) {
    FATAL("Write op not yet implemented");
    return 0;
}

void disk_init() {
    earth->disk_read = disk_read;
    earth->disk_write = disk_write;

    sunxi_mmc_init(sdc_no, sdc_bus_width);
}

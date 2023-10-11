/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: a 1MB (256*4KB) paging device, with 256
 * physical frames starting at address FRAME_CACHE_START
 */

#include "egos.h"
#include "disk.h"
#include <stdlib.h>
#include <string.h>
#define NBLOCKS_PER_PAGE PAGE_SIZE / BLOCK_SIZE  /* 4KB / 512B == 8 */

char *pages_start = (void*)FRAME_CACHE_START;

static int cache_eviction() { }

void paging_init() {  }

int paging_invalidate_cache(int frame_id) { }

int paging_write(int frame_id, uintptr_t page_no) {
    char* src = (void*)(page_no << 12);
    memcpy(pages_start + frame_id * PAGE_SIZE, src, PAGE_SIZE);
    return 0;
}

char* paging_read(int frame_id, int alloc_only) {
    return pages_start + frame_id * PAGE_SIZE;
}

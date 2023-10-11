#pragma once

/**
 *  We use the first 5M of the SD card.  
 * 
 *  Earth       : 512K -  1M
 *  Grass       : 1M   -  2M (grass_exec + sys_proc_exec + sys_file_exec + sys_dir_exec + sys_shell_exec)
 *  Filesystem  : 2M   -  4M
 *  Mem Paging  : 4M   -  5M
 */

/* Disk Layout */
#define BLOCK_SIZE            512

#define GRASS_EXEC_START      (1024 * 1024 / BLOCK_SIZE)
#define GRASS_EXEC_SIZE       (1024 * 1024)
#define GRASS_NEXEC           8
#define GRASS_EXEC_SEGMENT    (GRASS_EXEC_SIZE / GRASS_NEXEC / BLOCK_SIZE) /* 128K for each segment */
/** Given 128K per grass segment, we've only used the 640K for Grass, leaving the 360K area before 2M
 * to waste. Why ??
 */
#define SYS_PROC_EXEC_START   GRASS_EXEC_START + GRASS_EXEC_SEGMENT
#define SYS_FILE_EXEC_START   GRASS_EXEC_START + GRASS_EXEC_SEGMENT * 2
#define SYS_DIR_EXEC_START    GRASS_EXEC_START + GRASS_EXEC_SEGMENT * 3
#define SYS_SHELL_EXEC_START  GRASS_EXEC_START + GRASS_EXEC_SEGMENT * 4

#define GRASS_FS_START        2 * 1024 * 1024 / BLOCK_SIZE
#define FS_DISK_SIZE          2 * 1024 * 1024

#define PAGING_DEV_START      4 * 1024 * 1024 / BLOCK_SIZE
#define PAGING_DEV_SIZE       1024 * 1024


typedef unsigned int block_no;      /* index of a block */

typedef struct block {
    char bytes[BLOCK_SIZE];
} block_t;
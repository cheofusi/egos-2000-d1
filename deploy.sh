#!/bin/bash

set -e

DEVICE=/dev/sdb

if [ ! -b ${DEVICE} ]; then
    echo "${DEVICE} does not exist"
    exit 1 
fi

printf "DD'ing Earth layer with size: %sB\n" "$(wc -c build/release/earth.bin | awk '{print $1}')"
dd if=build/release/earth.bin of=${DEVICE} bs=1K seek=512 status=none

printf "DD'ing disk image \n"
dd if=tools/disk.img of=${DEVICE} bs=1K seek=1024 skip=1024 status=none

# The offsets are in KiBs
# EARTH_OFF=512 # 512K
# GRASS_OFF=1024 # 1024K=1M
# SYS_PROC_OFF=$(($GRASS_OFF + 128))
# SYS_FILE_OFF=$(($SYS_PROC_OFF + 128))
# SYS_DIR_OFF=$(($SYS_FILE_OFF + 128))
# SYS_SHELL_OFF=$(($SYS_DIR_OFF + 128))

# printf "DD'ing Earth layer with size: %sB\n" "$(wc -c build/release/earth.bin | awk '{print $1}')"
# dd if=build/release/earth.bin of=${DEVICE} bs=1k seek=${EARTH_OFF} status=none

# printf "DD'ing Grass layer with size: %sB\n" "$(wc -c build/release/grass.elf | awk '{print $1}')"
# dd if=build/release/grass.elf of=${DEVICE} bs=1k seek=${GRASS_OFF} status=none

# printf "DD'ing sys_proc \n"
# dd if=build/release/sys_proc.elf of=/dev/sdb bs=1k seek=${SYS_PROC_OFF} status=none

# printf "DD'ing sys_file to %d\n" ${SYS_FILE_OFF} 
# dd if=build/release/sys_file.elf of=/dev/sdb bs=1k seek=${SYS_FILE_OFF} status=none

# printf "DD'ing sys_dir\n"
# dd if=build/release/sys_dir.elf of=/dev/sdb bs=1k seek=${SYS_DIR_OFF} status=none

# printf "DD'ing sys_shell\n"
# dd if=build/release/sys_shell.elf of=/dev/sdb bs=1k seek=${SYS_SHELL_OFF} status=none


sync
echo Done

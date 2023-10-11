SD_BLKDEV = /dev/sdb

RISCV_CC = riscv64-unknown-elf-gcc -march=rv64im_zicsr_zifencei
OBJDUMP = riscv64-unknown-elf-objdump
OBJCOPY = riscv64-unknown-elf-objcopy

DEBUG = build/debug
RELEASE = build/release

APPS_DEPS = apps/*.* library/egos.h library/*/*
GRASS_DEPS = grass/* library/egos.h library/*/*
EARTH_DIRS = $(shell find earth/ -type d)
EARTH_PRIV_INC_DIRS=$(addprefix -I,$(EARTH_DIRS))
EARTH_DEPS = earth/* earth/modules/*/* library/egos.h library/*/*
USRAPP_ELFS = $(patsubst %.c, $(RELEASE)/%.elf, $(notdir $(wildcard apps/user/*.c)))
SYSAPP_ELFS = $(patsubst %.c, $(RELEASE)/%.elf, $(notdir $(wildcard apps/system/*.c)))

LDFLAGS = -nostdlib -lc -lgcc -Wl,--no-warn-rwx-segments
INCLUDE = -Ilibrary -Ilibrary/elf -Ilibrary/file -Ilibrary/libc -Ilibrary/servers
CFLAGS = -mabi=lp64 -Wl,--gc-sections -ffunction-sections -fdata-sections -fdiagnostics-show-option -mtune=thead-c906
COMMON = $(CFLAGS) $(INCLUDE)
DEBUG_FLAGS = --source --all-headers --demangle --line-numbers --wide

egos: $(USRAPP_ELFS) $(SYSAPP_ELFS) $(RELEASE)/grass.elf $(RELEASE)/earth.elf

$(RELEASE)/earth.elf: $(EARTH_DEPS)
	@echo "$(YELLOW)-------- Compile the Earth Layer --------$(END)"
	$(RISCV_CC) $(COMMON) $(EARTH_PRIV_INC_DIRS) earth/earth.s $(filter %.c, $(wildcard $^)) -Tearth/earth.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(DEBUG)/earth.lst
	@$(OBJCOPY) -O binary $@ $(RELEASE)/earth.bin

$(RELEASE)/grass.elf: $(GRASS_DEPS)
	@echo "$(GREEN)-------- Compile the Grass Layer --------$(END)"
	$(RISCV_CC) $(COMMON) grass/grass.s $(filter %.c, $(wildcard $^)) -Tgrass/grass.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(DEBUG)/grass.lst

$(SYSAPP_ELFS): $(RELEASE)/%.elf : apps/system/%.c $(APPS_DEPS)
	@echo "Compile app$(CYAN)" $(patsubst %.c, %, $(notdir $<)) "$(END)=>" $@
	@$(RISCV_CC) $(COMMON) -Iapps apps/app.s $(filter %.c, $(wildcard $^)) -Tapps/app.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(patsubst %.c, $(DEBUG)/%.lst, $(notdir $<))

$(USRAPP_ELFS): $(RELEASE)/%.elf : apps/user/%.c $(APPS_DEPS)
	@mkdir -p $(DEBUG) $(RELEASE)
	@echo "Compile app$(CYAN)" $(patsubst %.c, %, $(notdir $<)) "$(END)=>" $@
	@$(RISCV_CC) $(COMMON) -Iapps apps/app.s $(filter %.c, $(wildcard $^)) -Tapps/app.lds $(LDFLAGS) -o $@
	@$(OBJDUMP) $(DEBUG_FLAGS) $@ > $(patsubst %.c, $(DEBUG)/%.lst, $(notdir $<))

install: egos
	@echo "$(GREEN)-------- Create the Disk Image --------$(END)"
	$(CC) tools/mkfs.c library/file/file.c -DMKFS $(INCLUDE) -o tools/mkfs; cd tools; ./mkfs

.ONESHELL:
sd: install
	@echo "$(YELLOW)-------- Flash the SD card --------$(END)"
	@echo "Flash egos-2000-d1 to $(SD_BLKDEV) (y/n)?"
	@read answer
	@if [ "$${answer}" != "$${answer#[Nn]}" ]; then \
		exit 0; \
	fi
	@if [ ! -b $(SD_BLKDEV) ]; then \
		echo "$(SD_BLKDEV) does not exist"; \
		exit 1; \
	fi
	@printf "Writing D1 SRAM bootloader..\n"
	@dd if=boot0/boot0lite-sdcard.bin of=$(SD_BLKDEV) bs=1K seek=8 status=none
	@printf "Writing Earth layer binary..\n"
	@dd if=build/release/earth.bin of=$(SD_BLKDEV) bs=1K seek=512 status=none
	@printf "Writing disk image..\n"
	@dd if=tools/disk.img of=$(SD_BLKDEV) bs=1K seek=1024 skip=1024 status=none
	@sync
	@printf "Flash complete\n"

clean:
	rm -rf build tools/mkfs tools/mkrom tools/qemu/qemu.elf tools/disk.img tools/bootROM.bin

GREEN = \033[1;32m
YELLOW = \033[1;33m
CYAN = \033[1;36m
END = \033[0m

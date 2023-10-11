#pragma once

#include <stdint.h>

struct elf64_header {
    unsigned char	e_ident[16];
    uint16_t	e_type;
    uint16_t	e_machine;
    uint32_t	e_version;
    uint64_t	e_entry;
    uint64_t	e_phoff;
    uint64_t	e_shoff;
    uint32_t	e_flags;
    uint16_t	e_ehsize;
    uint16_t	e_phentsize;
    uint16_t	e_phnum;
    uint16_t	e_shentsize;
    uint16_t	e_shnum;
    uint16_t	e_shstrndx;
};

struct elf64_program_header {
    uint32_t	p_type;
    uint32_t	p_flags;
    uint64_t	p_offset;
    uint64_t	p_vaddr;
    uint64_t	p_paddr;
    uint64_t	p_filesz;
    uint64_t	p_memsz;
    uint64_t	p_align;
};

typedef int (*elf_reader)(int block_no, char* dst);
void elf_load(int pid, elf_reader reader, int argc, void** argv);

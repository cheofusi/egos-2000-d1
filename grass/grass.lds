OUTPUT_ARCH("riscv")

ENTRY(grass_enter)

MEMORY
{
    /*
        grass_ro: 8K for .text + .rodata
        grass_rw: 4K for data, bss and heap
     */

    grass_ro : ORIGIN = 0x40024000, LENGTH = 8K
    grass_rw : ORIGIN = 0x40026000, LENGTH = 4K
}

PHDRS
{
    ram PT_LOAD;
}

SECTIONS
{
    __heap_size = 0x800; /* min heap size */

    .text : ALIGN(8) {
        *(.text .text.*)
    } >grass_ro :ram

    .rodata : ALIGN(8) {
        *(.rdata)
        *(.rodata .rodata.*)
        . = ALIGN(8);
        *(.srodata .srodata.*)
    } >grass_ro :ram

    .data : ALIGN(8) {
        *(.data .data.*)
        . = ALIGN(8);
        *(.sdata .sdata.* .sdata2.*)
    } >grass_rw :ram

    .bss (NOLOAD): ALIGN(8) {
        *(.sbss*)
        *(.bss .bss.*)
        *(COMMON)
    } >grass_rw :ram

    .heap (NOLOAD) : ALIGN(8) {
        PROVIDE( __heap_start = . );
          . = __heap_size;
    } >grass_rw :ram
    PROVIDE( __heap_end = ORIGIN(grass_rw) + LENGTH(grass_rw) );
}

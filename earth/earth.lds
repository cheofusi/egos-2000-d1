OUTPUT_ARCH("riscv")

ENTRY(earth_enter)

MEMORY
{
    /* 
        earth_ro: 128K for .init + .text + .rodata
        earth_rw: 16K for data, bss and heap
     */
    earth_ro : ORIGIN = 0x40000000, LENGTH = 128K
    earth_rw : ORIGIN = 0x40020000, LENGTH = 16K
}

SECTIONS
{
    .init : {
        *(.text.enter)
    } >earth_ro

    .text : {
        *(.text .text.*)
    } >earth_ro

    .rodata : {
        *(.rdata)
        *(.rodata .rodata.*)
        . = ALIGN(8);
        *(.srodata .srodata.*)
    } >earth_ro

    .data : ALIGN(8) {
        *(.data .data.*)
        . = ALIGN(8);
        *(.sdata .sdata.* .sdata2.*)
    } >earth_rw

    .bss (NOLOAD): ALIGN(8) {
        *(.sbss*)
        *(.bss .bss.*)
        *(COMMON)
    } >earth_rw

    .heap (NOLOAD): ALIGN(8) {
        PROVIDE( __heap_start = . );
    } >earth_rw

    PROVIDE( bss_start = ADDR(.bss) );
    PROVIDE( bss_end = ADDR(.bss) + SIZEOF(.bss) );
    PROVIDE( data_start = ADDR(.data) );
    PROVIDE( data_end = ADDR(.data) + SIZEOF(.data) );
    PROVIDE( __heap_end =  ORIGIN(earth_rw) + LENGTH(earth_rw));
}

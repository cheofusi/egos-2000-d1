## About

This is a port of the awesome [egos-2000](https://github.com/yhzhang0128/egos-2000/) teaching operating system to Allwinner's D1 chip, using Sipeed's [Lichee RV](https://wiki.sipeed.com/hardware/en/lichee/RV/RV.html) compute module.

![](references/screenshots/egos-2000-d1.png)

There is no attempt at staying under 2K lines of code. Most of the chip dependent sources were gotten from [FlatHeadBro's D1 baremetal programs](https://github.com/Ouyancheng/FlatHeadBro) (to whom I am immensely grateful) and given a facelift.

The port either runs in M-mode, or M+S+U with Page Tables.

### Usages and Documentation

For compiling and running egos-2000-d1, please read [this document](references/USAGES.md).
The [RISC-V instruction set manual](references/riscv-privileged-v1.10.pdf), [C906 processor manual](references/XuanTie-OpenC906-UserManual.pdf) and [D1 chip manual](references/D1_User_Manual_V0.1_Draft_Version.pdf) introduce the privileged ISA and memory map. [egos.h](library/egos.h) details the memory layout


#### TODO
- Mem paging
- User-level threading
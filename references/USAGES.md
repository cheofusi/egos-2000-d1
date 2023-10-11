# Compile and run egos-2000-d1

The following hardware is required:
* [Lichee RV module](https://wiki.sipeed.com/hardware/en/lichee/RV/RV.html) or [Lichee RV dock](https://wiki.sipeed.com/hardware/en/lichee/RV/Dock.html)
* USB Type-C to USB-A for power
* USB-to-TTL Serial Cable for I/O
* microSD card and microSD card adapter


## Toolchain setup

Setup your working directory and name it `$EGOS`.

```shell
> export EGOS=/home/${USER}/egos
> cd $EGOS
> git clone https://github.com/cheofusi/egos-2000-d1.git
```

Compile and install the [official GNU toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain) which takes about an hour. Before proceeding follow the instructions for installing packages needed to build the toolchain.

```shell
# Prepare the environment
> cd $EGOS
> mkdir rv64-egos-2000-d1
> export PATH=$PATH:$EGOS/rv64-egos-2000-d1/bin

# Compile and install the GNU toolchain
> git clone --depth 1 git@github.com:riscv-collab/riscv-gnu-toolchain.git
> cd riscv-gnu-toolchain
> ./configure --with-arch=rv64imac --with-abi=lp64 --prefix=$EGOS/rv64-egos-2000-d1
> make
......
```

## Build and install egos-2000-d1
Plug in your SD card and **make sure your the block device for you SD card corresponds to the one in the prompt when flashing !!**. If it doesn't edit the first line of the Makefile apropriately (`/dev/sdb` by default).

```shell
> cd $EGOS/egos-2000-d1
> make clean && make
......
> make sd
-------- Flash the SD card --------
Writing D1 SRAM bootloader..
Writing Earth layer binary..
Writing disk image..
Flash complete
```

## Run egos-2000-d1
Insert the flashed card into the board, connect the TTL end of the serial cable to the GND, TX & RX pins on the board and the USB end to a USB port on your PC. Then open up a serial console to that port using a tool like screen (`screen /dev/ttyUSB1 115200`) or [picocom](https://packages.debian.org/search?keywords=picocom) (`picocom -b 115200 /dev/ttyUSB0`). Now power on the board with the USB Type-C to USB-A cable

![](screenshots/egos-2000-d1-setup.jpg)

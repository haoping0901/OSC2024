# Lab 0: Environment Setup

## Cross-Platform Development

### Cross Compiler

To compile executable files for the 64-bit ARM architecture, we need to install the relevant packages and tools.

```bash
# for 64-bit ARM architecture
sudo apt install gcc make gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu
```

### Linker

A linker takes multiple input object files in object file format and outputs a single executable file.

* Each input object file contains a list of sections.
* The section in the input file is called the input section, while the corresponding section in the output file is called the output section.

Here is an incomplete linker script.

* `.` is the location counter, which is initialized to zero by default.
* `.text` is defined as an output section.
  * `.text : {*(.text)}` will place all `.text` input sections from all input files at the address that was just assigned to the location counter, i.e., `0x80000`.

```bash
SECTIONS
{
  . = 0x80000;
  .text : { *(.text) }
}
```

### QEMU

Before deploying our code on the actual board, we can emulate the board using an emulator (e.g., QEMU) to test whether our code can execute correctly.

The following command installs the QEMU emulator. You can find more information on the [official website](https://www.qemu.org/download/#linux).

```bash
# The `-arm` flag is added to specify the packages for installation.
sudo apt install qemu-system-arm
```

## From Source Code to Kernel Image

### From Source Code to Object Files

Assuming there is a program named `a.S` with the following content:

* `.section ".text"`: According to the [GNU official manual](https://ftp.gnu.org/old-gnu/Manuals/gas-2.9.1/html_chapter/as_7.html#SEC119), this line of code in the GNU Assembler (GAS) is a directive used to assemble the following code into the `.text` section.
* `wfe`: According to [this document](https://developer.arm.com/documentation/den0024/a/The-A64-instruction-set/System-control-and-other-instructions/Hint-instructions), this code is intended to wait for an event.



```nasm
.section ".text"
_start:
  wfe
  b _start
```

We can convert `a.S` into an object file using the following command:

```bash
aarch64-linux-gnu-gcc -c a.S
```

### From Object Files to ELF

Next, the linker will link the previously compiled object file (i.e., `a.o`) with a linker script `linker.ld` and generate an ELF (Executable and Linkable Format) file (i.e., `nycuos.elf`) in the AArch64 architecture.

```bash
# Using GNU LD
aarch64-linux-gnu-ld -T linker.ld -o nycuos.elf a.o

# Using LLVM
ld.lld -m aarch64elf -T linker.ld -o nycuos.elf a.o
```

### From ELF to Kernel Image

The rpi3 bootloader cannot load ELF files directly, so the following commands are used to convert the ELF file (i.e., `nycuos.elf`) into a raw binary image (i.e., `nycuos.img`) suitable for loading by the rpi3 bootloader.:

```bash
# Using GNU objcopy
aarch64-linux-gnu-objcopy -O binary nycuos.elf nycuos.img

# Or using LLVM objcopy
llvm-objcopy --output-target=aarch64-rpi3-elf -O binary nycuos.elf nycuos.img
```

### Check on QEMU

After building, you can use QEMU to see the dumped assembly.

```bash
qemu-system-aarch64 -machine raspi3b -kernel nycuos.img -display none -d in_asm
```

## Deploy to REAL Rpi3

### Flash Bootable Image to SD Card

To prepare a bootable image for the rpi3, we'll need the following components:

* An FAT16/32 partition containing:
  * Firmware for the GPU.
  * Kernel image (`nycuos.img`).

Here is a method to prepare the bootable image using the bootable image provided by the course website:

1. Download the bootable image provided by the TA from [this link](https://github.com/nycu-caslab/OSC2024/raw/main/supplement/nycuos.img).
2. Use `lsblk` to identify the directory where our SD card is mounted.
3. Use the following command to write the TA-provided bootable image to our SD card:

```bash
# Assuming the SD card is mounted at /dev/sdc on my computer
dd if=nycuos.img of=/dev/sdc
```

## Debugging

### Debugging with QEMU

QEMU combined with GDB can be used to debug binaries, but the GDB used must support different architectures. Therefore, you need to install a specific version of GDB using the following command:

```bash
sudo apt install -y gdb-multiarch
```

Next, use the following command to emulate the rpi3b with QEMU:

* The `-s` option enables the GDB server, which waits for connections from GDB on TCP port 1234. This allows us to perform remote debugging with GDB.

```bash
qemu-system-aarch64 -machine raspi3b -kernel nycuos.img -display none -S -s
```

In GDB, load the `nycuos.elf` file and connect to QEMU's GDB server for debugging.

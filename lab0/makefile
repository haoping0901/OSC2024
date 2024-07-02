TOOLCHAIN_PREFIX = aarch64-linux-gnu-
CC = $(TOOLCHAIN_PREFIX)gcc
LD = $(TOOLCHAIN_PREFIX)ld
OBJCPY = $(TOOLCHAIN_PREFIX)objcopy


TARGET = nycuos.img
TARGET_ELF = nycuos.elf
OBJS = a.o
MACHINE = raspi3b

.PHONY: all check debug clean

all: $(TARGET)

$(TARGET): $(TARGET_ELF)
	$(OBJCPY) -O binary $(TARGET_ELF) $(TARGET)

$(TARGET_ELF): $(OBJS)
	$(LD) -T linker.ld -o nycuos.elf $(OBJS)

$(OBJS): a.S
	$(CC) -c $< -o $@

check:
	qemu-system-aarch64 -machine $(MACHINE) -kernel $(TARGET) -display none -d in_asm

debug:
	@printf "A QEMU emulator for the AArch64 architecture has been started.\n"
	@printf "You can launch GDB, which supports multiple architectures,\n"
	@printf "load the nycuos.elf file, and then connect to QEMU using the following commands.\n"
		
	@printf "file nycuos.elf\n"
	@printf "target remote :1234\n\n"
	
	qemu-system-aarch64 -machine raspi3b -kernel nycuos.img -display none -S -s

clean:
	$(RM) $(TARGET_ELF) *.o
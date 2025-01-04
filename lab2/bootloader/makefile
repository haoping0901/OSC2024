TOOLCHAIN = aarch64-linux-gnu-
CC = $(TOOLCHAIN)gcc
LD = $(TOOLCHAIN)ld
OBJCPY = $(TOOLCHAIN)objcopy
SRC_DIR = ./src
SRCS = $(wildcard $(SRC_DIR)/*.c) 
OBJS = $(SRCS:.c=.o) $(SRC_DIR)/start.o
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles

all: clean bootloader.img

$(SRC_DIR)/start.o: $(SRC_DIR)/start.S
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

bootloader.img: $(SRC_DIR)/start.o $(OBJS)
	$(LD) -T $(SRC_DIR)/linker.ld -o bootloader.elf $(OBJS)
	$(OBJCPY) -O binary bootloader.elf bootloader.img

clean:
	rm bootloader.* $(SRC_DIR)/*.o >/dev/null 2>/dev/null || true

test:
	qemu-system-aarch64 -machine raspi3b -kernel bootloader.img -display none -serial null -serial stdio

debug:
	qemu-system-aarch64 -machine raspi3b -kernel bootloader.img -display none -serial pty
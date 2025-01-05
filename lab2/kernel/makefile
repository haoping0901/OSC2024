TOOLCHAIN = aarch64-linux-gnu-
CC = $(TOOLCHAIN)gcc
LD = $(TOOLCHAIN)ld
OBJCPY = $(TOOLCHAIN)objcopy
SRC_DIR = ./src
SRCS = $(wildcard $(SRC_DIR)/*.c) 
OBJS = $(SRCS:.c=.o) $(SRC_DIR)/start.o
KERNEL = kernel8
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles

all: clean $(KERNEL).img

$(SRC_DIR)/start.o: $(SRC_DIR)/start.S
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL).img: $(SRC_DIR)/start.o $(OBJS)
	$(LD) -T $(SRC_DIR)/linker.ld -o $(KERNEL).elf $(OBJS)
	$(OBJCPY) -O binary $(KERNEL).elf $(KERNEL).img

clean:
	rm $(KERNEL).* $(SRC_DIR)/*.o >/dev/null 2>/dev/null || true
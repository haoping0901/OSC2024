TOOLCHAIN_PREFIX = aarch64-linux-gnu-
CC = $(TOOLCHAIN_PREFIX)gcc
LD = $(TOOLCHAIN_PREFIX)ld
OBJCPY = $(TOOLCHAIN_PREFIX)objcopy
SRC_DIR = ./src
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(SRC_DIR)/%.o)
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles

all: clean kernel8.img

$(SRC_DIR)/start.o: $(SRC_DIR)/start.S
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel8.img: $(SRC_DIR)/start.o $(OBJS)
	$(LD) -T linker.ld -o kernel8.elf $(SRC_DIR)/start.o $(OBJS)
	$(OBJCPY) -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.* start.o $(SRC_DIR)/*.o >/dev/null 2>/dev/null || true

test:
	qemu-system-aarch64 -machine raspi3b -kernel kernel8.img -display none -serial null -serial stdio

debug:
	qemu-system-aarch64 -machine raspi3b -kernel kernel8.img -display none -S -s
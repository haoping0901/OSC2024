TOOLCHAIN_PREFIX = aarch64-linux-gnu-
CC = $(TOOLCHAIN_PREFIX)gcc
LD = $(TOOLCHAIN_PREFIX)ld
OBJCPY = $(TOOLCHAIN_PREFIX)objcopy
SRC_DIR = ./src
INC_DIR = ./include
# SRCS = $(wildcard $(SRC_DIR)/*.c)
# OBJS = $(notdir $(SRCS:%.c=%.o))
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(SRC_DIR)/%.o)
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles

all: nycuos.img

start.o: start.S
	$(CC) $(CFLAGS) -c start.S -o start.o

# %.o: %.c
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

nycuos.img: start.o $(OBJS)
	$(LD) -T linker.ld -o nycuos.elf start.o $(OBJS)
	$(OBJCPY) -O binary nycuos.elf nycuos.img

clean:
	rm nycuos.elf rm nycuos.img start.o $(SRC_DIR)/*.o >/dev/null 2>/dev/null || true

test:
	qemu-system-aarch64 -machine raspi3b -kernel nycuos.img -display none -serial null -serial stdio
BUILD_DIR = ./build
BIN_DIR = ./bin

directories:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BIN_DIR)

DOOM_OBJS = ./build/doom.o

FILES = ./build/kernel.asm.o ./build/interrupts.o ./build/kernel.o ./build/errors.o \
	./build/vga.o ./build/vga13h.o ./build/idt.o ./build/exceptions.o \
	./build/pmm.o ./build/paging.o ./build/kheap.o \
	./build/shell.o ./build/history.o ./build/keyboard.o ./build/timer.o \
	./build/fs.o ./build/vfs.o ./build/romfs.o ./build/editor.o ./build/snake.o \
	./build/klibc.o \
	$(DOOM_OBJS)

FLAGS = -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 \
	-I./src/arch/x86 -I./src/arch/x86/boot -I./src/arch/x86/cpu \
	-I./src/drivers/video/text -I./src/drivers/video/graphics \
	-I./src/drivers/input -I./src/drivers/timer \
	-I./src/kernel -I./src/memory/physical -I./src/memory/virtual -I./src/memory/heap \
	-I./src/fs/vfs -I./src/fs/ramfs -I./src/fs/romfs -I./src/include \
	-I./src/apps/shell -I./src/apps/editor -I./src/apps/games \
	-I./src/apps/games/snake -I./src/apps/games/doom \
	-fno-asynchronous-unwind-tables -fno-exceptions -fno-stack-protector \
	-fno-builtin -m32 -Wa,--32

all: directories ./bin/os.bin

./bin/boot.bin: ./src/arch/x86/boot.asm ./src/arch/x86/gdt.asm ./bin/kernel.bin
	nasm -f bin -I./src/arch/x86 \
	  -DKERNEL_SECTORS=$$(( ($$(stat -c%s ./bin/kernel.bin) + 511) / 512 )) \
	  ./src/arch/x86/boot.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/arch/x86/boot/kernel.asm
	nasm -f elf -g ./src/arch/x86/boot/kernel.asm -o ./build/kernel.asm.o

./build/interrupts.o: ./src/arch/x86/cpu/interrupts.asm
	nasm -f elf -g ./src/arch/x86/cpu/interrupts.asm -o ./build/interrupts.o

./build/kernel.o: ./src/kernel/kernel.c ./src/kernel/kernel.h
	gcc $(FLAGS) -std=gnu99 -c ./src/kernel/kernel.c -o ./build/kernel.o

./build/errors.o: ./src/kernel/errors.c ./src/include/errors.h
	gcc $(FLAGS) -std=gnu99 -c ./src/kernel/errors.c -o ./build/errors.o

./build/vga.o: ./src/drivers/video/text/vga.c ./src/drivers/video/text/vga.h
	gcc $(FLAGS) -std=gnu99 -c ./src/drivers/video/text/vga.c -o ./build/vga.o

./build/vga13h.o: ./src/drivers/video/graphics/vga13h.c ./src/drivers/video/graphics/vga13h.h
	gcc $(FLAGS) -std=gnu99 -c ./src/drivers/video/graphics/vga13h.c -o ./build/vga13h.o

./build/idt.o: ./src/arch/x86/idt.c ./src/arch/x86/cpu/idt.h
	gcc $(FLAGS) -std=gnu99 -c ./src/arch/x86/idt.c -o ./build/idt.o

./build/exceptions.o: ./src/arch/x86/exceptions.c ./src/drivers/video/text/vga.h ./src/arch/x86/cpu/idt.h
	gcc $(FLAGS) -std=gnu99 -c ./src/arch/x86/exceptions.c -o ./build/exceptions.o

./build/pmm.o: ./src/memory/physical/pmm.c ./src/memory/physical/pmm.h
	gcc $(FLAGS) -std=gnu99 -c ./src/memory/physical/pmm.c -o ./build/pmm.o

./build/paging.o: ./src/memory/virtual/paging.c ./src/memory/virtual/paging.h
	gcc $(FLAGS) -std=gnu99 -c ./src/memory/virtual/paging.c -o ./build/paging.o

./build/kheap.o: ./src/memory/heap/kheap.c ./src/memory/heap/kheap.h ./src/memory/physical/pmm.h
	gcc $(FLAGS) -std=gnu99 -c ./src/memory/heap/kheap.c -o ./build/kheap.o

./build/shell.o: ./src/apps/shell/shell.c ./src/apps/shell/shell.h
	gcc $(FLAGS) -std=gnu99 -c ./src/apps/shell/shell.c -o ./build/shell.o

./build/history.o: ./src/apps/shell/history.c ./src/apps/shell/history.h
	gcc $(FLAGS) -std=gnu99 -c ./src/apps/shell/history.c -o ./build/history.o

./build/keyboard.o: ./src/drivers/input/keyboard.c ./src/drivers/input/keyboard.h
	gcc $(FLAGS) -std=gnu99 -c ./src/drivers/input/keyboard.c -o ./build/keyboard.o

./build/timer.o: ./src/drivers/timer/timer.c ./src/drivers/timer/timer.h
	gcc $(FLAGS) -std=gnu99 -c ./src/drivers/timer/timer.c -o ./build/timer.o

./build/fs.o: ./src/fs/ramfs/fs.c ./src/fs/ramfs/fs.h
	gcc $(FLAGS) -std=gnu99 -c ./src/fs/ramfs/fs.c -o ./build/fs.o

./build/vfs.o: ./src/fs/vfs/vfs.c ./src/fs/vfs/vfs.h
	gcc $(FLAGS) -std=gnu99 -c ./src/fs/vfs/vfs.c -o ./build/vfs.o

./build/romfs.o: ./src/fs/romfs/romfs.c ./src/fs/romfs/romfs.h
	gcc $(FLAGS) -std=gnu99 -c ./src/fs/romfs/romfs.c -o ./build/romfs.o

./build/editor.o: ./src/apps/editor/editor.c ./src/apps/editor/editor.h
	gcc $(FLAGS) -std=gnu99 -c ./src/apps/editor/editor.c -o ./build/editor.o

./build/snake.o: ./src/apps/games/snake/snake.c ./src/apps/games/snake/snake.h
	gcc $(FLAGS) -std=gnu99 -c ./src/apps/games/snake/snake.c -o ./build/snake.o

./build/klibc.o: ./src/lib/klibc.c
	gcc $(FLAGS) -std=gnu99 -c ./src/lib/klibc.c -o ./build/klibc.o

./build/doom.o: ./src/apps/games/doom/doom.c ./src/apps/games/doom/doom.h
	gcc $(FLAGS) -std=gnu99 \
		-DDOOMGENERIC_RESX=320 -DDOOMGENERIC_RESY=200 \
		-c ./src/apps/games/doom/doom.c -o ./build/doom.o

./build/kernel.elf: $(FILES) ./linkerscript.ld
	ld -m elf_i386 -g -relocatable $(FILES) -o ./build/completeKernel.o
	gcc $(FLAGS) -T ./linkerscript.ld -o ./build/kernel.elf -ffreestanding -O0 -nostdlib ./build/completeKernel.o -lgcc

./bin/kernel.bin: ./build/kernel.elf
	objcopy -O binary ./build/kernel.elf ./bin/kernel.bin

./bin/os.bin: ./bin/boot.bin ./bin/kernel.bin
	truncate -s 0 ./bin/os.bin
	dd if=./bin/boot.bin status=none >> ./bin/os.bin
	dd if=./bin/kernel.bin status=none >> ./bin/os.bin
	dd if=/dev/zero bs=512 count=8 status=none >> ./bin/os.bin

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
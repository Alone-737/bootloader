FILES = ./build/kernel.asm.o ./build/interrupts.o ./build/kernel.o ./build/vga.o ./build/idt.o ./build/exceptions.o ./build/pmm.o ./build/kheap.o ./build/shell.o ./build/history.o ./build/keyboard.o ./build/fs.o ./build/vfs.o ./build/editor.o
FLAGS = -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -I./src/arch/x86 -I./src/drivers -I./src/kernel -I./src/memory -I./src/shell -I./src/fs -I./src/include -I./src/editor -fno-asynchronous-unwind-tables -fno-exceptions -fno-stack-protector -fno-builtin -m32 -Wa,--32

all:
	nasm -f bin -I./src/arch/x86 ./src/arch/x86/boot.asm -o ./bin/boot.bin
	nasm -f elf -g ./src/arch/x86/kernel.asm -o ./build/kernel.asm.o
	nasm -f elf -g ./src/arch/x86/interrupts.asm -o ./build/interrupts.o
	gcc $(FLAGS) -std=gnu99 -c ./src/drivers/vga.c -o ./build/vga.o
	gcc $(FLAGS) -std=gnu99 -c ./src/arch/x86/idt.c -o ./build/idt.o
	gcc $(FLAGS) -std=gnu99 -c ./src/arch/x86/exceptions.c -o ./build/exceptions.o
	gcc $(FLAGS) -std=gnu99 -c ./src/memory/pmm.c -o ./build/pmm.o
	gcc $(FLAGS) -std=gnu99 -c ./src/memory/kheap.c -o ./build/kheap.o
	gcc $(FLAGS) -std=gnu99 -c ./src/fs/vfs.c -o ./build/vfs.o
	gcc $(FLAGS) -std=gnu99 -c ./src/shell/shell.c -o ./build/shell.o
	gcc $(FLAGS) -std=gnu99 -c ./src/shell/history.c -o ./build/history.o
	gcc $(FLAGS) -std=gnu99 -c ./src/drivers/keyboard.c -o ./build/keyboard.o
	gcc $(FLAGS) -std=gnu99 -c ./src/fs/fs.c -o ./build/fs.o
	gcc $(FLAGS) -std=gnu99 -c ./src/kernel/kernel.c -o ./build/kernel.o
	gcc $(FLAGS) -std=gnu99 -c ./src/editor/editor.c -o ./build/editor.o
	ld -m elf_i386 -g -relocatable $(FILES) -o ./build/completeKernel.o
	gcc $(FLAGS) -T ./linkerscript.ld -o ./build/kernel.elf -ffreestanding -O0 -nostdlib ./build/completeKernel.o
	objcopy -O binary ./build/kernel.elf ./bin/kernel.bin
	truncate -s 0 ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=512 count=8 >> ./bin/os.bin

clean:
	rm -rf ./bin/boot.bin
	rm -rf ./bin/kernel.bin
	rm -rf ./bin/os.bin
	rm -rf ./build/kernel.asm.o
	rm -rf ./build/interrupts.o
	rm -rf ./build/fs.o
	rm -rf ./build/kernel.o
	rm -rf ./build/editor.o
	rm -rf ./build/vga.o
	rm -rf ./build/idt.o
	rm -rf ./build/exceptions.o
	rm -rf ./build/pmm.o
	rm -rf ./build/kheap.o
	rm -rf ./build/vfs.o
	rm -rf ./build/shell.o
	rm -rf ./build/history.o
	rm -rf ./build/keyboard.o
	rm -rf ./build/completeKernel.o
	rm -rf ./build/kernel.elf
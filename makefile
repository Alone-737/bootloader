FILES = ./build/kernel.asm.o ./build/interrupts.o ./build/kernel.o ./build/vga.o ./build/idt.o ./build/exceptions.o ./build/pmm.o
FLAGS = -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc -fno-asynchronous-unwind-tables -fno-exceptions -fno-stack-protector -fno-builtin -m32 -Wa,--32

all:
	nasm -f bin ./src/boot.asm -o ./bin/boot.bin
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o
	nasm -f elf -g ./src/interrupts.asm -o ./build/interrupts.o
	i686-elf-gcc -I./src $(FLAGS) -std=gnu99 -c ./src/vga.c -o ./build/vga.o
	i686-elf-gcc -I./src $(FLAGS) -std=gnu99 -c ./src/idt.c -o ./build/idt.o
	i686-elf-gcc -I./src $(FLAGS) -std=gnu99 -c ./src/exceptions.c -o ./build/exceptions.o
	i686-elf-gcc -I./src $(FLAGS) -std=gnu99 -c ./src/pmm.c -o ./build/pmm.o
	i686-elf-gcc -I./src $(FLAGS) -std=gnu99 -c ./src/kernel.c -o ./build/kernel.o
	i686-elf-ld -g -relocatable $(FILES) -o ./build/completeKernel.o
	i686-elf-gcc $(FLAGS) -T ./linkerscript.ld -o ./build/kernel.elf -ffreestanding -O0 -nostdlib ./build/completeKernel.o
	i686-elf-objcopy -O binary ./build/kernel.elf ./bin/kernel.bin
	truncate -s 0 ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=512 count=8 >> ./bin/os.bin

clean:
	rm -rf ./bin/boot.bin
	rm -rf ./bin/kernel.bin
	rm -rf ./bin/os.bin
	rm -rf ./build/kernel.asm.o
	rm -rf ./build/kernel.o
	rm -rf ./build/vga.o
	rm -rf ./build/completeKernel.o
	rm -rf ./build/kernel.elf
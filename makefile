FILES = ./build/kernel.asm.o ./build/interrupts.o ./build/kernel.o ./build/vga.o ./build/idt.o ./build/exceptions.o ./build/pmm.o ./build/kheap.o ./build/shell.o ./build/history.o ./build/keyboard.o ./build/fs.o ./build/vfs.o ./build/editor.o
FLAGS = -g -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -I./src/arch/x86 -I./src/drivers -I./src/kernel -I./src/memory -I./src/shell -I./src/fs -I./src/include -I./src/editor -fno-asynchronous-unwind-tables -fno-exceptions -fno-stack-protector -fno-builtin -m32 -Wa,--32

all: ./bin/os.bin

./bin/boot.bin: ./src/arch/x86/boot.asm ./src/arch/x86/gdt.asm ./bin/kernel.bin
	nasm -f bin -I./src/arch/x86 \
	  -DKERNEL_SECTORS=$$(( ($$(stat -c%s ./bin/kernel.bin) + 511) / 512 )) \
	  ./src/arch/x86/boot.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/arch/x86/kernel.asm
	nasm -f elf -g ./src/arch/x86/kernel.asm -o ./build/kernel.asm.o

./build/interrupts.o: ./src/arch/x86/interrupts.asm
	nasm -f elf -g ./src/arch/x86/interrupts.asm -o ./build/interrupts.o

./build/vga.o: ./src/drivers/vga.c ./src/drivers/vga.h
	gcc $(FLAGS) -std=gnu99 -c ./src/drivers/vga.c -o ./build/vga.o

./build/idt.o: ./src/arch/x86/idt.c ./src/arch/x86/idt.h
	gcc $(FLAGS) -std=gnu99 -c ./src/arch/x86/idt.c -o ./build/idt.o

./build/exceptions.o: ./src/arch/x86/exceptions.c ./src/drivers/vga.h ./src/arch/x86/idt.h
	gcc $(FLAGS) -std=gnu99 -c ./src/arch/x86/exceptions.c -o ./build/exceptions.o

./build/pmm.o: ./src/memory/pmm.c ./src/memory/pmm.h
	gcc $(FLAGS) -std=gnu99 -c ./src/memory/pmm.c -o ./build/pmm.o
<<<<<<< HEAD
	gcc $(FLAGS) -std=gnu99 -c ./src/memory/kheap.c -o ./build/kheap.o
	gcc $(FLAGS) -std=gnu99 -c ./src/fs/vfs.c -o ./build/vfs.o
=======

./build/kheap.o: ./src/memory/kheap.c ./src/memory/kheap.h ./src/memory/pmm.h
	gcc $(FLAGS) -std=gnu99 -c ./src/memory/kheap.c -o ./build/kheap.o

./build/vfs.o: ./src/fs/vfs.c ./src/fs/vfs.h
	gcc $(FLAGS) -std=gnu99 -c ./src/fs/vfs.c -o ./build/vfs.o

./build/shell.o: ./src/shell/shell.c ./src/shell/shell.h
>>>>>>> f7fec4a (added new things and bug fixes)
	gcc $(FLAGS) -std=gnu99 -c ./src/shell/shell.c -o ./build/shell.o

./build/history.o: ./src/shell/history.c ./src/shell/history.h
	gcc $(FLAGS) -std=gnu99 -c ./src/shell/history.c -o ./build/history.o

./build/keyboard.o: ./src/drivers/keyboard.c ./src/drivers/keyboard.h
	gcc $(FLAGS) -std=gnu99 -c ./src/drivers/keyboard.c -o ./build/keyboard.o
<<<<<<< HEAD
	gcc $(FLAGS) -std=gnu99 -c ./src/fs/fs.c -o ./build/fs.o
	gcc $(FLAGS) -std=gnu99 -c ./src/kernel/kernel.c -o ./build/kernel.o
	gcc $(FLAGS) -std=gnu99 -c ./src/editor/editor.c -o ./build/editor.o
=======

./build/fs.o: ./src/fs/fs.c ./src/fs/fs.h
	gcc $(FLAGS) -std=gnu99 -c ./src/fs/fs.c -o ./build/fs.o

./build/kernel.o: ./src/kernel/kernel.c ./src/kernel/kernel.h
	gcc $(FLAGS) -std=gnu99 -c ./src/kernel/kernel.c -o ./build/kernel.o

./build/editor.o: ./src/editor/editor.c ./src/editor/editor.h
	gcc $(FLAGS) -std=gnu99 -c ./src/editor/editor.c -o ./build/editor.o

./build/kernel.elf: $(FILES) ./linkerscript.ld
>>>>>>> f7fec4a (added new things and bug fixes)
	ld -m elf_i386 -g -relocatable $(FILES) -o ./build/completeKernel.o
	gcc $(FLAGS) -T ./linkerscript.ld -o ./build/kernel.elf -ffreestanding -O0 -nostdlib ./build/completeKernel.o

./bin/kernel.bin: ./build/kernel.elf
	objcopy -O binary ./build/kernel.elf ./bin/kernel.bin

./bin/os.bin: ./bin/boot.bin ./bin/kernel.bin
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
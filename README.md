# Bare-Metal OS Bootloader

A minimal x86 operating system bootloader written in assembly and C.

## Features

- Real Mode to Protected Mode switching
- Global Descriptor Table (GDT) implementation
- A20 line enable
- Kernel loaded from disk
- C kernel with assembly entry point
- Cross-compiled with i686-elf-gcc
- QEMU testing support
- GDB debugging capabilities

## Building

```bash
# Set up cross-compiler path
export PATH="/usr/local/gcc-cross/bin:$PATH"

# Build OS image
./build.sh

# Or use make
make clean
make
```

## Testing

```bash
# Run with QEMU
qemu-system-x86_64 -drive format=raw,file=./bin/os.bin,if=ide,index=0

# Debug with GDB
qemu-system-x86_64 -drive format=raw,file=./bin/os.bin,if=ide,index=0 -s -S &
gdb ./bin/kernel.bin
(gdb) target remote localhost:1234
(gdb) break shoot_on_your_own_foot
(gdb) continue
```

## Architecture

- **Bootloader** (`src/arch/x86/boot.asm`): Real mode initialization, disk loading, mode switching
- **Kernel Assembly** (`src/arch/x86/kernel.asm`): 32-bit entry point
- **Interrupts and IDT** (`src/arch/x86/interrupts.asm`, `src/arch/x86/idt.c`, `src/arch/x86/idt.h`): Exception and interrupt setup
- **Drivers** (`src/drivers/`): VGA and keyboard support
- **Kernel C** (`src/kernel/kernel.c`): Main kernel logic
- **Memory** (`src/memory/pmm.c`, `src/memory/pmm.h`): Physical memory management
- **Shell** (`src/shell/`): Interactive shell and command history
- **Linker Script** (`linkerscript.ld`): Memory layout

## Requirements

- NASM assembler
- i686-elf GCC cross-compiler
- GNU Make
- QEMU (for testing)
- GDB (for debugging)

## License

MIT

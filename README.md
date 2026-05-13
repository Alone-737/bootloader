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
qemu-system-x86_64 -hda ./bin/os.bin

# Debug with GDB
qemu-system-x86_64 -hda ./bin/os.bin -s -S &
gdb ./bin/kernel.bin
(gdb) target remote localhost:1234
(gdb) break kernel_main
(gdb) continue
```

## Architecture

- **Bootloader** (`src/boot.asm`): Real mode initialization, disk loading, mode switching
- **Kernel Assembly** (`src/kernel.asm`): 32-bit entry point
- **Kernel C** (`src/kernel.c`): Main kernel logic
- **Linker Script** (`src/linkerscript.ld`): Memory layout

## Requirements

- NASM assembler
- i686-elf GCC cross-compiler
- GNU Make
- QEMU (for testing)
- GDB (for debugging)


## License

MIT

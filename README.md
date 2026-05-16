# Bare-Metal OS Bootloader

A minimal x86 operating system bootloader written in assembly and C.

## Features

- Real Mode to Protected Mode switching
- Global Descriptor Table (GDT) implementation
- A20 line enable
- Kernel loaded from disk
- C kernel with assembly entry point
- Physical Memory Manager (PMM)
- Interactive Shell with piping (`|`) and redirection (`>`, `>>`, `<`)
- Functional RAM Filesystem (flat structure)
- QEMU testing support
- GDB debugging capabilities

## Architecture

- **Bootloader** (`src/arch/x86/boot.asm`): Real mode initialization, disk loading, mode switching
- **Kernel Assembly** (`src/arch/x86/kernel.asm`): 32-bit entry point
- **Interrupts and IDT** (`src/arch/x86/interrupts.asm`, `src/arch/x86/idt.c`, `src/arch/x86/idt.h`): Exception and interrupt setup
- **Drivers** (`src/drivers/`): VGA and keyboard support
- **Kernel C** (`src/kernel/kernel.c`): Main kernel logic
- **Memory** (`src/memory/pmm.c`, `src/memory/pmm.h`): Physical memory management
- **Shell** (`src/shell/`): Interactive shell and command history
- **Linker Script** (`linkerscript.ld`): Memory layout

## Installation

### GCC and Binutils

The bootloader requires **GCC** (GNU Compiler Collection) and **binutils** to compile and link the kernel and bootloader code.

**GCC** compiles C code to machine code, while **binutils** provides essential tools like:

- `as` (assembler) - assembles .asm files to object files
- `ld` (linker) - links object files into the final kernel binary
- `objdump`, `objcopy`, `readelf` - utilities for examining and manipulating binary files

#### Debian/Ubuntu

```bash
sudo apt install gcc binutils nasm
```

#### Arch Linux

```bash
sudo pacman -S gcc binutils nasm
```

#### Fedora

```bash
sudo dnf install gcc binutils nasm
```

#### macOS (Homebrew)

```bash
brew install gcc binutils nasm
```

Verify installation:

```bash
gcc --version
ld --version
as --version
```

## Quick Start

After installing dependencies (see Installation section):

```bash
# Build OS image using native GCC (with -m32)
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

## Requirements

- NASM assembler
- GCC (with support for -m32)
- GNU Make
- QEMU (for testing)
- GDB (for debugging)

## Time Spent

- ~16hours

## License

MIT

# my loneliness is killing me

A hobby x86 operating system born from loneliness, written from scratch in assembly and C with a working shell, text editor, virtual filesystem, ROM filesystem, and a playable Snake game.

## Features

- Real Mode to Protected Mode switching
- Global Descriptor Table (GDT) implementation
- A20 line enable
- Kernel loaded from disk
- C kernel with assembly entry point
- Physical Memory Manager (PMM)
- PIT-based timer driver
- Interactive Shell with piping (`|`) and redirection (`>`, `>>`, `<`)
- **Arrow key history navigation**: press **Up** / **Down** to cycle through previously typed commands
- Virtual Filesystem (VFS) with inode/dentry tree
- RAM Filesystem (flat structure)
- ROM Filesystem (built-in read-only files: help.txt, version.txt, motd.txt)
- Gap-buffer text editor (`edit`)
- VGA Mode 13h graphics driver (320×200, 256 colors)
- Playable Snake game (`snake`)
- QEMU testing support
- GDB debugging capabilities

## Architecture

- **Bootloader** (`src/arch/x86/boot.asm`): Real mode initialization, disk loading, mode switching
- **Kernel Assembly** (`src/arch/x86/kernel.asm`): 32-bit entry point
- **Interrupts and IDT** (`src/arch/x86/interrupts.asm`, `src/arch/x86/idt.c`, `src/arch/x86/idt.h`): Exception and interrupt setup
- **Drivers** (`src/drivers/`): VGA text mode, VGA mode 13h, keyboard, and timer
- **Kernel C** (`src/kernel/kernel.c`): Main kernel logic
- **Memory** (`src/memory/pmm.c`, `src/memory/pmm.h`): Physical memory management (256MB)
- **ROM Filesystem** (`src/fs/romfs/`): Built-in read-only files (help.txt, version.txt, motd.txt)
- **Shell** (`src/shell/`): Interactive shell and command history
- **Filesystem** (`src/fs/`): Flat RAM FS and VFS layer
- **Editor** (`src/editor/`): Gap-buffer text editor
- **Games** (`src/games/`): Snake
- **Linker Script** (`linkerscript.ld`): Memory layout

## Shell Commands

| Command | Description |
|---------|-------------|
| `help` | List all commands |
| `clear` | Clear the screen |
| `echo <text>` | Print text |
| `uname` / `whoami` | Show current user |
| `history` | Show command history |
| `!N` | Run command N from history |
| `ls` | List files |
| `pwd` | Print working directory |
| `date` | Show date and time from RTC |
| `cat <file>` | Display file contents |
| `rm <file>` | Delete a file |
| `mv <src> <dst>` | Rename a file |
| `cp <src> <dst>` | Copy a file |
| `grep <pattern> <file>` | Search for text in a file |
| `wc <file>` | Count lines, words, characters |
| `touch <file>` | Create a new file |
| `mkdir <dir>` | Create a directory |
| `edit [file]` | Open the gap-buffer text editor |
| `snake` | Play Snake |
| `doom` | Play Doom (reboots to exit) |
| `reboot` | Reboot the PC |
| `sleep <seconds>` | Wait for N seconds |
| `hexdump <file>` | Hex dump of a file |

Piping and redirection: `|`, `>`, `>>`, `<`

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

- ~76hours

## License

MIT

# Abyss

The Abyss operating system runs the Solace kernel. Both are hobby projects
written from scratch in NASM assembly, freestanding C, and a little Rust. It
boots from a raw disk image and runs on QEMU.

## Features

- BIOS boot sector and custom bootloader, real mode to protected mode
- GDT, IDT, exception handlers, A20 line
- Physical memory manager, paging with a user-accessible window, kernel heap
- PIT timer, PS/2 keyboard, VGA text mode, VGA Mode 13h
- Ring 3 user mode with Linux `int 0x80` system calls and `-errno` returns
- ELF loader: `exec <file>` runs a user program in ring 3
- Mount table with `procfs` (`/proc`) and `devfs` (`/dev/null`, `/dev/zero`)
- Rust static library linked into the kernel (`rust/solace.rs`)
- Network stack: PCI scan, RTL8139 driver (polled PIO), ARP, IPv4, ICMP echo
- VFS with RAM and ROM filesystems, gap-buffer text editor
- Snake and Doom
- Shell with pipes (`|`), redirection (`>`, `>>`, `<`), and history
- QEMU testing and GDB debugging

## Repository layout

```
src/
  arch/x86/       boot code, GDT, IDT, interrupts, syscalls
  apps/           shell, editor, games
  drivers/        video, keyboard, timer, net (RTL8139)
  fs/             VFS, RAMFS, ROMFS
  kernel/         kernel entry and error handling
  lib/            freestanding C helpers
  memory/         physical, virtual, heap
  net/            ARP, IPv4, ICMP
rust/             Rust sources (staticlib, no Cargo)
```

## Building

Dependencies: `gcc` (with `-m32` support), `binutils`, `nasm`, `make`, `rustc`
with the `i586-unknown-linux-gnu` target, and QEMU for testing.

```bash
rustup target add i586-unknown-linux-gnu

make clean
make          # builds bin/os.bin
make run      # boots QEMU with an RTL8139 NIC
make debug    # boots QEMU paused for GDB
```

Package install:

```bash
# Debian/Ubuntu
sudo apt install gcc binutils nasm make qemu-system-x86

# Arch
sudo pacman -S gcc binutils nasm make qemu-system-x86

# Fedora
sudo dnf install gcc binutils nasm make qemu-system-x86
```

## Shell commands

| Command | Description |
|---------|-------------|
| `help` | List commands |
| `clear` / `echo <text>` | Clear the screen / print text |
| `uname` / `whoami` | Show OS version / current user |
| `history` / `!N` | Show history / run command N |
| `ls` / `pwd` | List files / print working directory |
| `date` | Read the RTC |
| `cat` / `rm` / `mv` / `cp` | File operations |
| `grep` / `wc` / `hexdump <file>` | Inspect files |
| `touch` / `mkdir` | Create files and directories |
| `exec <file>` | Load an ELF file and run it in ring 3 |
| `mount` | List mounted filesystems |
| `edit [file]` | Gap-buffer text editor |
| `ping [a.b.c.d] [count]` | ICMP echo (default gateway, 4 packets) |
| `ring3test` | Drop to ring 3, make syscalls, return |
| `snake` / `doom` | Games |
| `reboot` / `sleep <seconds>` | Power and timing |

Pipes and redirection: `|`, `>`, `>>`, `<`

## Networking

The stack is static, matching QEMU's user-mode network: guest `10.0.2.15`,
gateway `10.0.2.2`, no DHCP or DNS. `ping` resolves the next hop with ARP, then
sends ICMP echo requests. There is no TCP/HTTP yet.

```bash
root@abyss$ ping
reply from 10.0.2.2: icmp_seq=1 time=0 ms
...
--- 10.0.2.2: 4 sent, 4 received ---
```

`/proc` and `/dev` are mounted at boot: `cat /proc/uptime`, `cat
/proc/version`, and `echo x > /dev/null` all work. `exec` loads an ELF image
linked at `0x400000` (see `src/apps/userland/hello.asm`).

## Debugging

```bash
make debug
gdb ./build/kernel.elf
(gdb) target remote localhost:1234
(gdb) break kernel_start
(gdb) continue
```

## License

MIT

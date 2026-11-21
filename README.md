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
- Preemptive round-robin scheduler: idle is PID 0, shell is PID 1, timer ticks switch tasks
- Rust process table with typed PIDs, 7-field PCBs, fixed stacks, and an assembly context switch
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
  solace.rs       crate root, checksum routine
  process/
    mod.rs        module exports
    pid.rs        typed PID and atomic allocator
    context.rs    repr(C) saved register state
    process.rs    PCB, process table, task creation, idle, spawn
    scheduler.rs  context-switch decision
    switch.asm    assembly entry into the first task
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
| `ping [host|a.b.c.d] [count]` | Resolve a hostname with DNS, then ICMP echo |
| `ring3test` | Drop to ring 3, make syscalls, return |
| `snake` / `doom` | Games |
| `reboot` / `sleep <seconds>` | Power and timing |

Pipes and redirection: `|`, `>`, `>>`, `<`

## Process table

`rust/process/` defines the process model for Solace. Each PCB has seven
fields: `pid`, `state`, `context`, `kernel_stack`, `user_stack`, `entry_point`,
and `cr3`. The table is a fixed array of 64 slots with 16 KiB stacks. PIDs come
from an atomic counter.

Boot creates idle (PID 0) and the shell (PID 1). `sched_start` enters the shell
through a prepared `popad`/`iretd` frame. Each PIT tick calls `sched_switch`,
which saves the current stack pointer and loads the next ready task. Idle halts
the CPU until the next interrupt.

`proc_spawn(entry)` is available to C code but no shell command calls it yet,
so Doom, Snake, and the editor still take over the shell instead of running as
separate tasks. There is no `fork`, `wait`, signals, or per-process paging.

## Networking

The stack is static, matching QEMU's user-mode network: guest `10.0.2.15`,
gateway `10.0.2.2`, no DHCP. `ping` resolves the next hop with ARP, then
sends ICMP echo requests; hostnames go through a minimal DNS client that
asks the slirp resolver at `10.0.2.3` for an A record. There is no
TCP/HTTP yet.

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

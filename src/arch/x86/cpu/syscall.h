#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

#define SYS_WRITE   0
#define SYS_READ    1
#define SYS_GETPID  2
#define SYS_YIELD   3
#define SYS_EXIT    4

void syscall_init(void);

#endif
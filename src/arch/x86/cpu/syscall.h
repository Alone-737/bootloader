#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

/* Linux i386 system-call numbers. */
#define SYS_EXIT    1
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_GETPID  20
#define SYS_YIELD   158

void syscall_init(void);

#endif

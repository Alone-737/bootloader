#ifndef ELF_H
#define ELF_H

#include <stdint.h>

/* Loads a flat ELF32 i386 executable into the user window at USER_BASE.
   On success returns 0 and sets *entry to e_entry. */
int elf_load(const char *data, uint32_t size, uint32_t *entry);

#endif

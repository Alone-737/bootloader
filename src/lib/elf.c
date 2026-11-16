/*
 * ELF32 executable loader: checks the header, copies each PT_LOAD
 * segment to its p_vaddr, and returns the entry point.
 */

#include "elf.h"
#include "paging.h"
#include "string.h"

#define PT_LOAD 1
#define ET_EXEC 2
#define EM_386  3

struct elf32_ehdr
{
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
} __attribute__((packed));

struct elf32_phdr
{
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} __attribute__((packed));

static int in_user_window(uint32_t addr, uint32_t len)
{
    return addr >= USER_BASE && len <= USER_LIMIT - addr;
}

int elf_load(const char *data, uint32_t size, uint32_t *entry)
{
    if (size < sizeof(struct elf32_ehdr))
        return -1;

    const struct elf32_ehdr *eh = (const struct elf32_ehdr *)data;
    if (eh->e_ident[0] != 0x7F || eh->e_ident[1] != 'E' ||
        eh->e_ident[2] != 'L' || eh->e_ident[3] != 'F')
        return -1;
    if (eh->e_type != ET_EXEC || eh->e_machine != EM_386)
        return -1;
    if (!in_user_window(eh->e_entry, 1))
        return -1;

    if (eh->e_phoff + (uint32_t)eh->e_phnum * sizeof(struct elf32_phdr) > size)
        return -1;

    const struct elf32_phdr *ph =
        (const struct elf32_phdr *)(data + eh->e_phoff);

    for (int i = 0; i < eh->e_phnum; i++)
    {
        if (ph[i].p_type != PT_LOAD)
            continue;
        if (!in_user_window(ph[i].p_vaddr, ph[i].p_memsz))
            return -1;
        if (ph[i].p_offset > size || ph[i].p_filesz > size - ph[i].p_offset)
            return -1;

        /* filesz bytes are in the file; the rest of memsz is BSS. */
        uint8_t *dst = (uint8_t *)ph[i].p_vaddr;
        const uint8_t *src = (const uint8_t *)(data + ph[i].p_offset);
        for (uint32_t j = 0; j < ph[i].p_filesz; j++)
            dst[j] = src[j];
        for (uint32_t j = ph[i].p_filesz; j < ph[i].p_memsz; j++)
            dst[j] = 0;
    }

    *entry = eh->e_entry;
    return 0;
}

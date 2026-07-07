#include "kheap.h"
#include "pmm.h"
#include "errors.h"

struct block_header
{
    uint32_t magic;
    size_t size;
    struct block_header *next;
};

static struct block_header *free_list = NULL;
static int heap_initialized = 0;

void kheap_init(void)
{
    if (heap_initialized)
        return;

    for (uint32_t i = 0; i < KHEAP_PAGES; i++)
    {
        void *addr = (void *)(KHEAP_START + i * PAGE_SIZE);
        if (pmm_is_used(addr))
            panic("kheap: page 0x%x already in use", addr);
        pmm_mark_used(addr);
    }

    free_list = (struct block_header *)KHEAP_START;
    free_list->magic = KHEAP_MAGIC;
    free_list->size = KHEAP_SIZE - sizeof(struct block_header);
    free_list->next = NULL;

    heap_initialized = 1;
}

void *kmalloc(size_t size)
{
    if (size == 0)
        return NULL;

    struct block_header *prev = NULL;
    struct block_header *curr = free_list;

    size_t aligned_size = size;
    if (aligned_size % 4 != 0)
        aligned_size += 4 - (aligned_size % 4);

    size_t needed = aligned_size + sizeof(struct block_header);

    while (curr)
    {
        if (curr->magic != KHEAP_MAGIC)
            panic("kheap: corrupted block 0x%x magic=0x%x", curr, curr->magic);

        if (curr->size >= needed)
        {
            size_t remaining = curr->size - needed;
            if (remaining <= sizeof(struct block_header) + 4)
            {
                if (prev)
                    prev->next = curr->next;
                else
                    free_list = curr->next;

                curr->magic = 0;
                return (void *)((uint8_t *)curr + sizeof(struct block_header));
            }

            struct block_header *split = (struct block_header *)((uint8_t *)curr + needed);
            split->magic = KHEAP_MAGIC;
            split->size = remaining;
            split->next = curr->next;

            if (prev)
                prev->next = split;
            else
                free_list = split;

            curr->size = needed;
            curr->magic = 0;
            return (void *)((uint8_t *)curr + sizeof(struct block_header));
        }

        prev = curr;
        curr = curr->next;
    }

    return NULL;
}

void kfree(void *ptr)
{
    if (!ptr)
        return;

    struct block_header *header = (struct block_header *)((uint8_t *)ptr - sizeof(struct block_header));
    if (header->magic != 0 || header->size == 0)
        return;

    header->magic = KHEAP_MAGIC;

    struct block_header *prev = NULL;
    struct block_header *curr = free_list;

    while (curr && (uint32_t)curr < (uint32_t)header)
    {
        prev = curr;
        curr = curr->next;
    }

    header->next = curr;

    if (prev)
    {
        prev->next = header;

        struct block_header *coalesce = (struct block_header *)((uint8_t *)prev + sizeof(struct block_header) + prev->size);
        if ((uint32_t)coalesce == (uint32_t)header)
        {
            prev->size += sizeof(struct block_header) + header->size;
            prev->next = header->next;
            header = prev;
        }
    }
    else
    {
        free_list = header;
    }

    if (header->next)
    {
        struct block_header *next_coalesce = (struct block_header *)((uint8_t *)header + sizeof(struct block_header) + header->size);
        if ((uint32_t)next_coalesce == (uint32_t)header->next)
        {
            header->size += sizeof(struct block_header) + header->next->size;
            header->next = header->next->next;
        }
    }
}

size_t kmalloc_usable_size(void *ptr)
{
    if (!ptr)
        return 0;
    struct block_header *header = (struct block_header *)((uint8_t *)ptr - sizeof(struct block_header));
    return header->size - sizeof(struct block_header);
}

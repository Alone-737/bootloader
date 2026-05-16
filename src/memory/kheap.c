#include "kheap.h"
#include "pmm.h"

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

    void *base = pmm_allocate_block();
    if (!base)
    {
        for (;;)
            ;
    }

    free_list = (struct block_header *)base;
    free_list->magic = KHEAP_MAGIC;
    free_list->size = PAGE_SIZE - sizeof(struct block_header);
    free_list->next = NULL;

    for (int i = 1; i < KHEAP_PAGES; i++)
    {
        void *page = pmm_allocate_block();
        if (!page)
        {
            for (;;)
                ;
        }

        struct block_header *new_block = (struct block_header *)page;
        new_block->magic = KHEAP_MAGIC;
        new_block->size = PAGE_SIZE - sizeof(struct block_header);
        new_block->next = free_list;
        free_list = new_block;
    }

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
        {
            for (;;)
                ;
        }

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

    header->magic = KHEAP_MAGIC;
    header->size = 0;

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

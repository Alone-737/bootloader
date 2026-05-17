#include "kheap.h"
#include "pmm.h"

struct block_header
{
<<<<<<< HEAD
    uint32_t magic;
=======
    uint32_t create;
>>>>>>> f7fec4a (added new things and bug fixes)
    size_t size;
    struct block_header *next;
};

static struct block_header *free_list = NULL;
static int heap_initialized = 0;

void kheap_init(void)
{
    if (heap_initialized)
        return;

<<<<<<< HEAD
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
=======
    /* reserve PMM blocks at fixed KHEAP_START */
    for (uint32_t i = 0; i < KHEAP_PAGES; i++)
    {
        void *addr = (void *)(KHEAP_START + i * PAGE_SIZE);
        if (pmm_is_used(addr))
>>>>>>> f7fec4a (added new things and bug fixes)
        {
            for (;;)
                ;
        }
<<<<<<< HEAD

        struct block_header *new_block = (struct block_header *)page;
        new_block->magic = KHEAP_MAGIC;
        new_block->size = PAGE_SIZE - sizeof(struct block_header);
        new_block->next = free_list;
        free_list = new_block;
    }

=======
        pmm_mark_used(addr);
    }

    free_list = (struct block_header *)KHEAP_START;
    free_list->create = KHEAP_CREATE;
    free_list->size = KHEAP_SIZE - sizeof(struct block_header);
    free_list->next = NULL;

>>>>>>> f7fec4a (added new things and bug fixes)
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
<<<<<<< HEAD
        if (curr->magic != KHEAP_MAGIC)
=======
        if (curr->create != KHEAP_CREATE)
>>>>>>> f7fec4a (added new things and bug fixes)
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

<<<<<<< HEAD
                curr->magic = 0;
=======
                curr->create = 0;
>>>>>>> f7fec4a (added new things and bug fixes)
                return (void *)((uint8_t *)curr + sizeof(struct block_header));
            }

            struct block_header *split = (struct block_header *)((uint8_t *)curr + needed);
<<<<<<< HEAD
            split->magic = KHEAP_MAGIC;
=======
            split->create = KHEAP_CREATE;
>>>>>>> f7fec4a (added new things and bug fixes)
            split->size = remaining;
            split->next = curr->next;

            if (prev)
                prev->next = split;
            else
                free_list = split;

<<<<<<< HEAD
            curr->magic = 0;
=======
            curr->size = needed;
            curr->create = 0;
>>>>>>> f7fec4a (added new things and bug fixes)
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
<<<<<<< HEAD

    header->magic = KHEAP_MAGIC;
    header->size = 0;
=======
    if (header->create != 0 || header->size == 0)
        return;

    header->create = KHEAP_CREATE;
>>>>>>> f7fec4a (added new things and bug fixes)

    struct block_header *prev = NULL;
    struct block_header *curr = free_list;

    while (curr && (uint32_t)curr < (uint32_t)header)
    {
        prev = curr;
        curr = curr->next;
    }

    header->next = curr;

<<<<<<< HEAD
=======
    /* coalesce with previous block if adjacent */
>>>>>>> f7fec4a (added new things and bug fixes)
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

<<<<<<< HEAD
=======
    /* coalesce with next block if adjacent */
>>>>>>> f7fec4a (added new things and bug fixes)
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

#include "vfs.h"
#include "kheap.h"
#include "vga.h"

extern size_t strlen(const char *s);
extern int strcmp(const char *a, const char *b);
extern char *strcpy(char *dst, const char *src);

static struct dentry root_dentry;
static struct inode root_inode;
static uint32_t next_inode_id = 1;

void vfs_init(void)
{
    root_dentry.name[0] = '/';
    root_dentry.name[1] = '\0';
    root_dentry.parent = NULL;
    root_dentry.children = NULL;
    root_dentry.next = NULL;

    root_inode.id = next_inode_id++;
    root_inode.name[0] = '/';
    root_inode.name[1] = '\0';
    root_inode.type = INODE_DIR;
    root_inode.size = 0;
    root_inode.data = NULL;
    root_inode.refcount = 1;

    root_dentry.inode = &root_inode;
}

struct dentry *vfs_get_root(void)
{
    return &root_dentry;
}

struct dentry *vfs_lookup(struct dentry *parent, const char *name)
{
    if (!parent || !name)
        return NULL;

    struct dentry *child = parent->children;
    while (child)
    {
        if (strcmp(child->name, name) == 0)
            return child;
        child = child->next;
    }
    return NULL;
}

static struct dentry *dentry_alloc(void)
{
    return (struct dentry *)kmalloc(sizeof(struct dentry));
}

static struct inode *inode_alloc(void)
{
    return (struct inode *)kmalloc(sizeof(struct inode));
}

static void dentry_link(struct dentry *parent, struct dentry *child)
{
    child->parent = parent;
    child->next = parent->children;
    parent->children = child;
}

struct dentry *vfs_mkdir(struct dentry *parent, const char *name)
{
    if (!parent || !name)
        return NULL;
    if (strlen(name) == 0 || strlen(name) >= VFS_NAME)
        return NULL;
    if (vfs_lookup(parent, name))
        return NULL;

    struct dentry *dent = dentry_alloc();
    if (!dent)
        return NULL;

    struct inode *inode = inode_alloc();
    if (!inode)
    {
        kfree(dent);
        return NULL;
    }

    strcpy(dent->name, name);
    dent->children = NULL;
    dent->inode = inode;

    inode->id = next_inode_id++;
    strcpy(inode->name, name);
    inode->type = INODE_DIR;
    inode->size = 0;
    inode->data = NULL;
    inode->refcount = 1;

    dentry_link(parent, dent);
    return dent;
}

struct dentry *vfs_create(struct dentry *parent, const char *name)
{
    if (!parent || !name)
        return NULL;
    if (strlen(name) == 0 || strlen(name) >= VFS_NAME)
        return NULL;
    if (vfs_lookup(parent, name))
        return NULL;

    struct dentry *dent = dentry_alloc();
    if (!dent)
        return NULL;

    struct inode *inode = inode_alloc();
    if (!inode)
    {
        kfree(dent);
        return NULL;
    }

    strcpy(dent->name, name);
    dent->children = NULL;
    dent->inode = inode;

    inode->id = next_inode_id++;
    strcpy(inode->name, name);
    inode->type = INODE_FILE;
    inode->size = 0;
    inode->data = NULL;
    inode->refcount = 1;

    dentry_link(parent, dent);
    return dent;
}

int vfs_delete(struct dentry *dent)
{
    if (!dent || dent == &root_dentry)
        return -1;
    if (dent->children)
        return -1;

    struct dentry *prev = NULL;
    struct dentry *curr = dent->parent->children;
    while (curr)
    {
        if (curr == dent)
        {
            if (prev)
                prev->next = curr->next;
            else
                dent->parent->children = curr->next;
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    if (dent->inode->data)
        kfree(dent->inode->data);
    kfree(dent->inode);
    kfree(dent);
    return 0;
}

int vfs_write(struct inode *inode, const char *data, uint32_t len)
{
    if (!inode || inode->type != INODE_FILE)
        return -1;
    if (len == 0)
    {
        if (inode->data)
            kfree(inode->data);
        inode->data = NULL;
        inode->size = 0;
        return 0;
    }

    char *new_data = (char *)kmalloc(len);
    if (!new_data)
        return -1;

    for (uint32_t i = 0; i < len; i++)
        new_data[i] = data[i];

    if (inode->data)
        kfree(inode->data);

    inode->data = new_data;
    inode->size = len;
    return (int)len;
}

int vfs_read(struct inode *inode, char *buf, uint32_t max_len)
{
    if (!inode || inode->type != INODE_FILE || !buf)
        return -1;

    uint32_t to_copy = inode->size < max_len ? inode->size : max_len;
    for (uint32_t i = 0; i < to_copy; i++)
        buf[i] = inode->data[i];
    return (int)to_copy;
}

int vfs_append(struct inode *inode, const char *data, uint32_t len)
{
    if (!inode || inode->type != INODE_FILE)
        return -1;
    if (len == 0)
        return 0;

    char *new_data = (char *)kmalloc(inode->size + len);
    if (!new_data)
        return -1;

    for (uint32_t i = 0; i < inode->size; i++)
        new_data[i] = inode->data[i];
    for (uint32_t i = 0; i < len; i++)
        new_data[inode->size + i] = data[i];

    if (inode->data)
        kfree(inode->data);

    inode->data = new_data;
    inode->size += len;
    return (int)len;
}

int vfs_list(struct dentry *dir)
{
    if (!dir)
        return -1;

    int found = 0;
    struct dentry *child = dir->children;
    while (child)
    {
        if (child->inode->type == INODE_DIR)
            vga_printf("  %s/\n", child->name);
        else
            vga_printf("  %s  (%d bytes)\n", child->name, child->inode->size);
        found = 1;
        child = child->next;
    }
    if (!found)
        vga_puts("  (empty)\n");
    return found;
}

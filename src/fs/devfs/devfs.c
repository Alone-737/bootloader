/* /dev: null discards everything, zero reads back zeros. */

#include "devfs.h"
#include "vfs.h"

static int null_read(struct inode *inode, char *buf, uint32_t max_len)
{
    (void)inode;
    (void)buf;
    (void)max_len;
    return 0;
}

static int null_write(struct inode *inode, const char *data, uint32_t len)
{
    (void)inode;
    (void)data;
    return (int)len;
}

static int zero_read(struct inode *inode, char *buf, uint32_t max_len)
{
    (void)inode;
    for (uint32_t i = 0; i < max_len; i++)
        buf[i] = 0;
    return (int)max_len;
}

static struct inode_ops null_ops = {null_read, null_write};
static struct inode_ops zero_ops = {zero_read, NULL};

void devfs_init(void)
{
    struct dentry *dir = vfs_mount("/dev", "devfs");
    if (!dir)
        return;

    struct dentry *f = vfs_create(dir, "null");
    if (f)
        f->inode->ops = &null_ops;

    f = vfs_create(dir, "zero");
    if (f)
        f->inode->ops = &zero_ops;
}

#include "fs.h"
#include "vfs.h"
#include "vga.h"

static int str_eq(const char *a, const char *b)
{
    int i = 0;
    while (a[i] && b[i])
    {
        if (a[i] != b[i])
            return 0;
        i++;
    }
    return a[i] == b[i];
}

static int str_len(const char *s)
{
    int i = 0;
    while (s[i])
        i++;
    return i;
}

static void str_cpy(char *dst, const char *src)
{
    int i = 0;
    while (src[i])
    {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

int fs_create(const char *name)
{
    if (!name || str_len(name) == 0 || str_len(name) >= FS_NAME)
        return -1;

    struct dentry *root = vfs_get_root();
    if (vfs_lookup(root, name))
        return -1;

    struct dentry *dent = vfs_create(root, name);
    return dent ? 0 : -1;
}

int fs_mkdir(const char *name)
{
    if (!name || str_len(name) == 0 || str_len(name) >= FS_NAME)
        return -1;

    struct dentry *root = vfs_get_root();
    if (vfs_lookup(root, name))
        return -1;

    struct dentry *dent = vfs_mkdir(root, name);
    return dent ? 0 : -1;
}

int fs_delete(const char *name)
{
    struct dentry *root = vfs_get_root();
    struct dentry *dent = vfs_lookup(root, name);
    if (!dent)
        return -1;
    return vfs_delete(dent);
}

int fs_rename(const char *oldname, const char *newname)
{
    if (!oldname || !newname)
        return -1;
    if (str_len(newname) == 0 || str_len(newname) >= FS_NAME)
        return -1;
    if (str_eq(oldname, newname))
        return 0;

    struct dentry *root = vfs_get_root();
    struct dentry *dent = vfs_lookup(root, oldname);
    if (!dent)
        return -1;
    if (vfs_lookup(root, newname))
        return -1;

    str_cpy(dent->name, newname);
    str_cpy(dent->inode->name, newname);
    return 0;
}

int fs_copy(const char *src, const char *dst)
{
    struct dentry *root = vfs_get_root();
    struct dentry *src_dent = vfs_lookup(root, src);
    if (!src_dent || src_dent->inode->type != INODE_FILE)
        return -1;

    if (fs_create(dst) < 0)
        return -1;

    return fs_write(dst, src_dent->inode->data, src_dent->inode->size);
}

int fs_write(const char *name, const char *data, int len)
{
    struct dentry *root = vfs_get_root();
    struct dentry *dent = vfs_lookup(root, name);
    if (!dent || dent->inode->type != INODE_FILE)
        return -1;
    return vfs_write(dent->inode, data, (uint32_t)len);
}

int fs_append(const char *name, const char *data, int len)
{
    struct dentry *root = vfs_get_root();
    struct dentry *dent = vfs_lookup(root, name);
    if (!dent || dent->inode->type != INODE_FILE)
        return -1;
    return vfs_append(dent->inode, data, (uint32_t)len);
}

int fs_read(const char *name, char *buf, int max_len)
{
    struct dentry *root = vfs_get_root();
    struct dentry *dent = vfs_lookup(root, name);
    if (!dent || dent->inode->type != INODE_FILE)
        return -1;
    return vfs_read(dent->inode, buf, (uint32_t)max_len);
}

int fs_get_size(const char *name)
{
    struct dentry *root = vfs_get_root();
    struct dentry *dent = vfs_lookup(root, name);
    if (!dent)
        return -1;
    return (int)dent->inode->size;
}

int fs_exists(const char *name)
{
    struct dentry *root = vfs_get_root();
    return vfs_lookup(root, name) != NULL;
}

void fs_list(void)
{
    vfs_list(vfs_get_root());
}

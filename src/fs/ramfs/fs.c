#include "fs.h"
#include "vfs.h"
#include "vga.h"

extern size_t strlen(const char *s);
extern int strcmp(const char *a, const char *b);
extern char *strcpy(char *dst, const char *src);
extern char *strrchr(const char *s, int c);

static struct dentry *fs_resolve_parent(const char *path, const char **leaf)
{
    struct dentry *root = vfs_get_root();
    const char *slash = strrchr(path, '/');
    if (!slash)
    {
        *leaf = path;
        return root;
    }

    char parent_path[FS_NAME * 4];
    int plen = (int)(slash - path);
    if (plen == 0)
    {
        *leaf = slash + 1;
        return root;
    }
    if (plen >= (int)sizeof(parent_path))
        return NULL;

    for (int i = 0; i < plen; i++)
        parent_path[i] = path[i];
    parent_path[plen] = '\0';

    *leaf = slash + 1;
    return vfs_lookup_path(root, parent_path);
}

int fs_create(const char *name)
{
    if (!name || strlen(name) == 0)
        return -1;

    const char *leaf;
    struct dentry *parent = fs_resolve_parent(name, &leaf);
    if (!parent)
        return -1;
    if (strlen(leaf) == 0 || strlen(leaf) >= FS_NAME)
        return -1;
    if (vfs_lookup(parent, leaf))
        return -1;

    struct dentry *dent = vfs_create(parent, leaf);
    return dent ? 0 : -1;
}

int fs_mkdir(const char *name)
{
    if (!name || strlen(name) == 0)
        return -1;

    const char *leaf;
    struct dentry *parent = fs_resolve_parent(name, &leaf);
    if (!parent)
        return -1;
    if (strlen(leaf) == 0 || strlen(leaf) >= FS_NAME)
        return -1;
    if (vfs_lookup(parent, leaf))
        return -1;

    struct dentry *dent = vfs_mkdir(parent, leaf);
    return dent ? 0 : -1;
}

int fs_delete(const char *name)
{
    struct dentry *root = vfs_get_root();
    struct dentry *dent = vfs_lookup_path(root, name);
    if (!dent)
        return -1;
    return vfs_delete(dent);
}

int fs_rename(const char *oldname, const char *newname)
{
    if (!oldname || !newname)
        return -1;
    if (strlen(newname) == 0 || strlen(newname) >= FS_NAME)
        return -1;
    if (strcmp(oldname, newname) == 0)
        return 0;

    struct dentry *root = vfs_get_root();
    struct dentry *dent = vfs_lookup_path(root, oldname);
    if (!dent)
        return -1;

    const char *new_leaf;
    struct dentry *new_parent = fs_resolve_parent(newname, &new_leaf);
    if (!new_parent)
        return -1;
    if (strlen(new_leaf) == 0 || strlen(new_leaf) >= FS_NAME)
        return -1;
    if (vfs_lookup(new_parent, new_leaf))
        return -1;

    if (new_parent != dent->parent)
    {
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
        dent->next = new_parent->children;
        new_parent->children = dent;
        dent->parent = new_parent;
    }

    strcpy(dent->name, new_leaf);
    strcpy(dent->inode->name, new_leaf);
    return 0;
}

int fs_copy(const char *src, const char *dst)
{
    struct dentry *root = vfs_get_root();
    struct dentry *src_dent = vfs_lookup_path(root, src);
    if (!src_dent || src_dent->inode->type != INODE_FILE)
        return -1;

    if (fs_create(dst) < 0)
        return -1;

    return fs_write(dst, src_dent->inode->data, src_dent->inode->size);
}

int fs_write(const char *name, const char *data, int len)
{
    struct dentry *root = vfs_get_root();
    struct dentry *dent = vfs_lookup_path(root, name);
    if (!dent || dent->inode->type != INODE_FILE)
        return -1;
    return vfs_write(dent->inode, data, (uint32_t)len);
}

int fs_append(const char *name, const char *data, int len)
{
    struct dentry *root = vfs_get_root();
    struct dentry *dent = vfs_lookup_path(root, name);
    if (!dent || dent->inode->type != INODE_FILE)
        return -1;
    return vfs_append(dent->inode, data, (uint32_t)len);
}

int fs_read(const char *name, char *buf, int max_len)
{
    struct dentry *root = vfs_get_root();
    struct dentry *dent = vfs_lookup_path(root, name);
    if (!dent || dent->inode->type != INODE_FILE)
        return -1;
    return vfs_read(dent->inode, buf, (uint32_t)max_len);
}

int fs_get_size(const char *name)
{
    struct dentry *root = vfs_get_root();
    struct dentry *dent = vfs_lookup_path(root, name);
    if (!dent)
        return -1;
    return (int)dent->inode->size;
}

int fs_exists(const char *name)
{
    struct dentry *root = vfs_get_root();
    return vfs_lookup_path(root, name) != NULL;
}

void fs_list(void)
{
    vfs_list(vfs_get_root());
}

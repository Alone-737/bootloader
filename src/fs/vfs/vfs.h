#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

#define VFS_NAME 32

enum inode_type
{
    INODE_FILE,
    INODE_DIR
};

struct inode
{
    uint32_t id;
    char name[VFS_NAME];
    enum inode_type type;
    uint32_t size;
    char *data;
    uint32_t refcount;
};

struct dentry
{
    char name[VFS_NAME];
    struct inode *inode;
    struct dentry *parent;
    struct dentry *children;
    struct dentry *next;
};

void vfs_init(void);
struct dentry *vfs_get_root(void);
struct dentry *vfs_lookup(struct dentry *parent, const char *name);
struct dentry *vfs_lookup_path(struct dentry *start, const char *path);
struct dentry *vfs_mkdir(struct dentry *parent, const char *name);
struct dentry *vfs_create(struct dentry *parent, const char *name);
int vfs_delete(struct dentry *dent);
int vfs_write(struct inode *inode, const char *data, uint32_t len);
int vfs_read(struct inode *inode, char *buf, uint32_t max_len);
int vfs_append(struct inode *inode, const char *data, uint32_t len);
int vfs_list(struct dentry *dir);

#endif

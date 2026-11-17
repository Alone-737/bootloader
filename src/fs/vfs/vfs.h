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

struct inode;

/* A filesystem backed by generated content (procfs, devfs) fills these in.
   NULL ops means a normal tree inode whose data lives in inode->data. */
struct inode_ops
{
    int (*read)(struct inode *inode, char *buf, uint32_t max_len);
    int (*write)(struct inode *inode, const char *data, uint32_t len);
};

struct inode
{
    uint32_t id;
    char name[VFS_NAME];
    enum inode_type type;
    uint32_t size;
    char *data;
    uint32_t refcount;
    struct inode_ops *ops;
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
struct dentry *vfs_get_cwd(void);
void vfs_set_cwd(struct dentry *d);
struct dentry *vfs_lookup(struct dentry *parent, const char *name);
struct dentry *vfs_lookup_path(struct dentry *start, const char *path);
struct dentry *vfs_mkdir(struct dentry *parent, const char *name);
struct dentry *vfs_create(struct dentry *parent, const char *name);
int vfs_delete(struct dentry *dent);
int vfs_write(struct inode *inode, const char *data, uint32_t len);
int vfs_read(struct inode *inode, char *buf, uint32_t max_len);
int vfs_append(struct inode *inode, const char *data, uint32_t len);
int vfs_list(struct dentry *dir);

/* Mounts an empty directory at path and records it for vfs_list_mounts. */
struct dentry *vfs_mount(const char *path, const char *fstype);
void vfs_list_mounts(void);

#endif

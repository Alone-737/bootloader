/* /proc: files whose content is generated on each read. */

#include "procfs.h"
#include "vfs.h"
#include "timer.h"
#include "string.h"

extern int snprintf(char *buf, size_t sz, const char *fmt, ...);

static int version_read(struct inode *inode, char *buf, uint32_t max_len)
{
    (void)inode;
    static const char v[] = "Abyss 1.0 (Solace kernel)\n";
    uint32_t n = sizeof(v) - 1 < max_len ? sizeof(v) - 1 : max_len;
    for (uint32_t i = 0; i < n; i++)
        buf[i] = v[i];
    return (int)n;
}

static int uptime_read(struct inode *inode, char *buf, uint32_t max_len)
{
    (void)inode;
    uint32_t ticks = timer_get_ticks(); /* 100 Hz */
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "%u.%02u\n", ticks / 100, ticks % 100);
    uint32_t n = strlen(tmp) < max_len ? strlen(tmp) : max_len;
    for (uint32_t i = 0; i < n; i++)
        buf[i] = tmp[i];
    return (int)n;
}

static struct inode_ops version_ops = {version_read, NULL};
static struct inode_ops uptime_ops = {uptime_read, NULL};

void procfs_init(void)
{
    struct dentry *dir = vfs_mount("/proc", "procfs");
    if (!dir)
        return;

    struct dentry *f = vfs_create(dir, "version");
    if (f)
        f->inode->ops = &version_ops;

    f = vfs_create(dir, "uptime");
    if (f)
        f->inode->ops = &uptime_ops;
}

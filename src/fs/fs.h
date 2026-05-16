#ifndef FS_H
#define FS_H

#define FS_FILES 32
#define FS_NAME 32
#define FS_SIZE 4096

int fs_create(const char *name);
int fs_mkdir(const char *name);
int fs_delete(const char *name);
int fs_rename(const char *oldname, const char *newname);
int fs_copy(const char *src, const char *dst);
int fs_write(const char *name, const char *data, int len);
int fs_append(const char *name, const char *data, int len);
int fs_read(const char *name, char *buf, int max_len);
int fs_get_size(const char *name);
int fs_exists(const char *name);
void fs_list(void);

#endif

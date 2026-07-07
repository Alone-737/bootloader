#include "romfs.h"
#include "vfs.h"

extern size_t strlen(const char *s);

struct rom_file
{
    const char *name;
    const char *data;
    uint32_t size;
};

static const char str_help[] =
    "help    show this message\n"
    "clear   clear screen\n"
    "echo    print text\n"
    "uname   print system info\n"
    "whoami  print current user\n"
    "history show command history\n"
    "ls      list files\n"
    "mkdir   create directory\n"
    "pwd     print working directory\n"
    "date    show RTC date/time\n"
    "cat     show file contents\n"
    "rm      delete file\n"
    "mv      rename file\n"
    "cp      copy file\n"
    "grep    search file for pattern\n"
    "wc      count lines/words/chars\n"
    "touch   create empty file\n"
    "edit    open text editor\n"
    "snake   play Snake game\n"
    "doom    play Doom (WIP)\n"
    "!N      repeat command N from history\n"
    "|       pipe output to command\n"
    ">       redirect output to file\n"
    ">>      append output to file\n"
    "<       read input from file\n";

static const char str_version[] =
    "my loneliness is killing me v1.0\n"
    "x86-32 protected mode\n"
    "256MB RAM, 256KB heap\n"
    "built: " __DATE__ " " __TIME__ "\n";

static const char str_motd[] =
    "Welcome to my loneliness is killing me!\n"
    "A hobby x86 OS written from scratch.\n"
    "Type 'help' for available commands.\n"
    "Type 'cat help.txt' for details.\n"
    "Type 'snake' to play a game.\n"
    "Type 'edit' to edit a file.\n";

static const struct rom_file rom_files[] = {
    {"help.txt",    str_help,    sizeof(str_help) - 1},
    {"version.txt", str_version, sizeof(str_version) - 1},
    {"motd.txt",    str_motd,    sizeof(str_motd) - 1},
};

static const int rom_file_count = sizeof(rom_files) / sizeof(rom_files[0]);

void romfs_init(void)
{
    struct dentry *root = vfs_get_root();

    for (int i = 0; i < rom_file_count; i++)
    {
        if (vfs_lookup(root, rom_files[i].name))
            continue;

        struct dentry *dent = vfs_create(root, rom_files[i].name);
        if (!dent)
            continue;

        vfs_write(dent->inode, rom_files[i].data, rom_files[i].size);
    }
}

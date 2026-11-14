#include "shell.h"
#include "history.h"
#include "vga.h"
#include "fs.h"
#include "io.h"
#include "editor.h"
#include "keyboard.h"
#include "snake.h"
#include "doom.h"
#include "syscall.h"
#include "timer.h"
#include "net.h"
#include "paging.h"

extern char *strstr(const char *hay, const char *needle);
extern int atoi(const char *s);

static const char current_user[4] = "root";

static int cap_on = 0;
static char cap_buf[4096];
static int cap_len = 0;

static char pipe_in[4096];
static int pipe_in_len = 0;
static int pipe_in_active = 0;

static void putc(char c)
{
    if (cap_on)
    {
        if (cap_len < (int)sizeof(cap_buf) - 1)
            cap_buf[cap_len++] = c;
    }
    else
    {
        vga_putchar(c);
    }
}

static void puts(const char *s)
{
    if (!s)
        return;
    for (int i = 0; s[i]; i++)
        putc(s[i]);
}

static int str_find(const char *s, const char *sub)
{
    const char *p = strstr(s, sub);
    return p ? (int)(p - s) : -1;
}

static void str_cpy_n(char *dst, const char *src, int n)
{
    int i = 0;
    while (i < n - 1 && src[i])
    {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

static int parse_args(const char *s, char *a1, char *a2)
{
    while (*s == ' ')
        s++;
    int i = 0;
    while (s[i] && s[i] != ' ' && i < 63)
    {
        a1[i] = s[i];
        i++;
    }
    a1[i] = '\0';
    if (s[i] == '\0')
        return 1;
    while (s[i] == ' ')
        i++;
    if (s[i] == '\0')
        return 1;
    int j = 0;
    while (s[i] && s[i] != ' ' && j < 63)
    {
        a2[j] = s[i];
        i++;
        j++;
    }
    a2[j] = '\0';
    return 2;
}

static void print_prompt(void)
{
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("root@solace$ ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

void shell_banner(void)
{
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("Solace 1.0 - a tiny x86 OS\n");
    vga_puts("Type 'help' for commands.\n");
    vga_puts("Built-in files: cat help.txt, version.txt, motd.txt\n\n");
}

static signed char get_rtc_register(int reg)
{
    outb(0x70, reg);
    return inb(0x71);
}

static int bcd_to_bin(int bcd)
{
    return ((bcd / 16) * 10) + (bcd & 0xf);
}

static int is_update_in_progress(void)
{
    outb(0x70, 0x0A);
    return (inb(0x71) & 0x80);
}

static void cmd_date(const char *args)
{
    (void)args;
    while (is_update_in_progress())
        ;

    signed char second = get_rtc_register(0x00);
    signed char minute = get_rtc_register(0x02);
    signed char hour = get_rtc_register(0x04);
    signed char day = get_rtc_register(0x07);
    signed char month = get_rtc_register(0x08);
    signed char year = get_rtc_register(0x09);
    signed char statusB = get_rtc_register(0x0B);

    if (!(statusB & 0x04))
    {
        second = bcd_to_bin(second);
        minute = bcd_to_bin(minute);
        hour = (bcd_to_bin(hour & 0x7F) & 0x7F) | (hour & 0x80);
        day = bcd_to_bin(day);
        month = bcd_to_bin(month);
        year = bcd_to_bin(year);
    }

    if (!(statusB & 0x02) && (hour & 0x80))
    {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    vga_printf("Current date: 20%02d-%02d-%02d %02d:%02d:%02d\n", year, month, day, hour, minute, second);
}

static void cmd_cat(const char *args)
{
    char fname[64];
    char arg2[64];
    int i;
    for (i = 0; i < 64; i++) arg2[i] = 0;
    parse_args(args, fname, arg2);
    if (fname[0] == '\0')
    {
        puts("cat: missing operand\n");
        return;
    }
    if (!fs_exists(fname))
    {
        puts("cat: ");
        puts(fname);
        puts(": No such file\n");
        return;
    }
    char buf[FS_SIZE + 1];
    int n = fs_read(fname, buf, FS_SIZE);
    if (n > 0)
    {
        buf[n] = '\0';
        puts(buf);
    }
    if (n > 0 && buf[n - 1] != '\n')
        putc('\n');
}

static void cmd_rm(const char *args)
{
    char fname[64];
    char arg2[64];
    int i;
    for (i = 0; i < 64; i++) arg2[i] = 0;
    parse_args(args, fname, arg2);
    if (fname[0] == '\0')
    {
        puts("rm: missing operand\n");
        return;
    }
    if (fs_delete(fname) == 0)
        puts("rm: removed '");
    else
        puts("rm: cannot remove '");
    puts(fname);
    puts("'\n");
}

static void cmd_mv(const char *args)
{
    char src[64], dst[64];
    int n = parse_args(args, src, dst);
    if (n < 2)
    {
        puts("mv: missing operand\n");
        return;
    }
    if (!fs_exists(src))
    {
        puts("mv: ");
        puts(src);
        puts(": No such file\n");
        return;
    }
    if (fs_rename(src, dst) == 0)
    {
        puts("mv: renamed '");
        puts(src);
        puts("' -> '");
        puts(dst);
        puts("'\n");
    }
    else
    {
        puts("mv: error renaming '");
        puts(src);
        puts("'\n");
    }
}

static void cmd_cp(const char *args)
{
    char src[64], dst[64];
    int n = parse_args(args, src, dst);
    if (n < 2)
    {
        puts("cp: missing operand\n");
        return;
    }
    if (!fs_exists(src))
    {
        puts("cp: ");
        puts(src);
        puts(": No such file\n");
        return;
    }
    if (fs_copy(src, dst) == 0)
    {
        puts("cp: '");
        puts(src);
        puts("' -> '");
        puts(dst);
        puts("'\n");
    }
    else
    {
        puts("cp: error copying '");
        puts(src);
        puts("'\n");
    }
}

static void cmd_grep(const char *args)
{
    char pattern[64], fname[64];
    int n = parse_args(args, pattern, fname);

    if (pattern[0] == '\0')
    {
        puts("grep: missing pattern\n");
        return;
    }

    char buf[FS_SIZE + 1];
    int buf_len = 0;

    if (n >= 2 && fname[0] != '\0')
    {
        if (!fs_exists(fname))
        {
            puts("grep: ");
            puts(fname);
            puts(": No such file\n");
            return;
        }
        buf_len = fs_read(fname, buf, FS_SIZE);
        if (buf_len < 0)
        {
            puts("grep: ");
            puts(fname);
            puts(": not a file\n");
            return;
        }
        buf[buf_len] = '\0';
    }
    else if (pipe_in_active)
    {
        buf_len = pipe_in_len;
        for (int i = 0; i < buf_len; i++)
            buf[i] = pipe_in[i];
        buf[buf_len] = '\0';
    }
    else
    {
        puts("grep: no input\n");
        return;
    }

    int line_start = 0;
    for (int i = 0; i <= buf_len; i++)
    {
        if (buf[i] == '\n' || i == buf_len)
        {
            char saved = buf[i];
            buf[i] = '\0';
            if (strstr(&buf[line_start], pattern))
            {
                puts(&buf[line_start]);
                putc('\n');
            }
            buf[i] = saved;
            line_start = i + 1;
        }
    }
}

static void cmd_wc(const char *args)
{
    char fname[64];
    char arg2[64];
    int i;
    for (i = 0; i < 64; i++) arg2[i] = 0;
    int n = parse_args(args, fname, arg2);

    char buf[FS_SIZE + 1];
    int buf_len = 0;

    if (n >= 1 && fname[0] != '\0')
    {
        if (!fs_exists(fname))
        {
            puts("wc: ");
            puts(fname);
            puts(": No such file\n");
            return;
        }
        buf_len = fs_read(fname, buf, FS_SIZE);
        if (buf_len < 0)
        {
            puts("wc: ");
            puts(fname);
            puts(": not a file\n");
            return;
        }
        buf[buf_len] = '\0';
    }
    else if (pipe_in_active)
    {
        buf_len = pipe_in_len;
        for (int i = 0; i < buf_len; i++)
            buf[i] = pipe_in[i];
        buf[buf_len] = '\0';
    }
    else
    {
        puts("wc: no input\n");
        return;
    }

    int lines = 0, words = 0, chars = buf_len;
    int in_word = 0;
    for (int i = 0; i < buf_len; i++)
    {
        if (buf[i] == '\n')
            lines++;
        if (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n')
        {
            in_word = 0;
        }
        else if (!in_word)
        {
            in_word = 1;
            words++;
        }
    }
    vga_printf("%d  %d  %d", lines, words, chars);
    if (fname[0] != '\0')
    {
        putc(' ');
        puts(fname);
    }
    putc('\n');
}

static void cmd_snake(const char *args)
{
    (void)args;
    snake_run();
}

static void cmd_doom(const char *args)
{
    (void)args;
    /* Switch to VGA mode 13h (320x200) and launch Doom.
     * This call does not return; reboot to exit. */
    doom_run();
}

static int parse_ip(const char *s, uint8_t out[4])
{
    for (int i = 0; i < 4; i++)
    {
        int v = 0, digits = 0;
        while (*s >= '0' && *s <= '9' && digits < 3)
        {
            v = v * 10 + (*s++ - '0');
            digits++;
        }
        if (digits == 0 || v > 255)
            return 0;
        out[i] = (uint8_t)v;
        if (i < 3)
        {
            if (*s != '.')
                return 0;
            s++;
        }
    }
    return *s == '\0';
}

static void cmd_ping(const char *args)
{
    char a1[64], a2[64];
    int n = parse_args(args, a1, a2);
    uint8_t ip[4] = {10, 0, 2, 2};  /* slirp gateway */

    if (n >= 1 && a1[0] != '\0' && !parse_ip(a1, ip))
    {
        vga_puts("usage: ping [a.b.c.d] [count]\n");
        return;
    }

    int count = 4;
    if (n >= 2)
    {
        count = atoi(a2);
        if (count < 1)
            count = 1;
        if (count > 20)
            count = 20;
    }

    net_ping(ip, count);
}

static void cmd_help(const char *args)
{
    (void)args;
    vga_puts("help  clear  echo  uname  whoami  history  ls  mkdir  pwd  date\n");
    vga_puts("cat  rm  mv  cp  grep  wc  touch  snake  doom\n");
    vga_puts("reboot  sleep  hexdump  ping  ring3test\n");
    vga_puts("!N to run command N from history\n");
    vga_puts("Piping & redirection: |  >  >>  <\n");
}

static void cmd_clear(const char *args)
{
    (void)args;
    vga_clear();
}

static void cmd_echo(const char *args)
{
    if (args[0] == '\0')
    {
        putc('\n');
    }
    else
    {
        puts(args);
        putc('\n');
    }
}

static void cmd_uname(const char *args)
{
    (void)args;
    vga_puts("Solace 1.0 i386\n");
}

static void cmd_whoami(const char *args)
{
    (void)args;
    vga_printf("%s\n", current_user);
}

uint32_t ring3_return_esp = 0;
uint32_t ring3_return_ebp = 0;

/* shortcut: copied byte-for-byte to USER_BASE, so it must stay self-contained
   (own stack + int 0x80 only, no absolute references). */
__attribute__((section(".usertext"), used, noinline))
static void user_test_stub(void)
{
    char c = 'R';
    __asm__ volatile("int $0x80" : : "a"(SYS_WRITE), "b"(&c), "c"(1) : "memory");
    __asm__ volatile("int $0x80" : : "a"(SYS_EXIT) : "memory");
    for (;;) { }
}

static void cmd_ring3test(const char *args)
{
    (void)args;
    vga_puts("Testing ring-3 + syscall...\n");

    extern void enter_ring3(void *, uint32_t, uint32_t);
    extern char __usertext_start[], __usertext_end[];

    uint32_t size = (uint32_t)(__usertext_end - __usertext_start);
    if (size > USER_LIMIT - USER_BASE)
    {
        vga_puts("ring3test: user stub too large\n");
        return;
    }
    for (uint32_t i = 0; i < size; i++)
        ((uint8_t *)USER_BASE)[i] = ((const uint8_t *)__usertext_start)[i];

    uint32_t eflags;
    __asm__ volatile("pushfl; pop %0" : "=r"(eflags));

    __asm__ volatile(
        "lea 4(%%ebp), %0\n"
        "mov  (%%ebp), %1\n"
        : "=r"(ring3_return_esp), "=r"(ring3_return_ebp)
    );

    enter_ring3((void *)USER_BASE, USER_LIMIT, eflags);
}

static void cmd_history(const char *args)
{
    (void)args;
    int count = history_get_count();
    for (int h = 0; h < count; h++)
    {
        vga_printf("%d  %s\n", h, history_get(h));
    }
}

static void cmd_ls(const char *args)
{
    (void)args;
    fs_list();
}

static void cmd_pwd(const char *args)
{
    (void)args;
    puts("/\n");
}

static void cmd_touch(const char *args)
{
    if (args[0] == '\0')
    {
        puts("touch: missing operand\n");
        return;
    }
    char fname[64];
    char arg2[64];
    for (int i = 0; i < 64; i++) arg2[i] = 0;
    parse_args(args, fname, arg2);
    if (fname[0] == '\0')
    {
        puts("touch: missing operand\n");
    }
    else if (fs_exists(fname))
    {
        puts("touch: file '");
        puts(fname);
        puts("' already exists\n");
    }
    else if (fs_create(fname) == 0)
    {
        puts("touch: created '");
        puts(fname);
        puts("'\n");
    }
    else
    {
        puts("touch: failed to create '");
        puts(fname);
        puts("'\n");
    }
}

static void cmd_reboot(const char *args)
{
    (void)args;
    puts("rebooting...\n");
    outb(0x64, 0xFE);
    puts("reboot failed\n");
}

static void cmd_sleep(const char *args)
{
    int seconds = 0;
    while (*args >= '0' && *args <= '9')
    {
        seconds = seconds * 10 + (*args - '0');
        args++;
    }
    if (seconds <= 0)
    {
        puts("sleep: usage: sleep <seconds>\n");
        return;
    }
    uint32_t start = timer_get_ticks();
    uint32_t target = start + (uint32_t)seconds * (uint32_t)TIMER_FREQ;
    while (timer_get_ticks() < target)
        ;
    puts("slept\n");
}

static void cmd_hexdump(const char *args)
{
    char fname[64];
    char arg2[64];
    for (int i = 0; i < 64; i++) arg2[i] = 0;
    parse_args(args, fname, arg2);
    if (fname[0] == '\0')
    {
        puts("hexdump: missing operand\n");
        return;
    }
    if (!fs_exists(fname))
    {
        puts("hexdump: ");
        puts(fname);
        puts(": No such file\n");
        return;
    }
    char buf[FS_SIZE + 1];
    int n = fs_read(fname, buf, FS_SIZE);
    if (n <= 0)
    {
        puts("hexdump: empty file\n");
        return;
    }
    for (int i = 0; i < n; i += 16)
    {
        vga_printf("%08x  ", i);
        int j;
        for (j = 0; j < 16 && i + j < n; j++)
        {
            vga_printf("%02x ", (unsigned char)buf[i + j]);
        }
        for (; j < 16; j++)
        {
            puts("   ");
        }
        puts("  ");
        for (j = 0; j < 16 && i + j < n; j++)
        {
            char c = buf[i + j];
            putc(c >= 32 && c < 127 ? c : '.');
        }
        putc('\n');
    }
}

static void cmd_mkdir(const char *args)
{
    if (args[0] == '\0')
    {
        puts("mkdir: missing operand\n");
        return;
    }
    char dname[64];
    char arg2[64];
    for (int i = 0; i < 64; i++) arg2[i] = 0;
    parse_args(args, dname, arg2);
    if (dname[0] == '\0')
    {
        puts("mkdir: missing operand\n");
    }
    else if (fs_mkdir(dname) == 0)
    {
        puts("mkdir: created directory '");
        puts(dname);
        puts("'\n");
    }
    else
    {
        puts("mkdir: cannot create directory '");
        puts(dname);
        puts("'\n");
    }
}

static void cmd_edit(const char *args)
{
    if (args[0] == '\0')
    {
        editor_open(NULL);
    }
    else
    {
        char fname[64];
        char arg2[64];
        for (int i = 0; i < 64; i++) arg2[i] = 0;
        parse_args(args, fname, arg2);
        editor_open(fname);
    }
    editor_run();
}

static void exec_cmd(const char *cmd)
{
    if (cmd[0] == '\0')
        return;

    static const struct {
        const char *name;
        void (*handler)(const char *);
    } cmd_table[] = {
        {"help",       cmd_help},
        {"clear",      cmd_clear},
        {"echo",       cmd_echo},
        {"uname",      cmd_uname},
        {"whoami",     cmd_whoami},
        {"ring3test",  cmd_ring3test},
        {"history",    cmd_history},
        {"ls",         cmd_ls},
        {"pwd",        cmd_pwd},
        {"date",       cmd_date},
        {"cat",        cmd_cat},
        {"rm",         cmd_rm},
        {"mv",         cmd_mv},
        {"cp",         cmd_cp},
        {"grep",       cmd_grep},
        {"wc",         cmd_wc},
        {"touch",      cmd_touch},
        {"mkdir",      cmd_mkdir},
        {"reboot",     cmd_reboot},
        {"sleep",      cmd_sleep},
        {"hexdump",    cmd_hexdump},
        {"ping",       cmd_ping},
        {"edit",       cmd_edit},
        {"snake",      cmd_snake},
        {"doom",       cmd_doom},
    };
    int ncmds = sizeof(cmd_table) / sizeof(cmd_table[0]);

    for (int i = 0; i < ncmds; i++)
    {
        int j = 0;
        while (cmd_table[i].name[j] && cmd[j] && cmd_table[i].name[j] == cmd[j])
            j++;

        if (cmd_table[i].name[j] == '\0')
        {
            if (cmd[j] == '\0')
            {
                cmd_table[i].handler("");
                return;
            }
            if (cmd[j] == ' ')
            {
                cmd_table[i].handler(cmd + j + 1);
                return;
            }
        }
    }

    puts("command not found: ");
    puts(cmd);
    putc('\n');
}

void shell_exec(const char *line)
{
    int i = 0;
    while (line[i] == ' ' || line[i] == '\t')
        i++;
    const char *cmd = &line[i];
    if (cmd[0] == '\0')
        return;

    pipe_in_active = 0;

    if (cmd[0] == '!')
    {
        int idx = 0;
        int j = 1;
        while (cmd[j] >= '0' && cmd[j] <= '9')
        {
            idx = idx * 10 + (cmd[j] - '0');
            j++;
        }
        if (j > 1 && cmd[j] == '\0')
        {
            const char *hist = history_get(idx);
            if (hist[0] != '\0')
            {
                vga_puts(hist);
                vga_puts("\n");
                shell_exec(hist);
                return;
            }
            vga_puts("history index out of range\n");
            return;
        }
    }

    int append_pos = str_find(cmd, ">>");
    int pos = str_find(cmd, ">");
    int pipe_pos = str_find(cmd, "|");
    int in_pos = str_find(cmd, "<");

    if (append_pos >= 0)
    {
        char left[256], fname[64], arg2[64];
        int i;
        for (i = 0; i < 64; i++) arg2[i] = 0;
        str_cpy_n(left, cmd, append_pos + 1);
        const char *fn = cmd + append_pos + 2;
        while (*fn == ' ')
            fn++;
        parse_args(fn, fname, arg2);

        cap_on = 1;
        cap_len = 0;
        exec_cmd(left);
        cap_on = 0;
        cap_buf[cap_len] = '\0';

        if (!fs_exists(fname))
            fs_create(fname);
        fs_append(fname, cap_buf, cap_len);
        return;
    }

    if (pos >= 0 && (append_pos < 0 || pos < append_pos))
    {
        char left[256], fname[64], arg2[64];
        int i;
        for (i = 0; i < 64; i++) arg2[i] = 0;
        str_cpy_n(left, cmd, pos + 1);
        const char *fn = cmd + pos + 1;
        while (*fn == ' ')
            fn++;
        parse_args(fn, fname, arg2);

        cap_on = 1;
        cap_len = 0;
        exec_cmd(left);
        cap_on = 0;
        cap_buf[cap_len] = '\0';

        if (!fs_exists(fname))
            fs_create(fname);
        fs_write(fname, cap_buf, cap_len);
        return;
    }

    if (pipe_pos >= 0)
    {
        char left[256], right[256];
        str_cpy_n(left, cmd, pipe_pos + 1);
        const char *r = cmd + pipe_pos + 1;
        str_cpy_n(right, r, 255);

        cap_on = 1;
        cap_len = 0;
        exec_cmd(left);
        cap_on = 0;

        pipe_in_len = cap_len;
        for (int j = 0; j < cap_len; j++)
            pipe_in[j] = cap_buf[j];
        pipe_in_active = 1;

        exec_cmd(right);
        pipe_in_active = 0;
        return;
    }

    if (in_pos >= 0)
    {
        char left[256], fname[64], arg2[64];
        int i;
        for (i = 0; i < 64; i++) arg2[i] = 0;
        str_cpy_n(left, cmd, in_pos + 1);
        const char *fn = cmd + in_pos + 1;
        while (*fn == ' ')
            fn++;
        parse_args(fn, fname, arg2);

        if (!fs_exists(fname))
        {
            vga_puts("input redirection: ");
            vga_puts(fname);
            vga_puts(": No such file\n");
            return;
        }

        pipe_in_len = fs_read(fname, pipe_in, (int)sizeof(pipe_in) - 1);
        if (pipe_in_len < 0)
        {
            vga_puts("input redirection: ");
            vga_puts(fname);
            vga_puts(": not a file\n");
            return;
        }
        pipe_in[pipe_in_len] = '\0';
        pipe_in_active = 1;

        exec_cmd(left);
        pipe_in_active = 0;
        return;
    }

    exec_cmd(cmd);
}

static void shell_erase_chars(int count)
{
    for (int i = 0; i < count; i++)
    {
        vga_putchar('\b');
        vga_putchar(' ');
        vga_putchar('\b');
    }
}

void shell_run(void)
{
    char line[512];
    int len = 0;
    int hist_pos = -1; /* -1 = not navigating history */
    line[0] = '\0';

    print_prompt();

    for (;;)
    {
        unsigned char sc = keyboard_read_raw();

        /* Up arrow: recall previous history entry */
        if (sc == 0x48)
        {
            int count = history_get_count();
            if (count == 0)
                continue;
            if (hist_pos == -1)
                hist_pos = count - 1;
            else if (hist_pos > 0)
                hist_pos--;

            shell_erase_chars(len);

            const char *hist = history_get(hist_pos);
            int hlen = 0;
            while (hist[hlen] && hlen < (int)sizeof(line) - 1)
                hlen++;
            for (int i = 0; i < hlen; i++)
                line[i] = hist[i];
            line[hlen] = '\0';
            len = hlen;

            for (int i = 0; i < hlen; i++)
                vga_putchar(line[i]);
            continue;
        }

        /* Down arrow: go forward in history / clear line */
        if (sc == 0x50)
        {
            if (hist_pos == -1)
                continue;
            if (hist_pos == history_get_count() - 1)
            {
                hist_pos = -1;
                shell_erase_chars(len);
                len = 0;
                line[0] = '\0';
                continue;
            }
            hist_pos++;

            shell_erase_chars(len);

            const char *hist = history_get(hist_pos);
            int hlen = 0;
            while (hist[hlen] && hlen < (int)sizeof(line) - 1)
                hlen++;
            for (int i = 0; i < hlen; i++)
                line[i] = hist[i];
            line[hlen] = '\0';
            len = hlen;

            for (int i = 0; i < hlen; i++)
                vga_putchar(line[i]);
            continue;
        }

        char c = keyboard_translate(sc);
        if (c == 0)
            continue;

        if (c == '\n')
        {
            vga_putchar('\n');
            line[len] = '\0';
            if (len > 0)
            {
                history_add(line);
            }
            hist_pos = -1;
            shell_exec(line);
            len = 0;
            line[0] = '\0';
            print_prompt();
            continue;
        }

        if (c == '\b')
        {
            if (len > 0)
            {
                len--;
                line[len] = '\0';
                vga_putchar('\b');
                vga_putchar(' ');
                vga_putchar('\b');
            }
            hist_pos = -1;
            continue;
        }

        if (len < (int)sizeof(line) - 1)
        {
            line[len++] = c;
            line[len] = '\0';
            vga_putchar(c);
            hist_pos = -1;
        }
    }
}

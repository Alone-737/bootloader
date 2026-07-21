#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "kheap.h"
#include "vga13h.h"

void *malloc(size_t size)  { return kmalloc(size); }
void  free(void *ptr)      { kfree(ptr); }

void *calloc(size_t n, size_t size)
{
    size_t total = n * size;
    void *p = kmalloc(total);
    if (p)
    {
        uint8_t *b = (uint8_t *)p;
        for (size_t i = 0; i < total; i++)
            b[i] = 0;
    }
    return p;
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr)
        return kmalloc(size);
    if (size == 0)
    {
        kfree(ptr);
        return NULL;
    }

    size_t old_size = kmalloc_usable_size(ptr);
    void *p = kmalloc(size);
    if (p)
    {
        size_t copy_size = old_size < size ? old_size : size;
        uint8_t *src = (uint8_t *)ptr;
        uint8_t *dst = (uint8_t *)p;
        for (size_t i = 0; i < copy_size; i++)
            dst[i] = src[i];
        kfree(ptr);
    }
    return p;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < n; i++)
        d[i] = s[i];
    return dst;
}

void *memset(void *dst, int c, size_t n)
{
    uint8_t *d = (uint8_t *)dst;
    for (size_t i = 0; i < n; i++)
        d[i] = (uint8_t)c;
    return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    if (d < s)
        for (size_t i = 0; i < n; i++) d[i] = s[i];
    else
        for (size_t i = n; i > 0; i--) d[i-1] = s[i-1];
    return dst;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const uint8_t *p = (const uint8_t *)a;
    const uint8_t *q = (const uint8_t *)b;
    for (size_t i = 0; i < n; i++)
    {
        if (p[i] < q[i]) return -1;
        if (p[i] > q[i]) return  1;
    }
    return 0;
}

size_t strlen(const char *s)
{
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

char *strcpy(char *dst, const char *src)
{
    char *d = dst;
    while ((*d++ = *src++));
    return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
    size_t i;
    for (i = 0; i < n && src[i]; i++)
        dst[i] = src[i];
    for (; i < n; i++)
        dst[i] = 0;
    return dst;
}

char *strcat(char *dst, const char *src)
{
    char *d = dst;
    while (*d) d++;
    while ((*d++ = *src++));
    return dst;
}

char *strncat(char *dst, const char *src, size_t n)
{
    char *d = dst;
    while (*d) d++;
    while (n-- && *src) *d++ = *src++;
    *d = 0;
    return dst;
}

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n)
{
    while (n-- && *a && *a == *b) { a++; b++; }
    if (n == (size_t)-1) return 0;
    return (unsigned char)*a - (unsigned char)*b;
}

static inline char _to_lower(char c)
{
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

int strcasecmp(const char *a, const char *b)
{
    while (*a && _to_lower(*a) == _to_lower(*b)) { a++; b++; }
    return (unsigned char)_to_lower(*a) - (unsigned char)_to_lower(*b);
}

int strncasecmp(const char *a, const char *b, size_t n)
{
    while (n-- && *a && _to_lower(*a) == _to_lower(*b)) { a++; b++; }
    if (n == (size_t)-1) return 0;
    return (unsigned char)_to_lower(*a) - (unsigned char)_to_lower(*b);
}

char *strchr(const char *s, int c)
{
    while (*s)
    {
        if (*s == (char)c) return (char *)s;
        s++;
    }
    return (c == 0) ? (char *)s : 0;
}

char *strrchr(const char *s, int c)
{
    const char *last = 0;
    while (*s)
    {
        if (*s == (char)c) last = s;
        s++;
    }
    return (char *)last;
}

char *strstr(const char *hay, const char *needle)
{
    if (!*needle) return (char *)hay;
    for (; *hay; hay++)
    {
        const char *h = hay, *n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (!*n) return (char *)hay;
    }
    return 0;
}

int toupper(int c)
{
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}

int tolower(int c)
{
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

int isdigit(int c) { return c >= '0' && c <= '9'; }
int isspace(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
int isalpha(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
int isalnum(int c) { return isalpha(c) || isdigit(c); }
int isupper(int c) { return c >= 'A' && c <= 'Z'; }
int islower(int c) { return c >= 'a' && c <= 'z'; }

int abs(int x)      { return x < 0 ? -x : x; }
double fabs(double x) { return x < 0.0 ? -x : x; }

void exit(int code)
{
    (void)code;
    __asm__ volatile ("cli; hlt");
    for (;;);
}

static void _putchar_shim(char c)
{
    vga13h_putchar_console(c);
}

static int _vsnprintf(char *buf, size_t sz, const char *fmt, va_list ap)
{
    size_t pos = 0;

#define EMIT(c) do { if (buf && pos < sz-1) buf[pos] = (c); pos++; } while(0)

    for (size_t i = 0; fmt[i]; i++)
    {
        if (fmt[i] != '%') { EMIT(fmt[i]); continue; }
        i++;
        int left = 0, zero = 0, width = 0;
        if (fmt[i] == '-') { left = 1; i++; }
        if (fmt[i] == '0') { zero = 1; i++; }
        while (fmt[i] >= '0' && fmt[i] <= '9')
            width = width * 10 + (fmt[i++] - '0');

        char spec = fmt[i];
        char tmp[32];
        int tlen = 0;

        if (spec == 'd' || spec == 'i')
        {
            int v = va_arg(ap, int);
            int neg = (v < 0);
            unsigned int u = neg ? (unsigned int)(-v) : (unsigned int)v;
            if (u == 0) tmp[tlen++] = '0';
            while (u) { tmp[tlen++] = '0' + (u % 10); u /= 10; }
            if (neg) tmp[tlen++] = '-';
            int pad = width - tlen;
            if (!left) while (pad-- > 0) EMIT(zero ? '0' : ' ');
            while (tlen > 0) EMIT(tmp[--tlen]);
            if (left) while (pad-- > 0) EMIT(' ');
        }
        else if (spec == 'u')
        {
            unsigned int v = va_arg(ap, unsigned int);
            if (v == 0) tmp[tlen++] = '0';
            while (v) { tmp[tlen++] = '0' + (v % 10); v /= 10; }
            int pad = width - tlen;
            if (!left) while (pad-- > 0) EMIT(zero ? '0' : ' ');
            while (tlen > 0) EMIT(tmp[--tlen]);
            if (left) while (pad-- > 0) EMIT(' ');
        }
        else if (spec == 'x' || spec == 'X')
        {
            unsigned int v = va_arg(ap, unsigned int);
            const char *hex = (spec == 'x') ? "0123456789abcdef" : "0123456789ABCDEF";
            if (v == 0) tmp[tlen++] = '0';
            while (v) { tmp[tlen++] = hex[v & 0xF]; v >>= 4; }
            int pad = width - tlen;
            if (!left) while (pad-- > 0) EMIT(zero ? '0' : ' ');
            while (tlen > 0) EMIT(tmp[--tlen]);
            if (left) while (pad-- > 0) EMIT(' ');
        }
        else if (spec == 's')
        {
            const char *s = va_arg(ap, const char *);
            if (!s) s = "(null)";
            while (*s) EMIT(*s++);
        }
        else if (spec == 'c')
        {
            EMIT((char)va_arg(ap, int));
        }
        else if (spec == '%')
        {
            EMIT('%');
        }
    }
    if (buf && sz > 0)
        buf[pos < sz ? pos : sz-1] = 0;
    return (int)pos;

#undef EMIT
}

int snprintf(char *buf, size_t sz, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = _vsnprintf(buf, sz, fmt, ap);
    va_end(ap);
    return r;
}

int vsnprintf(char *buf, size_t sz, const char *fmt, va_list ap)
{
    return _vsnprintf(buf, sz, fmt, ap);
}

int printf(const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    int r = _vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    for (int i = 0; buf[i]; i++)
        _putchar_shim(buf[i]);
    return r;
}

int fprintf(void *stream, const char *fmt, ...)
{
    (void)stream;
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    int r = _vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    for (int i = 0; buf[i]; i++)
        _putchar_shim(buf[i]);
    return r;
}

int sprintf(char *buf, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = _vsnprintf(buf, 4096, fmt, ap);
    va_end(ap);
    return r;
}

typedef struct { int dummy; } FILE;
static FILE _stderr_stub = {0};
void *stderr = &_stderr_stub;
void *stdout = &_stderr_stub;
void *stdin  = &_stderr_stub;

void *fopen(const char *path, const char *mode)  { (void)path; (void)mode; return 0; }
int   fclose(void *f)                             { (void)f; return 0; }
int   fseek(void *f, long off, int whence)        { (void)f; (void)off; (void)whence; return -1; }
long  ftell(void *f)                              { (void)f; return -1; }
size_t fread(void *buf, size_t sz, size_t n, void *f) { (void)buf;(void)sz;(void)n;(void)f; return 0; }
size_t fwrite(const void *buf, size_t sz, size_t n, void *f) { (void)buf;(void)sz;(void)n;(void)f; return 0; }

int atoi(const char *s)
{
    int r = 0, neg = 0;
    while (isspace(*s)) s++;
    if (*s == '-') { neg = 1; s++; }
    else if (*s == '+') s++;
    while (isdigit(*s)) r = r * 10 + (*s++ - '0');
    return neg ? -r : r;
}

long strtol(const char *s, char **end, int base)
{
    long r = 0;
    int neg = 0;
    while (isspace(*s)) s++;
    if (*s == '-') { neg = 1; s++; }
    else if (*s == '+') s++;
    if (base == 0)
    {
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) { base = 16; s += 2; }
        else if (s[0] == '0') { base = 8; s++; }
        else base = 10;
    }
    while (*s)
    {
        int d;
        if (*s >= '0' && *s <= '9') d = *s - '0';
        else if (*s >= 'a' && *s <= 'f') d = *s - 'a' + 10;
        else if (*s >= 'A' && *s <= 'F') d = *s - 'A' + 10;
        else break;
        if (d >= base) break;
        r = r * base + d;
        s++;
    }
    if (end) *end = (char *)s;
    return neg ? -r : r;
}

unsigned long strtoul(const char *s, char **end, int base)
{
    return (unsigned long)strtol(s, end, base);
}

int sscanf(const char *buf, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int count = 0;
    const char *b = buf;
    for (const char *f = fmt; *f; f++)
    {
        if (*f != '%') { if (*b == *f) b++; continue; }
        f++;
        if (*f == 'd')
        {
            int *p = va_arg(ap, int *);
            while (isspace(*b)) b++;
            int neg = 0;
            if (*b == '-') { neg = 1; b++; }
            int v = 0;
            while (isdigit(*b)) v = v * 10 + (*b++ - '0');
            *p = neg ? -v : v;
            count++;
        }
        else if (*f == 's')
        {
            char *p = va_arg(ap, char *);
            while (isspace(*b)) b++;
            while (*b && !isspace(*b)) *p++ = *b++;
            *p = 0;
            count++;
        }
    }
    va_end(ap);
    return count;
}

int puts(const char *s)
{
    while (*s) _putchar_shim(*s++);
    _putchar_shim('\n');
    return 0;
}

int putchar(int c)
{
    _putchar_shim((char)c);
    return c;
}

double sqrt(double x)
{
    if (x <= 0.0) return 0.0;
    double r = x;
    for (int i = 0; i < 20; i++)
        r = 0.5 * (r + x / r);
    return r;
}

double floor(double x)
{
    long i = (long)x;
    return (double)(x < i ? i - 1 : i);
}

double ceil(double x)
{
    long i = (long)x;
    return (double)(x > i ? i + 1 : i);
}

char *strdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = (char *)kmalloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

char *strndup(const char *s, size_t n)
{
    size_t l = strlen(s);
    if (l > n) l = n;
    char *p = (char *)kmalloc(l + 1);
    if (p) { memcpy(p, s, l); p[l] = 0; }
    return p;
}

double strtod(const char *s, char **end)
{
    double r = 0.0, frac = 0.0;
    double div = 1.0;
    int neg = 0;
    while (isspace(*s)) s++;
    if (*s == '-') { neg = 1; s++; } else if (*s == '+') s++;
    while (isdigit(*s)) r = r * 10.0 + (*s++ - '0');
    if (*s == '.')
    {
        s++;
        while (isdigit(*s)) { frac = frac * 10.0 + (*s++ - '0'); div *= 10.0; }
        r += frac / div;
    }
    if (end) *end = (char *)s;
    return neg ? -r : r;
}

double atof(const char *s) { return strtod(s, 0); }

int fflush(void *f) { (void)f; return 0; }

int vfprintf(void *f, const char *fmt, va_list ap)
{
    (void)f;
    char buf[512];
    int r = _vsnprintf(buf, sizeof(buf), fmt, ap);
    for (int i = 0; buf[i]; i++) _putchar_shim(buf[i]);
    return r;
}

int vprintf(const char *fmt, va_list ap)
{
    char buf[512];
    int r = _vsnprintf(buf, sizeof(buf), fmt, ap);
    for (int i = 0; buf[i]; i++) _putchar_shim(buf[i]);
    return r;
}

int putc(int c, void *f) { (void)f; _putchar_shim((char)c); return c; }
int fputc(int c, void *f) { (void)f; _putchar_shim((char)c); return c; }
int fputs(const char *s, void *f) { (void)f; while (*s) _putchar_shim(*s++); return 0; }

int remove(const char *path) { (void)path; return -1; }
int rename(const char *a, const char *b) { (void)a; (void)b; return -1; }
int system(const char *cmd) { (void)cmd; return -1; }
int mkdir(const char *path, unsigned int mode) { (void)path; (void)mode; return -1; }

static int _errno_val = 0;
int *__errno_location(void) { return &_errno_val; }

static signed int _toupper_tab[384];

static void _init_toupper_tab(void)
{
    static int done = 0;
    if (done) return;
    done = 1;
    for (int i = 0; i < 384; i++)
    {
        int c = i - 128;
        _toupper_tab[i] = (c >= 'a' && c <= 'z') ? c - 32 : c;
    }
}

const int **__ctype_toupper_loc(void)
{
    _init_toupper_tab();
    static const int *p = &_toupper_tab[128];
    return &p;
}

const unsigned short **__ctype_b_loc(void)
{
    static unsigned short tbl[384];
    static unsigned short *p = &tbl[128];
    static int done = 0;
    if (!done)
    {
        done = 1;
        for (int i = 0; i < 384; i++)
        {
            int c = i - 128;
            unsigned short v = 0;
            if (c >= 'A' && c <= 'Z') v |= 0x0100;
            if (c >= 'a' && c <= 'z') v |= 0x0200;
            if (c >= '0' && c <= '9') v |= 0x0800;
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') v |= 0x2000;
            tbl[i] = v;
        }
    }
    return (const unsigned short **)&p;
}

int __isoc99_sscanf(const char *buf, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = sscanf(buf, fmt, ap);
    va_end(ap);
    return r;
}

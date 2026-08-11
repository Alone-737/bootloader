#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t strlen(const char *s);
char *strcpy(char *dst, const char *src);
int strcmp(const char *a, const char *b);
char *strrchr(const char *s, int c);

#endif
